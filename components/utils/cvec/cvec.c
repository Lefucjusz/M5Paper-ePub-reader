#include "cvec.h"

void cvec_create(vec_void_t *cvec)
{
    vec_init(cvec);
}

void cvec_destroy(vec_void_t *cvec)
{
    size_t i;
    void *item;
    vec_foreach(cvec, item, i) {
        free(item);
    }
    vec_deinit(cvec);
}

bool cvec_push(vec_void_t *cvec, const void *data, size_t size)
{
    void *item = malloc(size);
    if (item == NULL) {
        return false;
    }

    memcpy(item, data, size);
    vec_push(cvec, item);
    return true;
}

bool cvec_push_string(vec_void_t *cvec, const char *str) 
{
    const size_t size = strlen(str) + 1;
    return cvec_push(cvec, str, size);
}
