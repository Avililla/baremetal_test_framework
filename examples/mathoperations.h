// math_operations.h (o directamente en tu archivo de tests si son simples)
#ifndef MATH_OPERATIONS_H
#define MATH_OPERATIONS_H

#include <stdint.h>
#include <stdbool.h> // Para bool en is_prime

int add(int a, int b);
int subtract(int a, int b);
int multiply(int a, int b);
float divide(float a, float b); // Para probar flotantes
bool is_even(int n);
const char* get_static_string(void);
char* get_dynamic_string(char* buffer, const char* input); // Simula una función que escribe en un buffer
int* create_array(int size); // Simula asignación dinámica (cuidado en bare-metal)
bool is_prime(uint32_t n);   // Para un test un poco más complejo
void potentially_buggy_function(int input, int* output_ptr);

#endif // MATH_OPERATIONS_H
