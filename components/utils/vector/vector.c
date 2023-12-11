#include "vector.h"

void vector_create(vector_t *vec)
{
    vec_init(vec);
}

void vector_destroy(vector_t *vec)
{
    size_t i;
    void *item;
    vec_foreach(vec, item, i) {
        free(item);
    }
    vec_deinit(vec);
}

bool vector_push(vector_t *vec, const void *data, size_t size)
{
    void *vec_value = malloc(size);
    if (vec_value == NULL) {
        return false;
    }

    memcpy(vec_value, data, size);
    vec_push(vec, vec_value);
    return true;
}

bool vector_push_string(vector_t *vec, const char *str) 
{
    const size_t size = strlen(str) + 1;
    return vector_push(vec, str, size);
}
