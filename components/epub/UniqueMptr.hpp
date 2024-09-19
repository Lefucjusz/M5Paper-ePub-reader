#pragma once

#include <cstdlib>
#include <memory>

struct MallocDeleter
{
    auto operator()(void *p) const -> void {
        free(p);
    }
};

template <typename T>
using unique_mptr = std::unique_ptr<T, MallocDeleter>;
