// include/baremetal_test.h
// SPDX-License-Identifier: MIT
// Copyright (c) 2025 Alejandro Avila Marcos
//
// Este archivo es parte de la librería Bare-Metal Test Framework (BMT).
// BMT se distribuye bajo los términos de la Licencia MIT.
// Puedes encontrar una copia de la licencia en el archivo LICENSE.txt
// o en <https://opensource.org/licenses/MIT>.

#ifndef BAREMETAL_TEST_H
#define BAREMETAL_TEST_H

#include <stdbool.h>
#include <setjmp.h>     
#include "bmt_platform_io.h"

/**
 * @brief Maximum number of test cases that can be registered.
 */
#define BMT_MAX_TEST_CASES 64

/**
 * @brief Maximum length of a test case name.
 */
#define BMT_MAX_TEST_NAME_LEN 64

/**
 * @brief Maximum length of a test suite name.
 */
#define BMT_MAX_SUITE_NAME_LEN 64

/**
 * @brief Typedef for a test function pointer.
 *
 * Test functions should take no arguments and return void.
 */
typedef void (*bmt_test_func_ptr_t)(void);

/**
 * @struct bmt_test_case_t
 * @brief Structure to hold information about a single test case.
 */
typedef struct {
    char suite_name[BMT_MAX_SUITE_NAME_LEN]; /**< Name of the test suite. */
    char test_name[BMT_MAX_TEST_NAME_LEN];   /**< Name of the test case. */
    bmt_test_func_ptr_t func;                /**< Pointer to the test function. */
    bool last_run_passed;                    /**< Status of the last run (true if passed, false otherwise). */
    uint32_t duration_ms;                    /**< Duration of the last test run in milliseconds. */
} bmt_test_case_t;

/**
 * @brief Registers a new test case.
 *
 * This function is typically called by the TEST macro.
 *
 * @param suite_name Name of the test suite.
 * @param test_name Name of the test case.
 * @param func Pointer to the test function.
 */
void bmt_register_test(const char* suite_name, const char* test_name, bmt_test_func_ptr_t func);

/**
 * @brief Runs all registered test cases.
 *
 * @return The number of failed tests. 0 if all tests passed.
 */
int bmt_run_all_tests(void);

/**
 * @brief Reports a test failure.
 *
 * This function is called by assertion macros when a check fails.
 * It records the failure and prepares to terminate the current test.
 *
 * @param file The file where the failure occurred.
 * @param line The line number where the failure occurred.
 * @param assertion_type The type of assertion that failed (e.g., "ASSERT_TRUE").
 * @param expression The string representation of the expression that failed.
 * @param msg_fmt Optional custom message format string (printf-like).
 * @param ... Optional arguments for the custom message format string.
 */
void bmt_report_failure(const char* file, int line, const char* assertion_type, const char* expression, const char* msg_fmt, ...);

/**
 * @brief Terminates the current test execution.
 *
 * This function uses longjmp to return to the test runner, effectively ending
 * the current test case. It is called after a failure is reported by an ASSERT_* macro.
 */
void bmt_terminate_current_test(void);

/**
 * @brief Jump buffer used for handling assertion failures.
 *
 * When an ASSERT_* macro fails, it calls bmt_terminate_current_test(),
 * which uses this buffer to longjmp back to the test runner.
 */
extern jmp_buf g_bmt_assert_jmp_buf;

/**
 * @brief Flag indicating if the current test has failed an EXPECT_* macro.
 *
 * This flag is set to true if any EXPECT_* macro fails within the current test.
 * Unlike ASSERT_* macros, EXPECT_* macros do not terminate the test immediately.
 */
extern bool g_bmt_current_test_failed_expect;

// --- Macros para el Usuario ---

/**
 * @def TEST(TestSuiteName, TestName)
 * @brief Defines and registers a test case.
 *
 * This macro is the primary way to define a test. It creates a static function
 * for the test body and a constructor function to register the test.
 *
 * @param TestSuiteName The name of the test suite this test belongs to.
 * @param TestName The name of this specific test case.
 *
 * Example usage:
 * @code
 * TEST(MySuite, MyFirstTest) {
 *   ASSERT_TRUE(1 == 1);
 * }
 * @endcode
 */
#define TEST(TestSuiteName, TestName) \
    static void bmt_test_##TestSuiteName##_##TestName(void); \
    __attribute__((constructor)) \
    static void bmt_register_##TestSuiteName##_##TestName(void) { \
        bmt_register_test(#TestSuiteName, #TestName, bmt_test_##TestSuiteName##_##TestName); \
    } \
    static void bmt_test_##TestSuiteName##_##TestName(void)

/**
 * @brief Internal common logic for ASSERT_* macros.
 * Reports a failure if the condition is false and terminates the test.
 */
#define BMT_ASSERT_COMMON(condition, assertion_type, expr_str, ...) \
    do { \
        if (!(condition)) { \
            bmt_report_failure(__FILE__, __LINE__, assertion_type, expr_str, ##__VA_ARGS__); \
            bmt_terminate_current_test(); /* longjmp */ \
        } \
    } while (0)

// **Aserciones Booleanas**

/**
 * @def ASSERT_TRUE(condition)
 * @brief Asserts that a condition is true.
 * If the condition is false, the test is terminated.
 * @param condition The condition to check.
 */
#define ASSERT_TRUE(condition) BMT_ASSERT_COMMON(!!(condition), "ASSERT_TRUE", #condition, NULL)

/**
 * @def ASSERT_FALSE(condition)
 * @brief Asserts that a condition is false.
 * If the condition is true, the test is terminated.
 * @param condition The condition to check.
 */
#define ASSERT_FALSE(condition) BMT_ASSERT_COMMON(!(condition), "ASSERT_FALSE", #condition, NULL)

// **Aserciones Binarias de ComparaciÃ³n (Enteros y Punteros)**
// Nota: Se castea a (long) para la impresiÃ³n, lo que es generalmente seguro para la mayorÃ­a de los enteros y punteros.

/**
 * @def ASSERT_EQ(val1, val2)
 * @brief Asserts that two values are equal.
 * Compares `val1` and `val2`. If they are not equal, the test is terminated.
 * Values are cast to `long` for comparison and printing.
 * @param val1 The first value.
 * @param val2 The second value.
 */
#define ASSERT_EQ(val1, val2) BMT_ASSERT_COMMON(((long)(val1)) == ((long)(val2)), "ASSERT_EQ", #val1 " == " #val2, "Expected: %ld, Actual: %ld", (long)(val1), (long)(val2))

/**
 * @def ASSERT_NE(val1, val2)
 * @brief Asserts that two values are not equal.
 * Compares `val1` and `val2`. If they are equal, the test is terminated.
 * Values are cast to `long` for comparison and printing.
 * @param val1 The first value.
 * @param val2 The second value.
 */
#define ASSERT_NE(val1, val2) BMT_ASSERT_COMMON(((long)(val1)) != ((long)(val2)), "ASSERT_NE", #val1 " != " #val2, "Expected: %ld != %ld, but they are equal", (long)(val1), (long)(val2))

/**
 * @def ASSERT_LT(val1, val2)
 * @brief Asserts that `val1` is less than `val2`.
 * If `val1` is not less than `val2`, the test is terminated.
 * Values are cast to `long` for comparison and printing.
 * @param val1 The first value.
 * @param val2 The second value.
 */
#define ASSERT_LT(val1, val2) BMT_ASSERT_COMMON(((long)(val1)) < ((long)(val2)), "ASSERT_LT", #val1 " < " #val2, "Expected: %ld < %ld", (long)(val1), (long)(val2))

/**
 * @def ASSERT_LE(val1, val2)
 * @brief Asserts that `val1` is less than or equal to `val2`.
 * If `val1` is not less than or equal to `val2`, the test is terminated.
 * Values are cast to `long` for comparison and printing.
 * @param val1 The first value.
 * @param val2 The second value.
 */
#define ASSERT_LE(val1, val2) BMT_ASSERT_COMMON(((long)(val1)) <= ((long)(val2)), "ASSERT_LE", #val1 " <= " #val2, "Expected: %ld <= %ld", (long)(val1), (long)(val2))

/**
 * @def ASSERT_GT(val1, val2)
 * @brief Asserts that `val1` is greater than `val2`.
 * If `val1` is not greater than `val2`, the test is terminated.
 * Values are cast to `long` for comparison and printing.
 * @param val1 The first value.
 * @param val2 The second value.
 */
#define ASSERT_GT(val1, val2) BMT_ASSERT_COMMON(((long)(val1)) > ((long)(val2)), "ASSERT_GT", #val1 " > " #val2, "Expected: %ld > %ld", (long)(val1), (long)(val2))

/**
 * @def ASSERT_GE(val1, val2)
 * @brief Asserts that `val1` is greater than or equal to `val2`.
 * If `val1` is not greater than or equal to `val2`, the test is terminated.
 * Values are cast to `long` for comparison and printing.
 * @param val1 The first value.
 * @param val2 The second value.
 */
#define ASSERT_GE(val1, val2) BMT_ASSERT_COMMON(((long)(val1)) >= ((long)(val2)), "ASSERT_GE", #val1 " >= " #val2, "Expected: %ld >= %ld", (long)(val1), (long)(val2))

// **Aserciones de Punteros Nulos**

/**
 * @def ASSERT_NULL(ptr)
 * @brief Asserts that a pointer is NULL.
 * If the pointer `ptr` is not NULL, the test is terminated.
 * @param ptr The pointer to check.
 */
#define ASSERT_NULL(ptr) BMT_ASSERT_COMMON((ptr) == NULL, "ASSERT_NULL", #ptr " == NULL", "Actual: %p", (void*)(ptr))

/**
 * @def ASSERT_NOT_NULL(ptr)
 * @brief Asserts that a pointer is not NULL.
 * If the pointer `ptr` is NULL, the test is terminated.
 * @param ptr The pointer to check.
 */
#define ASSERT_NOT_NULL(ptr) BMT_ASSERT_COMMON((ptr) != NULL, "ASSERT_NOT_NULL", #ptr " != NULL", NULL)

// **Aserciones de Cadenas (Requieren strcmp, strncmp, strcasecmp de libc o implementaciones propias)**
// AsegÃºrate de que los punteros a cadena no sean NULL antes de pasarlos a strcmp, etc.
// o que tu strcmp maneje NULL de forma segura.

/**
 * @def ASSERT_STREQ(s1, s2)
 * @brief Asserts that two strings are equal.
 * Uses `strcmp`. If strings are different, or if either is NULL, the test is terminated.
 * @param s1 The first string.
 * @param s2 The second string.
 */
#define ASSERT_STREQ(s1, s2) \
    BMT_ASSERT_COMMON(((s1) != NULL && (s2) != NULL && strcmp((s1), (s2)) == 0), "ASSERT_STREQ", #s1 " STREQ " #s2, \
                      "Expected: \"%s\", Actual: \"%s\"", (s1) ? (s1) : "NULL", (s2) ? (s2) : "NULL")

/**
 * @def ASSERT_STRNE(s1, s2)
 * @brief Asserts that two strings are not equal.
 * Uses `strcmp`. If strings are equal (and both non-NULL), the test is terminated.
 * @param s1 The first string.
 * @param s2 The second string.
 */
#define ASSERT_STRNE(s1, s2) \
    BMT_ASSERT_COMMON(!((s1) != NULL && (s2) != NULL && strcmp((s1), (s2)) == 0), "ASSERT_STRNE", #s1 " STRNE " #s2, \
                      "Expected strings to be different. s1: \"%s\", s2: \"%s\"", (s1) ? (s1) : "NULL", (s2) ? (s2) : "NULL")

/**
 * @def ASSERT_STRCASEEQ(s1, s2)
 * @brief Asserts that two strings are equal, ignoring case.
 * Uses `strcasecmp`. If strings are different (ignoring case), or if either is NULL, the test is terminated.
 * @param s1 The first string.
 * @param s2 The second string.
 */
#define ASSERT_STRCASEEQ(s1, s2) \
    BMT_ASSERT_COMMON(((s1) != NULL && (s2) != NULL && strcasecmp((s1), (s2)) == 0), "ASSERT_STRCASEEQ", #s1 " STRCASEEQ " #s2, \
                      "Expected (ignore case): \"%s\", Actual: \"%s\"", (s1) ? (s1) : "NULL", (s2) ? (s2) : "NULL")

/**
 * @def ASSERT_STRCASENE(s1, s2)
 * @brief Asserts that two strings are not equal, ignoring case.
 * Uses `strcasecmp`. If strings are equal (ignoring case, and both non-NULL), the test is terminated.
 * @param s1 The first string.
 * @param s2 The second string.
 */
#define ASSERT_STRCASENE(s1, s2) \
    BMT_ASSERT_COMMON(!((s1) != NULL && (s2) != NULL && strcasecmp((s1), (s2)) == 0), "ASSERT_STRCASENE", #s1 " STRCASENE " #s2, \
                      "Expected strings to be different (ignore case). s1: \"%s\", s2: \"%s\"", (s1) ? (s1) : "NULL", (s2) ? (s2) : "NULL")

// Versiones con N caracteres (requieren strncmp, strncasecmp)

/**
 * @def ASSERT_STRNEQ(s1, s2, n)
 * @brief Asserts that the first `n` characters of two strings are equal.
 * Uses `strncmp`. If the first `n` characters are different, or if either string is NULL, the test is terminated.
 * @param s1 The first string.
 * @param s2 The second string.
 * @param n The number of characters to compare.
 */
#define ASSERT_STRNEQ(s1, s2, n) \
    BMT_ASSERT_COMMON(((s1) != NULL && (s2) != NULL && strncmp((s1), (s2), (n)) == 0), "ASSERT_STRNEQ", #s1 " STRNEQ(" #n ") " #s2, \
                      "Expected first %u chars: \"%s\", Actual: \"%s\"", (unsigned int)(n), (s1) ? (s1) : "NULL", (s2) ? (s2) : "NULL")

/**
 * @def ASSERT_STRNNE(s1, s2, n)
 * @brief Asserts that the first `n` characters of two strings are not equal.
 * Uses `strncmp`. If the first `n` characters are equal (and both strings non-NULL), the test is terminated.
 * @param s1 The first string.
 * @param s2 The second string.
 * @param n The number of characters to compare.
 */
#define ASSERT_STRNNE(s1, s2, n) \
    BMT_ASSERT_COMMON(!((s1) != NULL && (s2) != NULL && strncmp((s1), (s2), (n)) == 0), "ASSERT_STRNNE", #s1 " STRNNE(" #n ") " #s2, \
                      "Expected first %u chars of strings to be different. s1: \"%s\", s2: \"%s\"", (unsigned int)(n), (s1) ? (s1) : "NULL", (s2) ? (s2) : "NULL")

// **Aserciones de Punto Flotante (Requieren fabsf/fabs y soporte %f/%g en bmt_report_failure)**
// NOTA: La comparaciÃ³n directa de flotantes es generalmente una mala idea debido a errores de precisiÃ³n.
// Usa ASSERT_FLOAT_NEAR o ASSERT_DOUBLE_NEAR siempre que sea posible.

/**
 * @def ASSERT_FLOAT_EQ(val1, val2)
 * @brief Asserts that two float values are exactly equal.
 * @warning Direct comparison of floating-point values can be problematic due to precision issues.
 * Consider using ASSERT_FLOAT_NEAR for a robust comparison.
 * If `val1` is not equal to `val2`, the test is terminated.
 * @param val1 The first float value.
 * @param val2 The second float value.
 */
#define ASSERT_FLOAT_EQ(val1, val2) \
    BMT_ASSERT_COMMON((val1) == (val2), "ASSERT_FLOAT_EQ", #val1 " == " #val2, \
                      "Expected: %g, Actual: %g", (double)(val1), (double)(val2))

/**
 * @def ASSERT_DOUBLE_EQ(val1, val2)
 * @brief Asserts that two double values are exactly equal.
 * @warning Direct comparison of floating-point values can be problematic due to precision issues.
 * Consider using ASSERT_NEAR for a robust comparison with doubles.
 * If `val1` is not equal to `val2`, the test is terminated.
 * @param val1 The first double value.
 * @param val2 The second double value.
 */
#define ASSERT_DOUBLE_EQ(val1, val2) \
    BMT_ASSERT_COMMON((val1) == (val2), "ASSERT_DOUBLE_EQ", #val1 " == " #val2, \
                      "Expected: %g, Actual: %g", (double)(val1), (double)(val2))

// Compara si dos flotantes estÃ¡n dentro de un error absoluto.
// (val1) y (val2) son los valores a comparar.
// (abs_error) es el error absoluto mÃ¡ximo permitido.

/**
 * @def ASSERT_NEAR(val1, val2, abs_error)
 * @brief Asserts that two floating-point values (double) are close to each other.
 * The assertion passes if `fabs((val1) - (val2)) <= fabs(abs_error)`.
 * If the condition is not met, the test is terminated.
 * @param val1 The first double value.
 * @param val2 The second double value.
 * @param abs_error The maximum allowed absolute difference.
 */
#define ASSERT_NEAR(val1, val2, abs_error) \
    BMT_ASSERT_COMMON(fabs((val1) - (val2)) <= fabs(abs_error), "ASSERT_NEAR", #val1 " NEAR " #val2 ", error " #abs_error, \
                      "Value1: %g, Value2: %g, Diff: %g, Max Abs Error: %g", \
                      (double)(val1), (double)(val2), fabs((double)(val1) - (double)(val2)), fabs((double)(abs_error)))

// Para floats especÃ­ficamente (usa fabsf si estÃ¡ disponible y es diferente de fabs)

/**
 * @def ASSERT_FLOAT_NEAR(val1, val2, abs_error)
 * @brief Asserts that two float values are close to each other.
 * The assertion passes if `fabsf((val1) - (val2)) <= fabsf(abs_error)`.
 * If the condition is not met, the test is terminated.
 * @param val1 The first float value.
 * @param val2 The second float value.
 * @param abs_error The maximum allowed absolute difference (as a float).
 */
#define ASSERT_FLOAT_NEAR(val1, val2, abs_error) \
    BMT_ASSERT_COMMON(fabsf((val1) - (val2)) <= fabsf(abs_error), "ASSERT_FLOAT_NEAR", #val1 " NEAR " #val2 ", error " #abs_error, \
                      "Value1: %f, Value2: %f, Diff: %f, Max Abs Error: %f", \
                      (float)(val1), (float)(val2), fabsf((float)(val1) - (float)(val2)), fabsf((float)(abs_error)))

// **Aserciones de Fallo ExplÃ­cito**

/**
 * @def FAIL()
 * @brief Explicitly fails the current test.
 * This macro will always terminate the current test case.
 */
#define FAIL() BMT_ASSERT_COMMON(0, "FAIL", "Explicit failure triggered by FAIL()", NULL)

/**
 * @def ADD_FAILURE()
 * @brief Explicitly adds a failure to the current test but does not terminate it.
 * This is useful for reporting multiple failures within a test that should all be noted.
 * The test will be marked as failed, but execution will continue.
 * @note This macro relies on `g_bmt_current_test_failed_expect` being handled correctly by the runner
 * if it's intended to mark the test as failed without immediate termination.
 * However, the current implementation of `bmt_report_failure` is typically followed by `bmt_terminate_current_test`
 * in `BMT_ASSERT_COMMON`. This macro calls `bmt_report_failure` directly.
 */
#define ADD_FAILURE() bmt_report_failure(__FILE__, __LINE__, "ADD_FAILURE", "Explicit failure triggered by ADD_FAILURE()", NULL) // No salta

/**
 * @def SUCCEED()
 * @brief Explicitly indicates success at a point in a test.
 * This macro primarily serves as a marker or for debugging; it prints a success message.
 * It does not affect the test's pass/fail status.
 */
#define SUCCEED() bmt_platform_puts(__FILE__); bmt_platform_putchar(':'); /*bmt_itoa(__LINE__, num_buf, 10); bmt_platform_puts(num_buf);*/ bmt_platform_puts(": SUCCEED()\r\n") // Solo imprime

// --- Macros de Expectativa (EXPECT_* reporta el fallo pero continÃºa el test actual) ---

/**
 * @brief Internal common logic for EXPECT_* macros.
 * Reports a failure if the condition is false but does not terminate the test.
 * Sets `g_bmt_current_test_failed_expect` to true.
 */
#define BMT_EXPECT_COMMON(condition, assertion_type, expr_str, ...) \
    do { \
        if (!(condition)) { \
            bmt_report_failure(__FILE__, __LINE__, assertion_type, expr_str, ##__VA_ARGS__); \
            g_bmt_current_test_failed_expect = true; \
        } \
    } while (0)

// **Expectativas Booleanas**

/**
 * @def EXPECT_TRUE(condition)
 * @brief Expects that a condition is true.
 * If the condition is false, a failure is reported, but the test continues.
 * @param condition The condition to check.
 */
#define EXPECT_TRUE(condition) BMT_EXPECT_COMMON(!!(condition), "EXPECT_TRUE", #condition, NULL)

/**
 * @def EXPECT_FALSE(condition)
 * @brief Expects that a condition is false.
 * If the condition is true, a failure is reported, but the test continues.
 * @param condition The condition to check.
 */
#define EXPECT_FALSE(condition) BMT_EXPECT_COMMON(!(condition), "EXPECT_FALSE", #condition, NULL)

// **Expectativas Binarias de ComparaciÃ³n (Enteros y Punteros)**

/**
 * @def EXPECT_EQ(val1, val2)
 * @brief Expects that two values are equal.
 * If `val1` is not equal to `val2`, a failure is reported, but the test continues.
 * Values are cast to `long` for comparison and printing.
 * @param val1 The first value.
 * @param val2 The second value.
 */
#define EXPECT_EQ(val1, val2) BMT_EXPECT_COMMON(((long)(val1)) == ((long)(val2)), "EXPECT_EQ", #val1 " == " #val2, "Expected: %ld, Actual: %ld", (long)(val1), (long)(val2))

/**
 * @def EXPECT_NE(val1, val2)
 * @brief Expects that two values are not equal.
 * If `val1` is equal to `val2`, a failure is reported, but the test continues.
 * Values are cast to `long` for comparison and printing.
 * @param val1 The first value.
 * @param val2 The second value.
 */
#define EXPECT_NE(val1, val2) BMT_EXPECT_COMMON(((long)(val1)) != ((long)(val2)), "EXPECT_NE", #val1 " != " #val2, "Expected: %ld != %ld, but they are equal", (long)(val1), (long)(val2))

/**
 * @def EXPECT_LT(val1, val2)
 * @brief Expects that `val1` is less than `val2`.
 * If `val1` is not less than `val2`, a failure is reported, but the test continues.
 * Values are cast to `long` for comparison and printing.
 * @param val1 The first value.
 * @param val2 The second value.
 */
#define EXPECT_LT(val1, val2) BMT_EXPECT_COMMON(((long)(val1)) < ((long)(val2)), "EXPECT_LT", #val1 " < " #val2, "Expected: %ld < %ld", (long)(val1), (long)(val2))

/**
 * @def EXPECT_LE(val1, val2)
 * @brief Expects that `val1` is less than or equal to `val2`.
 * If `val1` is not less than or equal to `val2`, a failure is reported, but the test continues.
 * Values are cast to `long` for comparison and printing.
 * @param val1 The first value.
 * @param val2 The second value.
 */
#define EXPECT_LE(val1, val2) BMT_EXPECT_COMMON(((long)(val1)) <= ((long)(val2)), "EXPECT_LE", #val1 " <= " #val2, "Expected: %ld <= %ld", (long)(val1), (long)(val2))

/**
 * @def EXPECT_GT(val1, val2)
 * @brief Expects that `val1` is greater than `val2`.
 * If `val1` is not greater than `val2`, a failure is reported, but the test continues.
 * Values are cast to `long` for comparison and printing.
 * @param val1 The first value.
 * @param val2 The second value.
 */
#define EXPECT_GT(val1, val2) BMT_EXPECT_COMMON(((long)(val1)) > ((long)(val2)), "EXPECT_GT", #val1 " > " #val2, "Expected: %ld > %ld", (long)(val1), (long)(val2))

/**
 * @def EXPECT_GE(val1, val2)
 * @brief Expects that `val1` is greater than or equal to `val2`.
 * If `val1` is not greater than or equal to `val2`, a failure is reported, but the test continues.
 * Values are cast to `long` for comparison and printing.
 * @param val1 The first value.
 * @param val2 The second value.
 */
#define EXPECT_GE(val1, val2) BMT_EXPECT_COMMON(((long)(val1)) >= ((long)(val2)), "EXPECT_GE", #val1 " >= " #val2, "Expected: %ld >= %ld", (long)(val1), (long)(val2))

// **Expectativas de Punteros Nulos**

/**
 * @def EXPECT_NULL(ptr)
 * @brief Expects that a pointer is NULL.
 * If `ptr` is not NULL, a failure is reported, but the test continues.
 * @param ptr The pointer to check.
 */
#define EXPECT_NULL(ptr) BMT_EXPECT_COMMON((ptr) == NULL, "EXPECT_NULL", #ptr " == NULL", "Actual: %p", (void*)(ptr))

/**
 * @def EXPECT_NOT_NULL(ptr)
 * @brief Expects that a pointer is not NULL.
 * If `ptr` is NULL, a failure is reported, but the test continues.
 * @param ptr The pointer to check.
 */
#define EXPECT_NOT_NULL(ptr) BMT_EXPECT_COMMON((ptr) != NULL, "EXPECT_NOT_NULL", #ptr " != NULL", NULL)

// **Expectativas de Cadenas**

/**
 * @def EXPECT_STREQ(s1, s2)
 * @brief Expects that two strings are equal.
 * Uses `strcmp`. If strings are different, or if either is NULL, a failure is reported, but the test continues.
 * @param s1 The first string.
 * @param s2 The second string.
 */
#define EXPECT_STREQ(s1, s2) \
    BMT_EXPECT_COMMON(((s1) != NULL && (s2) != NULL && strcmp((s1), (s2)) == 0), "EXPECT_STREQ", #s1 " STREQ " #s2, \
                      "Expected: \"%s\", Actual: \"%s\"", (s1) ? (s1) : "NULL", (s2) ? (s2) : "NULL")

/**
 * @def EXPECT_STRNE(s1, s2)
 * @brief Expects that two strings are not equal.
 * Uses `strcmp`. If strings are equal (and both non-NULL), a failure is reported, but the test continues.
 * @param s1 The first string.
 * @param s2 The second string.
 */
#define EXPECT_STRNE(s1, s2) \
    BMT_EXPECT_COMMON(!((s1) != NULL && (s2) != NULL && strcmp((s1), (s2)) == 0), "EXPECT_STRNE", #s1 " STRNE " #s2, \
                      "Expected strings to be different. s1: \"%s\", s2: \"%s\"", (s1) ? (s1) : "NULL", (s2) ? (s2) : "NULL")

/**
 * @def EXPECT_STRCASEEQ(s1, s2)
 * @brief Expects that two strings are equal, ignoring case.
 * Uses `strcasecmp`. If strings are different (ignoring case), or if either is NULL, a failure is reported, but the test continues.
 * @param s1 The first string.
 * @param s2 The second string.
 */
#define EXPECT_STRCASEEQ(s1, s2) \
    BMT_EXPECT_COMMON(((s1) != NULL && (s2) != NULL && strcasecmp((s1), (s2)) == 0), "EXPECT_STRCASEEQ", #s1 " STRCASEEQ " #s2, \
                      "Expected (ignore case): \"%s\", Actual: \"%s\"", (s1) ? (s1) : "NULL", (s2) ? (s2) : "NULL")

/**
 * @def EXPECT_STRCASENE(s1, s2)
 * @brief Expects that two strings are not equal, ignoring case.
 * Uses `strcasecmp`. If strings are equal (ignoring case, and both non-NULL), a failure is reported, but the test continues.
 * @param s1 The first string.
 * @param s2 The second string.
 */
#define EXPECT_STRCASENE(s1, s2) \
    BMT_EXPECT_COMMON(!((s1) != NULL && (s2) != NULL && strcasecmp((s1), (s2)) == 0), "EXPECT_STRCASENE", #s1 " STRCASENE " #s2, \
                      "Expected strings to be different (ignore case). s1: \"%s\", s2: \"%s\"", (s1) ? (s1) : "NULL", (s2) ? (s2) : "NULL")

/**
 * @def EXPECT_STRNEQ(s1, s2, n)
 * @brief Expects that the first `n` characters of two strings are equal.
 * Uses `strncmp`. If the first `n` characters are different, or if either string is NULL, a failure is reported, but the test continues.
 * @param s1 The first string.
 * @param s2 The second string.
 * @param n The number of characters to compare.
 */
#define EXPECT_STRNEQ(s1, s2, n) \
    BMT_EXPECT_COMMON(((s1) != NULL && (s2) != NULL && strncmp((s1), (s2), (n)) == 0), "EXPECT_STRNEQ", #s1 " STRNEQ(" #n ") " #s2, \
                      "Expected first %u chars: \"%s\", Actual: \"%s\"", (unsigned int)(n), (s1) ? (s1) : "NULL", (s2) ? (s2) : "NULL")

/**
 * @def EXPECT_STRNNE(s1, s2, n)
 * @brief Expects that the first `n` characters of two strings are not equal.
 * Uses `strncmp`. If the first `n` characters are equal (and both strings non-NULL), a failure is reported, but the test continues.
 * @param s1 The first string.
 * @param s2 The second string.
 * @param n The number of characters to compare.
 */
#define EXPECT_STRNNE(s1, s2, n) \
    BMT_EXPECT_COMMON(!((s1) != NULL && (s2) != NULL && strncmp((s1), (s2), (n)) == 0), "EXPECT_STRNNE", #s1 " STRNNE(" #n ") " #s2, \
                      "Expected first %u chars of strings to be different. s1: \"%s\", s2: \"%s\"", (unsigned int)(n), (s1) ? (s1) : "NULL", (s2) ? (s2) : "NULL")

// **Expectativas de Punto Flotante**

/**
 * @def EXPECT_FLOAT_EQ(val1, val2)
 * @brief Expects that two float values are exactly equal.
 * @warning Direct comparison of floating-point values can be problematic. Consider EXPECT_FLOAT_NEAR.
 * If `val1` is not equal to `val2`, a failure is reported, but the test continues.
 * @param val1 The first float value.
 * @param val2 The second float value.
 */
#define EXPECT_FLOAT_EQ(val1, val2) \
    BMT_EXPECT_COMMON((val1) == (val2), "EXPECT_FLOAT_EQ", #val1 " == " #val2, \
                      "Expected: %g, Actual: %g", (double)(val1), (double)(val2))

/**
 * @def EXPECT_DOUBLE_EQ(val1, val2)
 * @brief Expects that two double values are exactly equal.
 * @warning Direct comparison of floating-point values can be problematic. Consider EXPECT_NEAR.
 * If `val1` is not equal to `val2`, a failure is reported, but the test continues.
 * @param val1 The first double value.
 * @param val2 The second double value.
 */
#define EXPECT_DOUBLE_EQ(val1, val2) \
    BMT_EXPECT_COMMON((val1) == (val2), "EXPECT_DOUBLE_EQ", #val1 " == " #val2, \
                      "Expected: %g, Actual: %g", (double)(val1), (double)(val2))

/**
 * @def EXPECT_NEAR(val1, val2, abs_error)
 * @brief Expects that two floating-point values (double) are close to each other.
 * The expectation passes if `fabs((val1) - (val2)) <= fabs(abs_error)`.
 * If the condition is not met, a failure is reported, but the test continues.
 * @param val1 The first double value.
 * @param val2 The second double value.
 * @param abs_error The maximum allowed absolute difference.
 */
#define EXPECT_NEAR(val1, val2, abs_error) \
    BMT_EXPECT_COMMON(fabs((val1) - (val2)) <= fabs(abs_error), "EXPECT_NEAR", #val1 " NEAR " #val2 ", error " #abs_error, \
                      "Value1: %g, Value2: %g, Diff: %g, Max Abs Error: %g", \
                      (double)(val1), (double)(val2), fabs((double)(val1) - (double)(val2)), fabs((double)(abs_error)))

/**
 * @def EXPECT_FLOAT_NEAR(val1, val2, abs_error)
 * @brief Expects that two float values are close to each other.
 * The expectation passes if `fabsf((val1) - (val2)) <= fabsf(abs_error)`.
 * If the condition is not met, a failure is reported, but the test continues.
 * @param val1 The first float value.
 * @param val2 The second float value.
 * @param abs_error The maximum allowed absolute difference (as a float).
 */
#define EXPECT_FLOAT_NEAR(val1, val2, abs_error) \
    BMT_EXPECT_COMMON(fabsf((val1) - (val2)) <= fabsf(abs_error), "EXPECT_FLOAT_NEAR", #val1 " NEAR " #val2 ", error " #abs_error, \
                      "Value1: %f, Value2: %f, Diff: %f, Max Abs Error: %f", \
                      (float)(val1), (float)(val2), fabsf((float)(val1) - (float)(val2)), fabsf((float)(abs_error)))

/**
 * @def RUN_ALL_TESTS()
 * @brief Macro to invoke the test runner to run all registered tests.
 * Typically called from the main function of the test executable.
 * @return The number of failed tests.
 */
#define RUN_ALL_TESTS() bmt_run_all_tests()

// (Opcional) Fixtures - MÃ¡s avanzado, podrÃ­a aÃ±adirse despuÃ©s.
// #define TEST_F(TestFixtureName, TestName) ...

#ifdef __cplusplus
}
#endif

#endif // BAREMETAL_TEST_H