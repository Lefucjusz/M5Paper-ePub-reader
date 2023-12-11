#pragma once

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

epub_err_t epub_open(const char *path);
epub_err_t epub_close(void);

epub_err_t epub_get_section(epub_direction_t direction, void *section_data); // TODO section_data should be some proper type
