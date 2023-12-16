#pragma once

#include "lvgl.h"

#define GUI_UNSUPPORTED_POPUP_TITLE LV_SYMBOL_WARNING" Unsupported file"
#define GUI_UNSUPPORTED_POPUP_MAX_MSG_LENGTH 128
#define GUI_UNSUPPORTED_POPUP_BORDER_WIDTH 2
#define GUI_UNSUPPORTED_POPUP_HEIGHT 200
#define GUI_UNSUPPORTED_POPUP_TITLE_HEIGHT 42

void gui_unsupported_popup_create(const char *filename);