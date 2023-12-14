#include "epub_toc_entry.h"
#include <stdlib.h>
#include <string.h>
#include <esp_log.h>

epub_toc_entry_t *epub_toc_entry_create(const char *title, const char *path)
{
    epub_toc_entry_t *entry = malloc(sizeof(epub_toc_entry_t));
    if (entry == NULL) {
        return NULL;
    }

    size_t length = strlen(title) + 1;
    entry->title = malloc(length);
    if (entry->title == NULL) {
        free(entry);
        return NULL;
    }
    strlcpy(entry->title, title, length);

    length = strlen(path) + 1;
    entry->path = malloc(length);
    if (entry->path == NULL) {
        free(entry->title);
        free(entry);
        return NULL;
    }
    strlcpy(entry->path, path, length);
    
    return entry;
}

void epub_toc_entry_destroy(epub_toc_entry_t *entry)
{
    if (entry == NULL) {
        return;
    }

    free(entry->title);
    free(entry->path);
    free(entry);
}
