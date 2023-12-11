#include "gui_files_list.h"
#include "lvgl.h"
#include "dir.h"
#include <esp_log.h>

struct gui_files_list_ctx_t
{
    vector_t *dirs;
    lv_obj_t *list_files;
};

static struct gui_files_list_ctx_t ctx;

/* Private function declarations */
static bool is_directory(struct dirent *entry);
static void on_directory_up_click(lv_event_t *event);
static void on_directory_click(lv_event_t *event);
static void reload_list(void);

/* Public functions */
void gui_files_list_create(void)
{
    ctx.list_files = lv_list_create(lv_scr_act());
    lv_obj_set_size(ctx.list_files, GUI_FILES_LIST_WIDTH, GUI_FILES_LIST_HEIGHT);
    lv_obj_align(ctx.list_files, LV_ALIGN_TOP_MID, 0, GUI_FILES_LIST_OFFSET_X);
    lv_obj_clear_flag(ctx.list_files, LV_OBJ_FLAG_SCROLL_ELASTIC);
    reload_list();
}

/* Private function definitions */
static bool is_directory(struct dirent *entry) 
{
    return (entry->d_type == DT_DIR);
}

static void on_directory_up_click(lv_event_t *event)
{
    const int ret = dir_return();
	if (ret != 0) {
		return;
	}
	reload_list();
}

static void on_directory_click(lv_event_t *event)
{
    const char *filename = (const char *)lv_event_get_user_data(event);
	const int ret = dir_enter(filename);
	if (ret != 0) {
		return;
	}
	reload_list();
}

static void reload_list(void)
{
    /* Delete current list */
	dir_list_free(ctx.dirs);

	/* Delete current list contents */
	lv_obj_clean(ctx.list_files);

	/* Create new files list */
	ctx.dirs = dir_list();

	/* Create directory up button */
	lv_obj_t *button;
	if (!dir_is_top()) {
		button = lv_list_add_btn(ctx.list_files, NULL, "..");
        lv_obj_set_style_text_font(button, &lv_font_montserrat_36, LV_PART_MAIN);
        lv_obj_set_style_pad_top(button, 20, LV_PART_MAIN);
        lv_obj_set_style_pad_bottom(button, 20, LV_PART_MAIN);
		lv_obj_add_event_cb(button, on_directory_up_click, LV_EVENT_CLICKED, NULL);
	}

	size_t i;
    struct dirent *entry;
    vec_foreach(ctx.dirs, entry, i) {
        lv_obj_t *list_btn;
        if (is_directory(entry)) {
            list_btn = lv_list_add_btn(ctx.list_files, LV_SYMBOL_DIRECTORY, entry->d_name);
            lv_obj_add_event_cb(list_btn, on_directory_click, LV_EVENT_CLICKED, (void *)entry->d_name);
        }
        else {
            list_btn = lv_list_add_btn(ctx.list_files, LV_SYMBOL_FILE, entry->d_name);
            // lv_obj_add_event_cb(list_btn, on_supported_file_click, LV_EVENT_CLICKED, (void *)entry->d_name);
        }
        lv_obj_set_style_text_font(list_btn, &lv_font_montserrat_36, LV_PART_MAIN);
        lv_obj_set_style_pad_top(list_btn, 20, LV_PART_MAIN);
        lv_obj_set_style_pad_bottom(list_btn, 20, LV_PART_MAIN); // TODO hardcoded values
        lv_obj_t *list_btn_label = lv_obj_get_child(list_btn, lv_obj_get_child_cnt(list_btn) - 1); // Label is created as a last child
        lv_label_set_long_mode(list_btn_label, LV_LABEL_LONG_WRAP); // Disable scrolling, just wrap the text
    }
}
