#pragma once

#include "gui_dimensions.h"
#include "gui_status_bar.h" // TODO this should not be included here...

#define GUI_FILES_LIST_MARGIN_TOP 2
#define GUI_FILES_LIST_WIDTH GUI_SCREEN_WIDTH
#define GUI_FILES_LIST_HEIGHT (GUI_SCREEN_HEIGHT - GUI_STATUS_BAR_HEIGHT - GUI_FILES_LIST_MARGIN_TOP)
#define GUI_FILES_LIST_OFFSET_X (GUI_STATUS_BAR_HEIGHT + GUI_HORIZONTAL_MARGIN_TOP + GUI_FILES_LIST_MARGIN_TOP)
#define GUI_FILES_LIST_BUTTON_PAD_TOP 20
#define GUI_FILES_LIST_BUTTON_PAD_BOTTOM 20

void gui_files_list_create(void);
