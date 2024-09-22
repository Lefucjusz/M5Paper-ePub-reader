#include "Gui.hpp"
#include "StatusBar.hpp"
#include "FilesListView.hpp"

namespace gui
{
    auto create(const std::filesystem::path &rootPath) -> void
    {
        statusBarCreate();
        filesListViewCreate(rootPath);
    }
}
