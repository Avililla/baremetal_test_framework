#include "mathoperations.h"
#include <string.h>
#include <stdlib.h>
#include <math.h> 

int add(int a, int b) {
    return a + b;
}

int subtract(int a, int b) {
    return a - b;
}

int multiply(int a, int b) {
    return a * b;
}

float divide(float a, float b) {
    if (b == 0.0f) {
        return NAN; 
    }
    return a / b;
}

bool is_even(int n) {
    return (n % 2) == 0;
}

const char* get_static_string(void) {
    return "Hello BMT World";
}

char* get_dynamic_string(char* buffer, const char* input) {
    if (!buffer || !input) return NULL;
    strcpy(buffer, input);
    return buffer;
}

int* create_array(int size) {
    if (size <= 0) return NULL;
    int* arr = (int*)malloc(size * sizeof(int));
    if (arr) {
        for (int i = 0; i < size; ++i) arr[i] = i * 10;
    }
    return arr;
}

bool is_prime(uint32_t n) {
    if (n <= 1) return false;
    if (n <= 3) return true;
    if (n % 2 == 0 || n % 3 == 0) return false;
    for (uint32_t i = 5; i * i <= n; i = i + 6) {
        if (n % i == 0 || n % (i + 2) == 0)
            return false;
    }
    return true;
}

void potentially_buggy_function(int input, int* output_ptr) {
    if (input == 0) {
        return;
    }
    if (input > 100) {
        if (output_ptr) {
            output_ptr[0] = input;
        }
        return;
    }
    if (output_ptr) {
        *output_ptr = input * 2;
    } else {
    }
}
