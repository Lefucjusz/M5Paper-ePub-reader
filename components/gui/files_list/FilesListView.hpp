#pragma once

#include <filesystem>

namespace gui
{
    auto filesListViewCreate(const std::filesystem::path &path) -> void;
}
