#pragma once

#include "eink.h"

// TODO constexprs

/* Total screen size */
#define GUI_DISPLAY_WIDTH EINK_DISPLAY_HEIGHT
#define GUI_DISPLAY_HEIGHT EINK_DISPLAY_WIDTH

/* Vertical margins to compensate for casing covering the screen */
#define GUI_VERTICAL_MARGIN_LEFT 5
#define GUI_VERTICAL_MARGIN_RIGHT 5
#define GUI_VERTICAL_MARGIN_TOTAL (GUI_VERTICAL_MARGIN_LEFT + GUI_VERTICAL_MARGIN_RIGHT)

/* Horizontal margins to compensate for casing covering the screen */
#define GUI_HORIZONTAL_MARGIN_TOP 10
#define GUI_HORIZONTAL_MARGIN_BOTTOM 10
#define GUI_HORIZONTAL_MARGIN_TOTAL (GUI_HORIZONTAL_MARGIN_TOP + GUI_HORIZONTAL_MARGIN_BOTTOM)

/* Status bar dimensions */
#define GUI_STATUS_BAR_WIDTH GUI_DISPLAY_WIDTH
#define GUI_STATUS_BAR_HEIGHT 36

/* Main area dimensions */
#define GUI_MAIN_AREA_WIDTH (GUI_DISPLAY_WIDTH - GUI_VERTICAL_MARGIN_TOTAL)
#define GUI_MAIN_AREA_HEIGHT (GUI_DISPLAY_HEIGHT - (GUI_STATUS_BAR_HEIGHT + GUI_HORIZONTAL_MARGIN_TOTAL))
#define GUI_MAIN_AREA_MIN_X GUI_VERTICAL_MARGIN_LEFT
#define GUI_MAIN_AREA_MAX_X (GUI_MAIN_AREA_MIN_X + GUI_MAIN_AREA_WIDTH)
#define GUI_MAIN_AREA_MIN_Y (GUI_STATUS_BAR_HEIGHT + GUI_HORIZONTAL_MARGIN_TOP)
#define GUI_MAIN_AREA_MAX_Y (GUI_MAIN_AREA_MIN_Y + GUI_MAIN_AREA_HEIGHT)