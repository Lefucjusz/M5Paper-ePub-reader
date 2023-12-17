#pragma once

#include "vec.h"

typedef enum
{
    EPUB_FONT_NORMAL,
    EPUB_FONT_BOLD
} epub_font_type_t;

typedef struct
{
    char *data;
    epub_font_type_t type;
} epub_paragraph_t;

vec_void_t *epub_section_parse(const char *raw);
void epub_section_destroy(vec_void_t *section);
