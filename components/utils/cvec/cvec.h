/* CVec - Copying Vector - extension of rxi's vec library,
 * allowing to create generic vector containing copies of 
 * the inserted objects, not just referencing the 
 * originals. */

#pragma once

#include "vec.h"
#include <stdbool.h>

void cvec_create(vec_void_t *vec);
void cvec_destroy(vec_void_t *vec);

bool cvec_push(vec_void_t *vec, const void *data, size_t size);
bool cvec_push_string(vec_void_t *vec, const char *str);
