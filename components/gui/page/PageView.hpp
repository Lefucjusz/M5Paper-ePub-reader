#pragma once

#include <Epub.hpp>

namespace gui
{
    auto pageViewCreate(const Epub *epub, std::size_t spineEntryIndex) -> void; // TODO error handling
}
