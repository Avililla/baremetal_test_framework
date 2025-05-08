// include/bmt_platform_io.h
// SPDX-License-Identifier: MIT
// Copyright (c) 2025 Alejandro Avila Marcos
//
// Este archivo es parte de la librería Bare-Metal Test Framework (BMT).
// BMT se distribuye bajo los términos de la Licencia MIT.
// Puedes encontrar una copia de la licencia en el archivo LICENSE.txt
// o en <https://opensource.org/licenses/MIT>.

#ifndef BMT_PLATFORM_IO_H
#define BMT_PLATFORM_IO_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initializes the communication interface (e.g., UART).
 *        Called once by bmt_run_all_tests().
 * @note The user MUST implement this function.
 */
void bmt_platform_io_init(void);

/**
 * @brief Sends a single character through the communication interface.
 * @param c Character to send.
 * @note The user MUST implement this function.
 */
void bmt_platform_putchar(char c);

/**
 * @brief Sends a null-terminated string through the interface.
 * @param str String to send.
 * @note The user MUST implement this function.
 */
void bmt_platform_puts(const char *str);

/**
 * @brief Gets the current timestamp in milliseconds (or ticks).
 *        Used to measure test duration.
 * @return Current timestamp.
 * @note The user MUST implement this function. If timing is not needed, it can return 0.
 */
uint32_t bmt_platform_get_msec_ticks(void);

#ifdef __cplusplus
}
#endif

#endif // BMT_PLATFORM_IO_H