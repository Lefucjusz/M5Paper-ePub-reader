#include "epub_text_block.h"

#include <string.h>

epub_text_block_t *epub_text_block_create(const char *text, epub_font_type_t font_type)
{
    epub_text_block_t *block = malloc(sizeof(epub_text_block_t));
    if (block == NULL) {
        return NULL;
    }

    const size_t text_size = strlen(text) + 1;
    block->text = malloc(text_size);
    if (block->text == NULL) {
        free(block);
        return NULL;
    }
    memcpy(block->text, text, text_size);

    block->font_type = font_type;

    return block;
}

void epub_text_block_destroy(epub_text_block_t *block)
{
    free(block->text);
    free(block);
}
