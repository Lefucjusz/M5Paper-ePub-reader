#include "EpubSection.hpp"
#include <esp_log.h>
#include <algorithm>
#include <stdexcept>

#define TAG __FILENAME__

auto EpubSectionWalker::for_each(pugi::xml_node &node) -> bool
{
    switch (node.type()) {
        case pugi::xml_node_type::node_element:
            if (isHeading(node)) {
                fontStack.push_back(Font::Bold);
            }
            else if (isBlock(node)) {
                fontStack.push_back(Font::Normal);
            }
            break;

        case pugi::xml_node_type::node_pcdata:
            if (!fontStack.empty()) {
                currentBlockText += node.value();
            }
            break;

        default:
            break;
    }
    return true;
}

auto EpubSectionWalker::on_leave(pugi::xml_node &node) -> bool
{
    if (node.type() != pugi::xml_node_type::node_element) {
        return true;
    }
    if (currentBlockText.empty()) {
        return true;
    }

    if (isHeading(node) || isBlock(node)) {
        /* Remove newlines */
        std::replace(currentBlockText.begin(), currentBlockText.end(), '\n', ' ');

        /* Substitute HTML entities */
        currentBlockText = htmlEntities.substitute(currentBlockText);

        /* Push new block */
        blocks.emplace_back(currentBlockText, fontStack.back());
        currentBlockText.clear();
        fontStack.pop_back();
    }
    return true;
}

auto EpubSectionWalker::getTextBlocks() const -> const TextBlocks &
{
    return blocks;
}

auto EpubSectionWalker::isHeading(const pugi::xml_node &node) const -> bool 
{
    return std::any_of(hNodes.begin(), hNodes.end(), [&](const auto &hNode) {
        return node.name() == hNode;
    });
}

auto EpubSectionWalker::isBlock(const pugi::xml_node &node) const -> bool 
{
    return std::any_of(blockNodes.begin(), blockNodes.end(), [&](const auto &blockNode) {
        return node.name() == blockNode;
    });
}


EpubSection::EpubSection(const std::string &rawContent) : rawContent{rawContent}
{
    if (rawContent.empty()) {
        return;
    }

    pugi::xml_document doc;
    const auto &result = doc.load_string(this->rawContent.c_str());
    if (!result) {
        throw std::runtime_error{std::string{"failed to parse section: "} + result.description()};
    }

    doc.traverse(walker);
}

auto EpubSection::getRaw() const -> const std::string &
{
    return rawContent;
}

auto EpubSection::getBlocks() const -> const TextBlocks &
{
    return walker.getTextBlocks();
}
