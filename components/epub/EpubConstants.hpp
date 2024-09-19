#pragma once

namespace constants::epub
{
    namespace container
    {
        inline constexpr auto xmlPath = "META-INF/container.xml";
        inline constexpr auto rootfileNode = "rootfile";
        inline constexpr auto mediaTypeAttr = "media-type";
        inline constexpr auto mediaTypeAttrValue = "application/oebps-package+xml";
        inline constexpr auto fullPathAttr = "full-path";
    }

    namespace opf
    {
        inline constexpr auto manifestNode = "manifest";
        inline constexpr auto itemId = "id";
        inline constexpr auto itemHref = "href";
        inline constexpr auto ncxAttrValue = "ncx";
        inline constexpr auto spineNode = "spine";
        inline constexpr auto idrefAttr = "idref";
    }

    namespace ncx
    {
        inline constexpr auto navMapNode = "navMap";
        inline constexpr auto contentNode = "content";
        inline constexpr auto srcAttr = "src";
        inline constexpr auto navLabelNode = "navLabel";
        inline constexpr auto textNode = "text";
    }
}
