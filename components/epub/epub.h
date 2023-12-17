#pragma once

#include "vec.h"
#include "../third_party/miniz/miniz.h" // TODO fix this better way
#include "epub_toc_entry.h"

typedef enum
{
    EPUB_OK = 0,
    EPUB_INVALID_ARG = -1,
    EPUB_IO_ERROR = -2,
    EPUB_NO_MEMORY = -3,
    EPUB_NOT_FOUND = -4,
    EPUB_PARSING_ERROR = -5,
    EPUB_GENERAL_ERROR = -6
} epub_err_t;

typedef struct
{
    mz_zip_archive zip;
    vec_void_t spine;
    vec_void_t toc;
} epub_t;

epub_err_t epub_open(epub_t *epub, const char *path);
epub_err_t epub_close(epub_t *epub);

const vec_void_t *epub_get_toc(epub_t *epub);

ssize_t epub_get_spine_entry_index(epub_t *epub, const char *href);
ssize_t epub_get_spine_size(epub_t *epub);

char *epub_get_section_content(epub_t *epub, size_t spine_entry);
