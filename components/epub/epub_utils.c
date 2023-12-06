#include "epub_utils.h"
#include <esp_log.h>

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

void map_str_set(map_str_t *map, const char *key, const char *str) 
{
    const size_t size = strlen(str) + 1;
    char *map_value = malloc(size); // TODO error handling

    memcpy(map_value, str, size);
    map_set(map, key, map_value);
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
