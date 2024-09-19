/*
 * Custom directory iterator implementation, as in ESP-IDF
 * std::filesystem::directory_iterator doesn't work...
 */

#pragma once

#include <filesystem>
#include <dirent.h>

namespace fs
{
    class DirectoryEntry
    {
      public:
        ~DirectoryEntry() noexcept;

        auto is_file() const noexcept -> bool;
        auto is_directory() const noexcept -> bool;
        auto path() const -> std::filesystem::path;

      private:
        explicit DirectoryEntry(const std::filesystem::path &path);

        friend class DirectoryIterator;

        std::filesystem::path basePath;
        DIR *dirstream;
        struct dirent *entry;
    };

    class DirectoryIterator
    {
      public:
        using iterator_category = std::input_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = DirectoryEntry;
        using pointer           = DirectoryEntry *;
        using reference         = DirectoryEntry &;

        DirectoryIterator() = default;
        explicit DirectoryIterator(const std::filesystem::path &path);

        reference operator*() const noexcept;
        pointer operator->() const noexcept;

        DirectoryIterator &operator++() noexcept;
        DirectoryIterator operator++(int) noexcept;

        friend bool operator==(const DirectoryIterator &a, const DirectoryIterator &b);
        friend bool operator!=(const DirectoryIterator &a, const DirectoryIterator &b);

      private:
        std::shared_ptr<DirectoryEntry> directoryEntry;
    };

    inline DirectoryIterator begin(const DirectoryIterator &it) noexcept
    {
        return it;
    }

    inline DirectoryIterator end(DirectoryIterator) noexcept
    {
        return {};
    }
} // namespace fs
