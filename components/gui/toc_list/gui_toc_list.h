#pragma once

#include "gui_dimensions.h"
#include "gui_status_bar.h" // TODO this should not be included here...
#include "epub.h"

#define GUI_TOC_BAR_MARGIN_TOP 2
#define GUI_TOC_BAR_WIDTH GUI_SCREEN_WIDTH
#define GUI_TOC_BAR_HEIGHT 60
#define GUI_TOC_BAR_OFFSET_X (GUI_STATUS_BAR_HEIGHT + GUI_HORIZONTAL_MARGIN_TOP + GUI_TOC_BAR_MARGIN_TOP)

#define GUI_TOC_BACK_BUTTON_OFFSET_X -4

#define GUI_TOC_LIST_WIDTH GUI_SCREEN_WIDTH
#define GUI_TOC_LIST_HEIGHT (GUI_SCREEN_HEIGHT - GUI_STATUS_BAR_HEIGHT - GUI_TOC_BAR_HEIGHT)
#define GUI_TOC_LIST_BUTTON_PAD_TOP 20
#define GUI_TOC_LIST_BUTTON_PAD_BOTTOM 20

void gui_toc_list_create(epub_t *epub);
