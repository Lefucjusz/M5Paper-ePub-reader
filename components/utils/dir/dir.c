#include "dir.h"
#include <sys/syslimits.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

struct dir_ctx_t
{
	char path[PATH_MAX];
	size_t depth;
};

static struct dir_ctx_t dir_ctx;

/* Private function prototypes */
static int path_append(const char *name);
static int path_remove(void);

void dir_init(const char *root_path)
{
	strncpy(dir_ctx.path, root_path, sizeof(dir_ctx.path)-1); // TODO fix
	dir_ctx.depth = 0;
}

bool dir_is_top(void)
{
	return (dir_ctx.depth == 0);
}

int dir_enter(const char *name)
{
	const int ret = path_append(name);
	if (ret) {
		return ret;
	}
	dir_ctx.depth++;

	return 0;
}

int dir_return(void)
{
	const int ret = path_remove();
	if (ret) {
		return ret;
	}
	dir_ctx.depth--;

	return 0;
}

const char *dir_get_fs_path(void)
{
	return dir_ctx.path;
}

vector_t *dir_list(void)
{
	vector_t *list = malloc(sizeof(vector_t));
	if (list == NULL) {
		return NULL;
	}
	vector_create(list);

	struct dirent *entry;
    DIR *dir = opendir(dir_ctx.path);
    if (dir == NULL) {
        return NULL;
    }

    while ((entry = readdir(dir)) != NULL) {
		vector_push(list, entry, sizeof(*entry));
    }
    closedir(dir);

	// TODO add sorting

	return list;
}

void dir_list_free(vector_t *list)
{
	if (list == NULL) {
		return;
	}
	
	vector_destroy(list);
	free(list);
}

/* Private function definitions */
static int path_append(const char *name)
{
	const size_t cur_path_len = strlen(dir_ctx.path);
	const size_t space_left = sizeof(dir_ctx.path) - cur_path_len;
	if (strlen(name) >= space_left) {
		return -ENAMETOOLONG;
	}

	snprintf(dir_ctx.path + cur_path_len, space_left, "/%s", name);

	return 0;
}

static int path_remove(void)
{
	if (dir_ctx.depth == 0) {
		return -ENOENT;
	}

	char *last_slash = strrchr(dir_ctx.path, '/');
	*last_slash = '\0';

	return 0;
}
