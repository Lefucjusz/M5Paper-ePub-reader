#include "Gui.hpp"
// #include "gui_status_bar.h"
#include "FilesListView.hpp"

namespace gui
{
    auto create(const std::filesystem::path &rootPath) -> void
    {
        // gui_status_bar_create();
        filesListViewCreate(rootPath);
    }
}
