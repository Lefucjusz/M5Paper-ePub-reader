#pragma once

#include "vec.h"
#include "../third_party/miniz/miniz.h" // TODO fix this better way
#include "epub_toc_entry.h"

typedef enum
{
    EPUB_OK,
    EPUB_INVALID_ARG,
    EPUB_IO_ERROR,
    EPUB_NO_MEMORY,
    EPUB_OUT_OF_RANGE,
    EPUB_PARSING_ERROR,
    EPUB_GENERAL_ERROR
} epub_err_t;

typedef enum 
{
    EPUB_PREVIOUS,
    EPUB_NEXT
} epub_direction_t;

typedef struct
{
    mz_zip_archive zip;
    vec_void_t spine;
    vec_void_t toc;
} epub_t;

epub_err_t epub_open(epub_t *epub, const char *path);
epub_err_t epub_close(epub_t *epub);

const vec_void_t *epub_get_toc(epub_t *epub);

// epub_err_t epub_get_section(epub_direction_t direction, void *section_data); // TODO section_data should be some proper type
