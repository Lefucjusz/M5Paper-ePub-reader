#pragma once

#include "EpubSection.hpp"
#include <miniz/miniz.h>
#include <vector>
#include <filesystem>

class Epub
{
    public:
        struct TocEntry
        {
            std::string title;
            std::filesystem::path contentPath;
        };

        static constexpr auto invalidSpineEntryIndex = std::numeric_limits<std::size_t>::max();

        Epub(const std::filesystem::path &path);
        ~Epub() noexcept;

        [[nodiscard]] auto getTableOfContent() const -> const std::vector<TocEntry> &;
        [[nodiscard]] auto getSpineEntryIndex(const std::filesystem::path &spineHref) const -> std::size_t;
        [[nodiscard]] auto getSpineItemsCount() const -> std::size_t;
        [[nodiscard]] auto getSection(std::size_t spineEntryIndex) const -> EpubSection;
        [[nodiscard]] auto getSection(const std::filesystem::path &spineHref) const -> EpubSection;

    private:
        mutable mz_zip_archive zip;
        std::vector<std::filesystem::path> spine;
        std::vector<TocEntry> toc;

        [[nodiscard]] auto getContentOpfPath() const -> std::filesystem::path;
        [[nodiscard]] auto getRootDirectoryPath(const std::filesystem::path &contentOpfPath) const -> std::filesystem::path;
        auto parseContentOpf(const std::filesystem::path &contentOpfPath, const std::filesystem::path &rootPath) -> std::filesystem::path;
        auto parseTocNcx(const std::filesystem::path &ncxPath, const std::filesystem::path &rootPath) -> bool;
};
