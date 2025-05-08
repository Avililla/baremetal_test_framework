/**
 * @file main_tests.c
 * @brief Archivo principal de ejemplos que demuestra el uso del framework Bare-Metal Test (BMT).
 *
 * Este archivo contiene varias suites de pruebas y casos de prueba para ejercitar
 * las funcionalidades del framework BMT y para probar un conjunto de operaciones matemáticas
 * y de manejo de cadenas de ejemplo.
 */

#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"
#include "baremetal_test.h"
#include "mathoperations.h"
#include <string.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>

/**
 * @brief Suite de pruebas para operaciones matemáticas básicas.
 */
TEST(BasicMath, Addition) {
    ASSERT_EQ(add(2, 2), 4);
    ASSERT_EQ(add(-1, 1), 0);
    ASSERT_EQ(add(0, 0), 0);
    ASSERT_NE(add(2, 3), 4);
}

/**
 * @brief Prueba la funcionalidad de resta.
 */
TEST(BasicMath, Subtraction) {
    ASSERT_EQ(subtract(5, 3), 2);
    ASSERT_EQ(subtract(3, 5), -2);
    ASSERT_EQ(subtract(0, 0), 0);
    EXPECT_EQ(subtract(10, 5), 5);
}

/**
 * @brief Prueba la funcionalidad de multiplicación.
 */
TEST(BasicMath, Multiplication) {
    ASSERT_EQ(multiply(3, 4), 12);
    ASSERT_EQ(multiply(-2, 5), -10);
    ASSERT_EQ(multiply(7, 0), 0);
    ASSERT_EQ(multiply(-3, -3), 9);
}

/**
 * @brief Prueba la determinación de números pares.
 */
TEST(BasicMath, IsEven) {
    ASSERT_TRUE(is_even(2));
    ASSERT_TRUE(is_even(0));
    ASSERT_TRUE(is_even(-4));
    ASSERT_FALSE(is_even(3));
    ASSERT_FALSE(is_even(-1));
}

#ifndef BMT_NO_FLOAT_TESTS

/**
 * @brief Suite de pruebas para operaciones matemáticas con punto flotante.
 * @note Estas pruebas solo se compilan si BMT_NO_FLOAT_TESTS no está definido.
 */
TEST(FloatingPointMath, Division) {
    const float epsilon = 0.00001f;
    ASSERT_NEAR(divide(10.0f, 2.0f), 5.0f, epsilon);
    ASSERT_NEAR(divide(1.0f, 3.0f), 0.33333f, epsilon);
    ASSERT_NEAR(divide(-5.0f, 2.0f), -2.5f, epsilon);

    #if defined(isnan) || defined(__GNUC__)
        ASSERT_TRUE(isnan(divide(1.0f, 0.0f)));
    #else
        bmt_platform_puts("NOTE: isnan not available for FloatingPointMath.Division NaN test.\r\n");
    #endif
}

/**
 * @brief Prueba comparaciones adicionales de punto flotante, incluyendo la precisión.
 */
TEST(FloatingPointMath, MoreComparisons) {
    float f1 = 0.1f + 0.2f;
    float f2 = 0.3f;
    const float small_epsilon = 1e-6f;

    ASSERT_NEAR(f1, f2, small_epsilon);
    EXPECT_NEAR(1.0f / 7.0f, 0.142857f, small_epsilon);
}

#endif

/**
 * @brief Suite de pruebas para operaciones con cadenas.
 */
TEST(StringOperations, StaticString) {
    const char* str = get_static_string();
    ASSERT_NOT_NULL(str);
    ASSERT_TRUE(strlen(str) > 0);
    bmt_platform_puts("NOTE: strcmp not available for StringOperations.StaticString full check.\r\n");
}

/**
 * @brief Prueba la función que copia una cadena a un buffer proporcionado.
 */
TEST(StringOperations, DynamicString) {
    char buffer[50];
    const char* source = "Test String";
    char* result = get_dynamic_string(buffer, source);

    ASSERT_NOT_NULL(result);
    ASSERT_EQ(result, buffer);

    ASSERT_TRUE(strlen(result) == strlen(source));
    bmt_platform_puts("NOTE: strcmp not available for StringOperations.DynamicString full check.\r\n");

    ASSERT_NULL(get_dynamic_string(NULL, source));
    ASSERT_NULL(get_dynamic_string(buffer, NULL));
}

/**
 * @brief Suite de pruebas para operaciones con punteros y memoria.
 */
TEST(PointerAndMemory, CreateArray) {
    #ifdef BMT_HAS_MALLOC
    int size = 5;
    int* arr = create_array(size);
    ASSERT_NOT_NULL(arr);
    if (arr != NULL) {
        for (int i = 0; i < size; ++i) {
            ASSERT_EQ(arr[i], i * 10);
        }
        free(arr);
    }

    ASSERT_NULL(create_array(0));
    ASSERT_NULL(create_array(-1));
    #else
    bmt_platform_puts("NOTE: BMT_HAS_MALLOC not defined, skipping PointerAndMemory.CreateArray dynamic parts.\r\n");
    ASSERT_NULL(create_array(5));
    ASSERT_NULL(create_array(0));
    ASSERT_NULL(create_array(-1));
    #endif
}

/**
 * @brief Suite de pruebas para lógica más compleja, como la determinación de números primos.
 */
TEST(ComplexLogic, IsPrimeBasic) {
    ASSERT_FALSE(is_prime(0));
    ASSERT_FALSE(is_prime(1));
    ASSERT_TRUE(is_prime(2));
    ASSERT_TRUE(is_prime(3));
    ASSERT_FALSE(is_prime(4));
    ASSERT_TRUE(is_prime(5));
    ASSERT_FALSE(is_prime(6));
    ASSERT_TRUE(is_prime(7));
}

/**
 * @brief Prueba la función is_prime con casos más avanzados y números más grandes.
 */
TEST(ComplexLogic, IsPrimeAdvanced) {
    EXPECT_TRUE(is_prime(13));
    EXPECT_TRUE(is_prime(29));
    EXPECT_TRUE(is_prime(97));
    EXPECT_FALSE(is_prime(100));
    EXPECT_FALSE(is_prime(81));
}

/**
 * @brief Suite de pruebas para funciones con errores intencionales o casos borde.
 */
TEST(EdgeCasesAndBugs, PotentiallyBuggyFunction_ValidInput) {
    int output;
    potentially_buggy_function(10, &output);
    ASSERT_EQ(output, 20);

    potentially_buggy_function(1, &output);
    ASSERT_EQ(output, 2);
}

/**
 * @brief Prueba el comportamiento de potentially_buggy_function con un puntero de salida nulo.
 * Se espera que la función no falle.
 */
TEST(EdgeCasesAndBugs, PotentiallyBuggyFunction_NullPointer) {
    potentially_buggy_function(50, NULL);
    SUCCEED();
}

/**
 * @brief Prueba un bug intencional en potentially_buggy_function cuando la entrada es cero.
 */
TEST(EdgeCasesAndBugs, PotentiallyBuggyFunction_InputZero) {
    int output = 123;
    potentially_buggy_function(0, &output);
    ASSERT_EQ(output, 123);
    ADD_FAILURE();
}

/**
 * @brief Prueba potentially_buggy_function con una entrada grande.
 */
TEST(EdgeCasesAndBugs, PotentiallyBuggyFunction_LargeInput) {
    int output;
    potentially_buggy_function(200, &output);
    ASSERT_EQ(output, 200);
}

/**
 * @brief Suite de pruebas para demostrar las capacidades de fallo del framework.
 */
TEST(FrameworkDemo, IntentionallyFailingAssert) {
    ASSERT_EQ(1, 0);
    bmt_platform_puts("Esta línea NUNCA se imprimirá.\r\n");
}

/**
 * @brief Demuestra el comportamiento de las macros EXPECT que reportan fallos pero continúan la ejecución del test.
 */
TEST(FrameworkDemo, IntentionallyFailingExpect) {
    EXPECT_EQ(1, 0);
    bmt_platform_puts("Esta línea SÍ se imprimirá después de EXPECT.\r\n");
    EXPECT_TRUE(0 > 1);
    ASSERT_EQ(5, 5);
}

/**
 * @brief Demuestra el uso de múltiples macros EXPECT en un solo test.
 */
TEST(FrameworkDemo, MultipleExpects) {
    EXPECT_LT(10, 100);
    EXPECT_NE(5, 6);
    EXPECT_STREQ("hello", "hello");
    EXPECT_NULL(NULL);
}

/**
 * @brief Función principal para ejecutar todas las pruebas.
 *
 * Inicializa la plataforma, ejecuta todas las pruebas registradas utilizando el framework BMT,
 * e imprime un resumen de los resultados.
 * @return 0 si todas las pruebas pasan, un número distinto de cero en caso contrario.
 */
int main()
{
    init_platform();
    int ret = RUN_ALL_TESTS();
    if (ret == 0) {
        bmt_platform_puts("ALL TESTS PASSED\r\n");
    } else {
        char buf[30];
        bmt_platform_puts(buf);
        bmt_platform_puts(" TESTS FAILED\r\n");
    }
    cleanup_platform();
    return 0;
}
