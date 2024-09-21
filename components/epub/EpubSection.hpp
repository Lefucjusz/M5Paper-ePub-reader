#pragma once

#include "TextBlock.hpp"
#include <pugixml/pugixml.hpp>
#include <string>
#include <vector>
#include <array>

using TextBlocks = std::vector<TextBlock>;

class EpubSectionWalker : public pugi::xml_tree_walker
{
    public:
        auto for_each(pugi::xml_node &node) -> bool override final;
        auto on_leave(pugi::xml_node &node) -> bool override final;

        [[nodiscard]] auto getTextBlocks() const -> const TextBlocks &;

    private:
        static constexpr std::array<std::string, 6> hNodes = {"h1", "h2", "h3", "h4", "h5", "h6"};
        static constexpr std::array<std::string, 2> blockNodes = {"p", "div"};

        TextBlocks blocks;
        std::string currentBlockText;
        std::vector<Font> fontStack;

        [[nodiscard]] auto isHeading(const pugi::xml_node &node) const -> bool;
        [[nodiscard]] auto isBlock(const pugi::xml_node &node) const -> bool;
};

class EpubSection
{
    public:
        EpubSection(const std::string &rawContent = {});
        ~EpubSection() = default;

        [[nodiscard]] auto getRaw() const -> const std::string &;
        [[nodiscard]] auto getBlocks() const -> const TextBlocks &;

    private:
        std::string rawContent;
        EpubSectionWalker walker;
};
