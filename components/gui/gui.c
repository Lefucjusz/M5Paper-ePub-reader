#include "gui.h"
#include "gui_status_bar.h"
#include "gui_files_list.h"

void gui_create(void)
{
    gui_status_bar_create();
    gui_files_list_create();
}
