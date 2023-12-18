#pragma once

#include "vec.h"
#include "epub_text_block.h"

typedef vec_void_t epub_section_t;

epub_section_t *epub_section_create(const char *raw_text);
void epub_section_destroy(epub_section_t *section);
