#pragma once

#include <filesystem>

namespace gui
{
    auto create(const std::filesystem::path &rootPath) -> void; // TODO error handling
}
