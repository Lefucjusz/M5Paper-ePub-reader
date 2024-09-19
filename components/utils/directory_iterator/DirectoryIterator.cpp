#include "DirectoryIterator.hpp"

namespace fs
{
    DirectoryEntry::DirectoryEntry(const std::filesystem::path &path) : basePath{path}
    {
        dirstream = opendir(basePath.c_str());
        if (dirstream == nullptr) {
            throw std::filesystem::filesystem_error("opendir failed on " + path.string(), std::error_code{});
        }
        entry = readdir(dirstream);
    }

    DirectoryEntry::~DirectoryEntry() noexcept
    {
        closedir(dirstream);
    }

    auto DirectoryEntry::is_file() const noexcept -> bool
    {
        return entry->d_type == DT_REG;
    }

    auto DirectoryEntry::is_directory() const noexcept -> bool
    {
        return entry->d_type == DT_DIR;
    }

    auto DirectoryEntry::path() const -> std::filesystem::path
    {
        return basePath / entry->d_name;
    }

    DirectoryIterator::DirectoryIterator(const std::filesystem::path &path) : directoryEntry{new DirectoryEntry(path)}
    {
        if (directoryEntry->entry == nullptr) {
            directoryEntry.reset();
        }
    }

    DirectoryIterator::reference DirectoryIterator::operator*() const noexcept
    {
        return *directoryEntry;
    }

    DirectoryIterator::pointer DirectoryIterator::operator->() const noexcept
    {
        return &**this;
    }

    DirectoryIterator &DirectoryIterator::operator++() noexcept
    {
        directoryEntry->entry = readdir(directoryEntry->dirstream);
        if (directoryEntry->entry == nullptr) {
            directoryEntry.reset();
        }
        return *this;
    }

    DirectoryIterator DirectoryIterator::operator++(int) noexcept
    {
        auto prev = *this;
        ++(*this);
        return prev;
    }

    bool operator==(const DirectoryIterator &a, const DirectoryIterator &b)
    {
        return a.directoryEntry == b.directoryEntry;
    }

    bool operator!=(const DirectoryIterator &a, const DirectoryIterator &b)
    {
        return a.directoryEntry != b.directoryEntry;
    }
} // namespace fs
