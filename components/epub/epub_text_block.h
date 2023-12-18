#pragma once

typedef enum
{
    EPUB_FONT_NORMAL,
    EPUB_FONT_BOLD
} epub_font_type_t;

typedef struct
{
    char *text;
    epub_font_type_t font_type;
} epub_text_block_t;

epub_text_block_t *epub_text_block_create(const char *text, epub_font_type_t font_type);
void epub_text_block_destroy(epub_text_block_t *block);
