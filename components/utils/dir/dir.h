#pragma once

#include "cvec.h"
#include <stdbool.h>
#include <dirent.h>

typedef vec_void_t dir_list_t;

void dir_init(const char *root_path);

/* Returns true if currently at top directory */
bool dir_is_top(void);

int dir_enter(const char *name);
int dir_return(void);

const char *dir_get_fs_path(void);

dir_list_t *dir_list(void);
void dir_list_free(dir_list_t *list);
