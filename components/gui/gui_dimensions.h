#pragma once

#include "eink.h"

/* Total screen size */
#define GUI_DISPLAY_WIDTH EINK_DISPLAY_HEIGHT
#define GUI_DISPLAY_HEIGHT EINK_DISPLAY_WIDTH

/* Vertical margins to compensate for casing covering the screen */
#define GUI_VERTICAL_MARGIN_LEFT 5
#define GUI_VERTICAL_MARGIN_RIGHT 5

/* Horizontal margins to compensate for casing covering the screen */
#define GUI_HORIZONTAL_MARGIN_TOP 10
#define GUI_HORIZONTAL_MARGIN_BOTTOM 10

/* Usable screen size */
#define GUI_SCREEN_WIDTH (GUI_DISPLAY_WIDTH - (GUI_VERTICAL_MARGIN_LEFT + GUI_VERTICAL_MARGIN_RIGHT))
#define GUI_SCREEN_HEIGHT (GUI_DISPLAY_HEIGHT - (GUI_HORIZONTAL_MARGIN_TOP + GUI_HORIZONTAL_MARGIN_BOTTOM))
