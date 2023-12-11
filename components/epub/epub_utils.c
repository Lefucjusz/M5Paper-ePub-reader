#include "epub_utils.h"
#include <esp_log.h>

/* Map of strings */
void map_str_create(map_str_t *map) 
{
    map_init(map);
}

void map_str_destroy(map_str_t *map)
{
    map_iter_t it = map_iter(map);
    const char *key;
    while ((key = map_next(map, &it)) != NULL) {
        free(*map_get(map, key));
    }
    map_deinit(map);
}

bool map_str_set(map_str_t *map, const char *key, const char *str) 
{
    const size_t size = strlen(str) + 1;
    char *map_value = malloc(size);
    if (map_value == NULL) {
        return false;
    }

    memcpy(map_value, str, size);
    map_set(map, key, map_value);
    return true;
}

const char *map_str_get(map_str_t *map, const char *key)
{
    char **value = map_get(map, key);
    if (value == NULL) {
        return NULL;
    }
    return *value;
}

map_iter_t map_str_iter(map_str_t *map)
{
    return map_iter(map);
}

const char *map_str_next(map_str_t *map, map_iter_t *iter)
{
    return map_next(map, iter);
}

/* Vector of strings */
void vec_str_create(vec_str_t *vec)
{
    vec_init(vec);
}

void vec_str_destroy(vec_str_t *vec)
{
    size_t i;
    char *item;
    vec_foreach(vec, item, i) {
        free(item);
    }
    vec_deinit(vec);
}

bool vec_str_push(vec_str_t *vec, const char *str)
{
    const size_t size = strlen(str) + 1;
    char *vec_value = malloc(size);
    if (vec_value == NULL) {
        return false;
    }

    memcpy(vec_value, str, size);
    vec_push(vec, vec_value);
    return true;
}
