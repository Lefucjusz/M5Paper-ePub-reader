#pragma once

typedef struct
{
    char *title;
    char *path;
} epub_toc_entry_t;

epub_toc_entry_t *epub_toc_entry_create(const char *title, const char *path);
void epub_toc_entry_destroy(epub_toc_entry_t *entry);
