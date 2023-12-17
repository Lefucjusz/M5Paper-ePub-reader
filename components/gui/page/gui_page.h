#pragma once

#include "gui_dimensions.h"
#include "vec.h"

#define GUI_PAGE_MARGIN_TOP 2
#define GUI_PAGE_WIDTH GUI_MAIN_AREA_WIDTH
#define GUI_PAGE_HEIGHT (GUI_MAIN_AREA_HEIGHT - GUI_PAGE_MARGIN_TOP)
#define GUI_PAGE_OFFSET_Y (GUI_MAIN_AREA_OFFSET_Y + GUI_PAGE_MARGIN_TOP)

void gui_page_create(vec_void_t *section);
void gui_page_update(const char *text);
