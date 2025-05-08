// src/bmt_runner.c
// SPDX-License-Identifier: MIT
// Copyright (c) 2025 Alejandro Avila Marcos
//
// Este archivo es parte de la librería Bare-Metal Test Framework (BMT).
// BMT se distribuye bajo los términos de la Licencia MIT.
// Puedes encontrar una copia de la licencia en el archivo LICENSE.txt
// o en <https://opensource.org/licenses/MIT>.

#include "baremetal_test.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

/**
 * @internal
 * @brief Array to store all registered test cases.
 */
static bmt_test_case_t g_bmt_test_cases[BMT_MAX_TEST_CASES];

/**
 * @internal
 * @brief Counter for the number of registered test cases.
 */
static int g_bmt_test_count = 0;

/**
 * @internal
 * @brief Jump buffer used by the BMT_ASSERT macros to immediately terminate a test
 *        upon assertion failure and return control to the test runner.
 */
jmp_buf g_bmt_assert_jmp_buf;

/**
 * @internal
 * @brief Flag indicating if any BMT_EXPECT macro has failed within the current test.
 *        This allows a test to continue after an EXPECT failure but still be marked as failed.
 */
bool g_bmt_current_test_failed_expect = false;


/**
 * @internal
 * @brief Converts a long integer to a null-terminated string.
 *
 * This is a simplified implementation for internal use.
 * Currently, only supports base 10 (decimal).
 *
 * @param val The long integer value to convert.
 * @param buf Pointer to the character array where the string will be stored.
 *            The buffer must be large enough to hold the converted string, including the null terminator
 *            and a potential negative sign.
 * @param radix The numerical base to use for the conversion. Currently, only 10 is supported.
 *              If any other radix is provided, the buffer will contain "radix_err".
 */
static void bmt_itoa(long val, char* buf, int radix) {
    if (radix != 10) { strcpy(buf, "radix_err"); return; }
    if (val == 0) { strcpy(buf, "0"); return; }

    char* p = buf;
    long t = val;
    if (val < 0) {
        t = -t;
        *p++ = '-';
    }
    int num_digits = 0;
    long temp = t;
    while (temp > 0) {
        temp /= 10;
        num_digits++;
    }
    p += num_digits;
    *p-- = '\0';
    while (t > 0) {
        *p-- = (t % 10) + '0';
        t /= 10;
    }
}

/**
 * @brief Registers a test case to be run by bmt_run_all_tests().
 *
 * Adds the specified test function to an internal list of tests.
 * If the maximum number of test cases (BMT_MAX_TEST_CASES) is reached,
 * an error message is printed via bmt_platform_puts().
 *
 * @param suite_name The name of the test suite to which this test belongs.
 *                   The name will be truncated if it exceeds BMT_MAX_SUITE_NAME_LEN - 1.
 * @param test_name The name of the test case.
 *                  The name will be truncated if it exceeds BMT_MAX_TEST_NAME_LEN - 1.
 * @param func A function pointer to the test case to be executed.
 *             The function should have a `void (void)` signature.
 */
void bmt_register_test(const char* suite_name, const char* test_name, bmt_test_func_ptr_t func) {
    if (g_bmt_test_count < BMT_MAX_TEST_CASES) {
        strncpy(g_bmt_test_cases[g_bmt_test_count].suite_name, suite_name, BMT_MAX_SUITE_NAME_LEN - 1);
        g_bmt_test_cases[g_bmt_test_count].suite_name[BMT_MAX_SUITE_NAME_LEN - 1] = '\0';
        strncpy(g_bmt_test_cases[g_bmt_test_count].test_name, test_name, BMT_MAX_TEST_NAME_LEN - 1);
        g_bmt_test_cases[g_bmt_test_count].test_name[BMT_MAX_TEST_NAME_LEN - 1] = '\0';
        g_bmt_test_cases[g_bmt_test_count].func = func;
        g_bmt_test_cases[g_bmt_test_count].last_run_passed = false; // Default
        g_bmt_test_cases[g_bmt_test_count].duration_ms = 0;
        g_bmt_test_count++;
    } else {
        bmt_platform_puts("ERROR: Max test cases reached. Increase BMT_MAX_TEST_CASES.\r\n");
    }
}

/**
 * @brief Reports a test failure, typically called by assertion macros.
 *
 * Formats and prints a detailed failure message to the platform's output.
 * This includes the file name, line number, assertion type, the expression
 * that failed, and an optional custom message with variadic arguments.
 *
 * @param file The name of the source file where the failure occurred (usually `__FILE__`).
 * @param line The line number in the source file where the failure occurred (usually `__LINE__`).
 * @param assertion_type A string describing the type of assertion that failed (e.g., "ASSERT_TRUE", "EXPECT_EQ").
 * @param expression A string representation of the expression that was evaluated.
 * @param msg_fmt A printf-style format string for an optional custom message. Can be NULL.
 * @param ... Variadic arguments corresponding to the `msg_fmt` format string.
 *
 * @note Supports '%s' for strings and '%ld' for long integers in `msg_fmt`.
 */
void bmt_report_failure(const char* file, int line, const char* assertion_type, const char* expression, const char* msg_fmt, ...) {
    char buffer[256];

    bmt_platform_puts(file);
    bmt_platform_putchar(':');
    char line_buf[12];
    bmt_itoa(line, line_buf, 10);
    bmt_platform_puts(line_buf);
    bmt_platform_puts(": Failure\r\n");

    strcpy(buffer, "  "); // Indent
    strcat(buffer, assertion_type);
    strcat(buffer, "(");
    strncat(buffer, expression, sizeof(buffer) - strlen(buffer) - 2);
    strcat(buffer, ")\r\n");
    bmt_platform_puts(buffer);

    if (msg_fmt) {
        bmt_platform_puts("    Message: ");
        va_list args;
        va_start(args, msg_fmt);
        const char *s_arg;
        long d_arg;
        char temp_buf[20];
        char* p = (char*)msg_fmt;
        while(*p) {
            if (*p == '%') {
                p++;
                if (*p == 's') {
                    s_arg = va_arg(args, const char*);
                    bmt_platform_puts(s_arg ? s_arg : "(null)");
                } else if (*p == 'l' && *(p+1) == 'd') {
                    d_arg = va_arg(args, long);
                    bmt_itoa(d_arg, temp_buf, 10);
                    bmt_platform_puts(temp_buf);
                    p++;
                } else {
                    bmt_platform_putchar('%');
                    bmt_platform_putchar(*p);
                }
            } else {
                bmt_platform_putchar(*p);
            }
            p++;
        }
        va_end(args);
        bmt_platform_puts("\r\n");
    }
}

/**
 * @brief Terminates the execution of the current test case immediately.
 *
 * This function is typically called by BMT_ASSERT macros when an assertion fails.
 * It performs a `longjmp()` to a point in `bmt_run_all_tests()`, effectively
 * skipping the remainder of the current test function.
 */
void bmt_terminate_current_test(void) {
    longjmp(g_bmt_assert_jmp_buf, 1);
}

/**
 * @brief Runs all registered test cases and reports the results.
 *
 * This is the main entry point for executing the test suite. It performs the following steps:
 * 1. Initializes the platform I/O using `bmt_platform_io_init()`.
 * 2. Prints a header indicating the start of test execution and the total number of tests.
 * 3. Iterates through each registered test case:
 *    a. Prints a "[ RUN      ]" message with the test suite and name.
 *    b. Resets failure flags for the current test.
 *    c. Records the start time using `bmt_platform_get_msec_ticks()`.
 *    d. Executes the test function. A `setjmp()` is used to catch `longjmp()` calls
 *       from `bmt_terminate_current_test()` (triggered by BMT_ASSERT macros).
 *    e. Records the end time and calculates the test duration, handling timer overflows.
 *    f. Determines if the test passed or failed based on assertion and expectation results.
 *    g. Prints an "[       OK ]" or "[  FAILED  ]" message along with the test name and duration.
 *    h. Updates overall pass/fail counters and total duration.
 * 4. Prints a summary of the test run, including:
 *    a. Total number of tests run and total duration.
 *    b. Number of passed tests.
 *    c. If any tests failed, the number of failed tests and a list of their names.
 * 5. Prints the final count of failed tests.
 *
 * @return The total number of tests that failed. Returns 0 if all tests passed.
 */
int bmt_run_all_tests(void) {
    bmt_platform_io_init(); // Initialize platform I/O

    char buffer[128];
    bmt_platform_puts("[==========] Running ");
    bmt_itoa(g_bmt_test_count, buffer, 10);
    bmt_platform_puts(buffer);
    bmt_platform_puts(" tests.\r\n");

    int tests_passed = 0;
    int tests_failed = 0;
    uint32_t total_duration_ms = 0;

    for (int i = 0; i < g_bmt_test_count; ++i) {
        bmt_platform_puts("[ RUN      ] ");
        bmt_platform_puts(g_bmt_test_cases[i].suite_name);
        bmt_platform_putchar('.');
        bmt_platform_puts(g_bmt_test_cases[i].test_name);
        bmt_platform_puts("\r\n");

        g_bmt_current_test_failed_expect = false; // Reset for EXPECT macros
        bool current_test_passed_assert = true;  // Assume no ASSERT failures initially

        uint32_t start_ticks = bmt_platform_get_msec_ticks();

        if (setjmp(g_bmt_assert_jmp_buf) == 0) {
            // Execute the test
            g_bmt_test_cases[i].func();
        } else {
            // An ASSERT macro failed and caused a longjmp here
            current_test_passed_assert = false;
        }
        
        uint32_t end_ticks = bmt_platform_get_msec_ticks();
        // Handle timer overflow when calculating duration
        g_bmt_test_cases[i].duration_ms = (end_ticks >= start_ticks) ? (end_ticks - start_ticks) : (0xFFFFFFFF - start_ticks + end_ticks + 1); 
        total_duration_ms += g_bmt_test_cases[i].duration_ms;

        g_bmt_test_cases[i].last_run_passed = current_test_passed_assert && !g_bmt_current_test_failed_expect;

        if (g_bmt_test_cases[i].last_run_passed) {
            bmt_platform_puts("[       OK ] ");
            tests_passed++;
        } else {
            bmt_platform_puts("[  FAILED  ] ");
            tests_failed++;
        }
        bmt_platform_puts(g_bmt_test_cases[i].suite_name);
        bmt_platform_putchar('.');
        bmt_platform_puts(g_bmt_test_cases[i].test_name);
        bmt_platform_puts(" (");
        bmt_itoa(g_bmt_test_cases[i].duration_ms, buffer, 10);
        bmt_platform_puts(buffer);
        bmt_platform_puts(" ms)\r\n");
    }

    bmt_platform_puts("[==========] ");
    bmt_itoa(g_bmt_test_count, buffer, 10);
    bmt_platform_puts(buffer);
    bmt_platform_puts(" tests ran. (");
    bmt_itoa(total_duration_ms, buffer, 10);
    bmt_platform_puts(buffer);
    bmt_platform_puts(" ms total)\r\n");
    
    bmt_platform_puts("[  PASSED  ] ");
    bmt_itoa(tests_passed, buffer, 10);
    bmt_platform_puts(buffer);
    bmt_platform_puts(" tests.\r\n");

    if (tests_failed > 0) {
        bmt_platform_puts("[  FAILED  ] ");
        bmt_itoa(tests_failed, buffer, 10);
        bmt_platform_puts(buffer);
        bmt_platform_puts(" tests, listed below:\r\n");
        for (int i = 0; i < g_bmt_test_count; ++i) {
            if (!g_bmt_test_cases[i].last_run_passed) {
                bmt_platform_puts("[  FAILED  ] ");
                bmt_platform_puts(g_bmt_test_cases[i].suite_name);
                bmt_platform_putchar('.');
                bmt_platform_puts(g_bmt_test_cases[i].test_name);
                bmt_platform_puts("\r\n");
            }
        }
    }
    bmt_platform_puts("\r\n");
    bmt_itoa(tests_failed, buffer, 10);
    bmt_platform_puts(buffer);
    if (tests_failed == 1) {
        bmt_platform_puts(" FAILED TEST\r\n");
    } else {
        bmt_platform_puts(" FAILED TESTS\r\n");
    }
    
    return tests_failed;
}
