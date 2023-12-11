#pragma once

#include "map.h"
#include <stdbool.h>

/* Map of strings */
void map_str_create(map_str_t *map);
void map_str_destroy(map_str_t *map);

bool map_str_set(map_str_t *map, const char *key, const char *str);
const char *map_str_get(map_str_t *map, const char *key);

map_iter_t map_str_iter(map_str_t *map);
const char *map_str_next(map_str_t *map, map_iter_t *iter);
