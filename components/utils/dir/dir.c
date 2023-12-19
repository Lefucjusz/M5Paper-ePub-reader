#include "dir.h"
#include <sys/syslimits.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <esp_log.h>

typedef struct
{
	char path[PATH_MAX];
	size_t depth;
} dir_ctx_t;

static dir_ctx_t ctx;

static int path_append(const char *name);
static int path_remove(void);
static int path_ascending_comparator(const void *p, const void *q);


void dir_init(const char *root_path)
{
	strlcpy(ctx.path, root_path, sizeof(ctx.path));
	ctx.depth = 0;
}

bool dir_is_top(void)
{
	return (ctx.depth == 0);
}

int dir_enter(const char *name)
{
	const int ret = path_append(name);
	if (ret) {
		return ret;
	}
	ctx.depth++;

	return 0;
}

int dir_return(void)
{
	const int ret = path_remove();
	if (ret) {
		return ret;
	}
	ctx.depth--;

	return 0;
}

const char *dir_get_fs_path(void)
{
	return ctx.path;
}

dir_list_t *dir_list(void)
{
	dir_list_t *list = malloc(sizeof(dir_list_t));
	if (list == NULL) {
		return NULL;
	}
	cvec_create(list);

	struct dirent *entry;
    DIR *dir = opendir(ctx.path);
    if (dir == NULL) {
        return NULL;
    }

    while ((entry = readdir(dir)) != NULL) {
		cvec_push(list, entry, sizeof(*entry));
    }
    closedir(dir);

	vec_sort(list, path_ascending_comparator);

	return list;
}

void dir_list_free(dir_list_t *list)
{
	if (list == NULL) {
		return;
	}
	
	cvec_destroy(list);
	free(list);
}


static int path_append(const char *name)
{
	const size_t cur_path_len = strlen(ctx.path);
	const size_t space_left = sizeof(ctx.path) - cur_path_len;
	if (strlen(name) >= space_left) {
		return -ENAMETOOLONG;
	}

	snprintf(ctx.path + cur_path_len, space_left, "/%s", name);

	return 0;
}

static int path_remove(void)
{
	if (ctx.depth == 0) {
		return -ENOENT;
	}

	char *last_slash = strrchr(ctx.path, '/');
	*last_slash = '\0';

	return 0;
}

static int path_ascending_comparator(const void *p, const void *q)
{
	const struct dirent *entry_p = *(struct dirent **)p;
	const struct dirent *entry_q = *(struct dirent **)q;

	return strcmp(entry_p->d_name, entry_q->d_name); // TODO fix sorting numeric values
}
