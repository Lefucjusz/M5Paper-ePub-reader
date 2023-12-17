#include "gui_toc_list.h"
#include "lvgl.h"
#include <esp_log.h>

#include "gui_page.h"
#include "epub_section.h"

#define TAG "GUI-TOC"

typedef struct
{
    lv_obj_t *top_bar;
    lv_obj_t *toc_label;
    lv_obj_t *back_btn;
    lv_obj_t *back_btn_icon;
    lv_obj_t *toc_list;

    epub_t *current_epub;
} gui_toc_list_ctx;

static gui_toc_list_ctx ctx;

/* Private function prototypes */
static void gui_toc_item_click_callback(lv_event_t *event);
static void gui_back_button_click_callback(lv_event_t *event);

/* Public functions */
void gui_toc_list_create(epub_t *epub)
{
    /* Store current epub */
    ctx.current_epub = epub;

    /* Create top bar */
    ctx.top_bar = lv_obj_create(lv_scr_act());
    lv_obj_set_size(ctx.top_bar, GUI_TOC_BAR_WIDTH, GUI_TOC_BAR_HEIGHT);
    lv_obj_align(ctx.top_bar, LV_ALIGN_TOP_MID, 0, GUI_TOC_BAR_OFFSET_Y);
    lv_obj_set_style_border_side(ctx.top_bar, LV_BORDER_SIDE_BOTTOM, LV_PART_MAIN);
    lv_obj_clear_flag(ctx.top_bar, LV_OBJ_FLAG_SCROLLABLE);

    /* Add list label */
    ctx.toc_label = lv_label_create(ctx.top_bar);
    lv_label_set_text(ctx.toc_label, "Table of contents");
    lv_obj_set_style_text_font(ctx.toc_label, &lv_font_montserrat_28, LV_PART_MAIN);
    lv_obj_center(ctx.toc_label);

    /* Add back button */
    ctx.back_btn = lv_btn_create(ctx.top_bar);
    lv_obj_set_style_border_side(ctx.back_btn, LV_BORDER_SIDE_NONE, LV_PART_MAIN);
    lv_obj_set_height(ctx.back_btn, 50); // TODO define
    lv_obj_align(ctx.back_btn, LV_ALIGN_LEFT_MID, GUI_TOC_BACK_BUTTON_OFFSET_X, 0);
    lv_obj_add_event_cb(ctx.back_btn, gui_back_button_click_callback, LV_EVENT_CLICKED, NULL);
    
    /* Add back button icon */
    ctx.back_btn_icon = lv_label_create(ctx.back_btn);
    lv_label_set_text(ctx.back_btn_icon, LV_SYMBOL_BACKSPACE);
    lv_obj_set_style_text_font(ctx.back_btn_icon, &lv_font_montserrat_44, LV_PART_MAIN);
    lv_obj_center(ctx.back_btn_icon);

    /* Create a list */
    ctx.toc_list = lv_list_create(lv_scr_act());
    lv_obj_set_size(ctx.toc_list, GUI_TOC_LIST_WIDTH, GUI_TOC_LIST_HEIGHT);
    lv_obj_align_to(ctx.toc_list, ctx.top_bar, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
    lv_obj_clear_flag(ctx.toc_list, LV_OBJ_FLAG_SCROLL_ELASTIC);

    /* Fill the list with TOC items */
    const vec_void_t *toc = epub_get_toc(ctx.current_epub);
    size_t i;
    epub_toc_entry_t *entry;
    vec_foreach(toc, entry, i) {
        lv_obj_t *list_btn = lv_list_add_btn(ctx.toc_list, LV_SYMBOL_LIST, entry->title); // TODO change to some more appropriate symbol
        lv_obj_add_event_cb(list_btn, gui_toc_item_click_callback, LV_EVENT_CLICKED, (void *)entry);
        lv_obj_set_style_text_font(list_btn, &lv_font_montserrat_36, LV_PART_MAIN);
        lv_obj_set_style_pad_top(list_btn, GUI_TOC_LIST_BUTTON_PAD_TOP, LV_PART_MAIN);
        lv_obj_set_style_pad_bottom(list_btn, GUI_TOC_LIST_BUTTON_PAD_BOTTOM, LV_PART_MAIN);
        lv_obj_t *list_btn_label = lv_obj_get_child(list_btn, lv_obj_get_child_cnt(list_btn) - 1); // Label is created as the last child
        lv_label_set_long_mode(list_btn_label, LV_LABEL_LONG_WRAP); // Disable scrolling, just wrap the text
    }
}

/* Private function definitions */
static void gui_toc_item_click_callback(lv_event_t *event)
{
    // ESP_LOGW(TAG, "Not supported yet!");

    epub_toc_entry_t *entry = lv_event_get_user_data(event);

    ESP_LOGI(TAG, "Entry title: %s", entry->title);
    ESP_LOGI(TAG, "Entry href: %s", entry->path);
    ESP_LOGI(TAG, "Entry spine index: %zu", epub_get_spine_entry_index(ctx.current_epub, entry->path));
    char *content = epub_get_section_content(ctx.current_epub, epub_get_spine_entry_index(ctx.current_epub, entry->path));
    // ESP_LOGI(TAG, "Entry content: %s", content);
    // gui_page_create();
    // content[4096] = '\0';
    // gui_page_update("");
    vec_void_t *section = epub_section_parse(content);
    free(content);
    gui_page_create(section);
}

static void gui_back_button_click_callback(lv_event_t *event)
{
    epub_close(ctx.current_epub);

    lv_obj_del(ctx.top_bar);
    lv_obj_del(ctx.toc_list);
}
