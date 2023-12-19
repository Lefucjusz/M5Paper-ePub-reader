#pragma once

#include "gui_dimensions.h"
#include "epub.h"

#define GUI_PAGE_MARGIN_TOP 2
#define GUI_PAGE_WIDTH GUI_MAIN_AREA_WIDTH
#define GUI_PAGE_HEIGHT (GUI_MAIN_AREA_HEIGHT - GUI_PAGE_MARGIN_TOP)
#define GUI_PAGE_OFFSET_Y (GUI_MAIN_AREA_MIN_Y + GUI_PAGE_MARGIN_TOP)

#define GUI_PAGE_LINE_SPACING 5

#define GUI_PAGE_END_MSG "Reached the end of the book"

void gui_page_create(epub_t *epub, size_t spine_index);
