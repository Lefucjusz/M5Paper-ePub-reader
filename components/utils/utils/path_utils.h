#pragma once

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

inline static bool is_extension(const char *filename, const char *ext)
{
    const char *dot_ptr = strrchr(filename, '.');
    return ((dot_ptr != NULL) && (strcasecmp(dot_ptr, ext) == 0));
}

inline static char *path_concatenate(const char *path_1, const char *path_2) 
{
    const size_t length_1 = strlen(path_1);
    const size_t length_2 = strlen(path_2);
    char *result;

    if (length_1 == 0) {
        result = malloc(length_2 + 1);
        if (result == NULL) {
            return NULL;
        }
        strcpy(result, path_2);
    }
    else if (length_2 == 0) {
        result = malloc(length_1 + 1);
        if (result == NULL) {
            return NULL;
        }
        strcpy(result, path_1);
    }
    else {
        result = malloc(length_1 + length_2 + 2);
        if (result == NULL) {
            return NULL;
        }
        sprintf(result, "%s/%s", path_1, path_2);
    }

    return result;
}
