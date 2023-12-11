#pragma once

#include "vector.h"
#include <stdbool.h>
#include <dirent.h>

void dir_init(const char *root_path);

/* Returns true if currently at top directory */
bool dir_is_top(void);

int dir_enter(const char *name);
int dir_return(void);

const char *dir_get_fs_path(void);

vector_t *dir_list(void);
void dir_list_free(vector_t *list);
