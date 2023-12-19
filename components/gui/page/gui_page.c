#include "gui_page.h"
#include "gui_fonts.h"
#include <esp_log.h>

#define TAG "GUI_PAGE"

typedef enum
{
    GUI_PAGE_PREVIOUS,
    GUI_PAGE_NEXT
} gui_page_direction_t;

typedef struct
{
    lv_obj_t *page;
    epub_t *epub;
    epub_section_t *section;

    size_t spine_index;
    size_t block_index;
    size_t block_offset_bytes;
    size_t block_bytes_left;
} gui_page_ctx_t;

static gui_page_ctx_t ctx;

static lv_obj_t *gui_page_add_block(const char *text, epub_font_type_t font_type);
static ssize_t gui_page_get_first_excess_char_index(lv_obj_t *block);
static bool gui_page_is_book_end(void);
static bool gui_page_is_section_end(void);
static void gui_page_render(gui_page_direction_t direction);
static void gui_page_swipe_callback(lv_event_t *event);

void gui_page_create(epub_t *epub, size_t spine_index)
{
    /* Sanity check */
    if (epub == NULL) {
        return;
    }

    /* Initialize context */
    memset(&ctx, 0, sizeof(ctx));
    ctx.epub = epub;
    ctx.spine_index = spine_index;

    /* Create page object */
    ctx.page = lv_obj_create(lv_scr_act());
    lv_obj_set_size(ctx.page, GUI_PAGE_WIDTH, GUI_PAGE_HEIGHT);
    lv_obj_align(ctx.page, LV_ALIGN_TOP_MID, 0, GUI_PAGE_OFFSET_Y);
    lv_obj_set_style_pad_all(ctx.page, 0, LV_PART_MAIN);
    lv_obj_set_style_border_side(ctx.page, LV_BORDER_SIDE_NONE, LV_PART_MAIN);
    lv_obj_clear_flag(ctx.page, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(ctx.page, LV_OBJ_FLAG_GESTURE_BUBBLE);
    lv_obj_add_event_cb(ctx.page, gui_page_swipe_callback, LV_EVENT_GESTURE, NULL);

    /* Render the page */
    gui_page_render(GUI_PAGE_NEXT);
}

static lv_obj_t *gui_page_add_block(const char *text, epub_font_type_t font_type)
{
    /* Add new block */
    lv_obj_t *label = lv_label_create(ctx.page);
    lv_label_set_text(label, text);
    lv_obj_set_width(label, GUI_PAGE_WIDTH);
    lv_obj_set_style_pad_bottom(label, 0, LV_PART_MAIN); // Needed to maintain line spacing between blocks
    lv_obj_set_style_text_line_space(label, GUI_PAGE_LINE_SPACING, LV_PART_MAIN);
    switch (font_type) {
        case EPUB_FONT_NORMAL:
            lv_obj_set_style_text_font(label, &gui_montserrat_medium_28, LV_PART_MAIN);
            break;

        case EPUB_FONT_BOLD:
            lv_obj_set_style_text_font(label, &gui_montserrat_medium_36, LV_PART_MAIN);
            break;

        default:
            break;
    }
    
    /* Align to previous block if exists */
    if (lv_obj_get_child_cnt(ctx.page) > 1) {
        lv_obj_set_style_pad_top(label, GUI_PAGE_LINE_SPACING, LV_PART_MAIN); // Maintain line spacing between blocks
        const size_t prev_label_index = lv_obj_get_child_cnt(ctx.page) - 2;
        lv_obj_t *prev_label = lv_obj_get_child(ctx.page, prev_label_index);
        lv_obj_align_to(label, prev_label, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
    }

    /* Force update coordinates */
    lv_obj_update_layout(ctx.page);

    return label;
}

static ssize_t gui_page_get_first_excess_char_index(lv_obj_t *block) // TODO this algorithm doesn't work every time
{
    lv_area_t area;
    lv_obj_get_coords(block, &area);

    ESP_LOGI(TAG, "Last label position (%d; %d)", area.x1, area.y1);
    ESP_LOGI(TAG, "Last label position end (%d; %d)", area.x2, area.y2);

    const size_t block_end_y = area.y2;
    if (block_end_y <  GUI_MAIN_AREA_MAX_Y) {
        return -1;
    }

    const size_t x_excess = ((GUI_MAIN_AREA_MIN_X - area.x1) >= 0) ? (GUI_MAIN_AREA_MIN_X - area.x1) : 0;
    const size_t y_excess = ((GUI_MAIN_AREA_MAX_Y - area.y1) >= 0) ? (GUI_MAIN_AREA_MAX_Y - area.y1) : 0;
    lv_point_t point_excess = {
        .x = x_excess,
        .y = y_excess
    };

    if (lv_label_is_char_under_pos(block, &point_excess)) {
        ESP_LOGI(TAG, "Detected excess char at (%d; %d) -> %c (%lu)!", point_excess.x, point_excess.y, lv_label_get_text(block)[lv_label_get_letter_on(block, &point_excess)], lv_label_get_letter_on(block, &point_excess));
        return lv_label_get_letter_on(block, &point_excess);
    }
    return -1;
}

static bool gui_page_is_book_end(void)
{
    return (ctx.spine_index >= ctx.epub->spine.length);
}

static bool gui_page_is_section_end(void)
{
    return (ctx.block_index >= ctx.section->length);
}

static void gui_page_render(gui_page_direction_t direction)
{
    /* Clear page before drawing new one */
    lv_obj_clean(ctx.page);

    /* Reached end of book */
    if (gui_page_is_book_end()) {
        ESP_LOGW(TAG, "Detected end of book! Not supported yet");
        return;
    }

    /* Get new section if not initialized yet or at end of previous one */
    if ((ctx.section == NULL) || gui_page_is_section_end()) {
        epub_section_destroy(ctx.section);
        ctx.section = epub_get_section(ctx.epub, ctx.spine_index);

        ESP_LOGI(TAG, "Reading section %zu/%zu", ctx.spine_index, ctx.epub->spine.length);

        // ctx.spine_index++;
        ctx.block_index = 0;
        ctx.block_offset_bytes = 0;
        const epub_text_block_t *epub_block = ctx.section->data[0];
        ctx.block_bytes_left = strlen(epub_block->text); // Get length of first block

        ESP_LOGI(TAG, "Initial block_bytes_left: %zu", ctx.block_bytes_left);
    }

    /* Render new page */
    bool page_full = false;
    do {
        const epub_text_block_t *epub_block = ctx.section->data[ctx.block_index];
        char *text = &epub_block->text[ctx.block_offset_bytes];

        lv_obj_t *new_gui_block = gui_page_add_block(text, epub_block->font_type);
        const ssize_t excess_char_index = gui_page_get_first_excess_char_index(new_gui_block);

        if (excess_char_index < 0) { // Entire block fits on page
            ctx.block_bytes_left = 0;
        }
        else if (excess_char_index == 0) { // Not a single char fits
            lv_obj_del(new_gui_block);
            page_full = true;
        }
        else if (excess_char_index > 0) { // Block fits partially
            lv_obj_del(new_gui_block);

            /* Convert char index to byte offset */
            const size_t bytes_to_add = _lv_txt_encoded_get_byte_id(text, excess_char_index);

            /* Temporarily terminate text array at last fitting char, add block to page and un-terminate */
            const char excess_byte = text[bytes_to_add];
            text[bytes_to_add] = '\0';
            gui_page_add_block(text, epub_block->font_type);
            text[bytes_to_add] = excess_byte;

            /* Update variables */
            ctx.block_offset_bytes += bytes_to_add;
            ctx.block_bytes_left -= bytes_to_add;
            page_full = true;
        }

        if (ctx.block_bytes_left == 0) {
            ctx.block_index++;
            if (gui_page_is_section_end()) {
                ESP_LOGI(TAG, "Reached end of section %zu", ctx.spine_index);
                ctx.spine_index++;
                break; // Stop rendering loop
            }
            else {
                ctx.block_offset_bytes = 0;
                const epub_text_block_t *next_epub_block = ctx.section->data[ctx.block_index];
                ctx.block_bytes_left = strlen(next_epub_block->text);
            }
        }
    } while (!page_full);
}

static void gui_page_swipe_callback(lv_event_t *event)
{
    ESP_LOGI(TAG, "Received swipe gesture %d!", lv_indev_get_gesture_dir(lv_indev_get_act()));
    gui_page_render(GUI_PAGE_NEXT);
}
