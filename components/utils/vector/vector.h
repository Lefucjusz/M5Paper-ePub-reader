#pragma once

#include "vec.h"
#include <stdbool.h>

typedef vec_void_t vector_t;

void vector_create(vector_t *vec);
void vector_destroy(vector_t *vec);

bool vector_push(vector_t *vec, const void *data, size_t size);
bool vector_push_string(vector_t *vec, const char *str);
