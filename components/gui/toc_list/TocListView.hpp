#pragma once

#include <filesystem>

namespace gui
{
    auto tocListViewCreate(const std::filesystem::path &epubPath) -> void;
}
