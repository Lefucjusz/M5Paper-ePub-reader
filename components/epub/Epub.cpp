#include "Epub.hpp"
#include "EpubConstants.hpp"
#include "UniqueMptr.hpp"
#include "pugixml.hpp"
#include <esp_log.h>
#include <map>
#include <algorithm>

#define TAG "Epub"

using namespace constants::epub;

Epub::Epub(const std::filesystem::path &path)
{
    /* Open file */
    mz_zip_zero_struct(&zip);
    if (!mz_zip_reader_init_file(&zip, path.c_str(), 0)) {
        throw std::runtime_error{std::string{"failed to open file "} + path.c_str()};
    }

    /* Get OPF file path */
    const auto &contentOpfPath = getContentOpfPath();
    if (contentOpfPath.empty()) {
        throw std::runtime_error{"failed to get OPF file path"};
    }

    /* Parse OPF file */
    const auto &rootPath = getRootDirectoryPath(contentOpfPath);
    const auto &ncxPath = parseContentOpf(contentOpfPath, rootPath);
    if (ncxPath.empty()) {
        throw std::runtime_error{"failed to get NCX file path"};
    }

    /* Parse NCX file */
    const auto ncxStatus = parseTocNcx(ncxPath, rootPath);
    if (!ncxStatus) {
        throw std::runtime_error{std::string{"failed to parse NCX file "} + ncxPath.c_str()};
    }
}

Epub::~Epub() noexcept
{
    mz_zip_reader_end(&zip);
}

auto Epub::getTableOfContent() const -> const std::vector<TocEntry> &
{
    return toc;
}

// auto Epub::getSpineEntryIndex(const std::filesystem::path &spineHref) const -> std::size_t
// {
//     const auto it = std::find_if(spine.begin(), spine.end(), [&](const auto &val) {
//         ESP_LOGW("", "%s", val.c_str());
//         return val == spineHref;
//     });

//     return (it != spine.end()) ? std::distance(spine.begin(), it) : invalidSpineEntryIndex;
// }

// auto Epub::getSection(std::size_t spineEntryIndex) const -> EpubSection
// {
//     return {};
// }

auto Epub::getSection(const std::filesystem::path &spineHref) const -> EpubSection
{
    auto sectionRaw = mz_zip_reader_extract_file_to_heap(&zip, spineHref.c_str(), nullptr, 0);
    auto sectionContents = unique_mptr<char[]>(static_cast<char *>(sectionRaw));
    if (sectionContents == nullptr) {
        ESP_LOGE(TAG, "Failed to extract '%s' from archive", spineHref.c_str());
        return {};
    }
    return EpubSection{sectionContents.get()};
}

auto Epub::getContentOpfPath() const -> std::filesystem::path
{
    /* Read container file from the archive and parse it */
    auto containerRaw = mz_zip_reader_extract_file_to_heap(&zip, container::xmlPath, nullptr, 0);
    auto containerContents = unique_mptr<char[]>(static_cast<char *>(containerRaw));
    if (containerContents == nullptr) {
        ESP_LOGE(TAG, "Failed to extract '%s' from archive", container::xmlPath);
        return {};
    }

    pugi::xml_document doc;
    const auto &result = doc.load_string(containerContents.get());
    if (!result) {
        ESP_LOGE(TAG, "Failed to parse '%s', error: %s", container::xmlPath, result.description());
        return {};
    }

    const auto &contentOpfNode = doc.find_node([](const pugi::xml_node &node) {
        const auto &attribute = node.attribute(container::mediaTypeAttr);
        const auto nameMatch = (strcmp(node.name(), container::rootfileNode) == 0);
        const auto attributeMatch = !attribute.empty() && (strcmp(attribute.as_string(), container::mediaTypeAttrValue) == 0);

        return nameMatch && attributeMatch;
    });
    if (contentOpfNode.empty()) {
        ESP_LOGE(TAG, "Failed to find valid rootfile tag in '%s'", container::xmlPath);
        return {};
    }

    return contentOpfNode.attribute(container::fullPathAttr).as_string();
}

auto Epub::getRootDirectoryPath(const std::filesystem::path &contentOpfPath) const -> std::filesystem::path
{
    return contentOpfPath.parent_path();
}

auto Epub::parseContentOpf(const std::filesystem::path &contentOpfPath, const std::filesystem::path &rootPath) -> std::filesystem::path
{
    /* Read OPF file and parse it */
    auto opfRaw = mz_zip_reader_extract_file_to_heap(&zip, contentOpfPath.c_str(), nullptr, 0);
    auto opfContents = unique_mptr<char[]>(static_cast<char *>(opfRaw));
    if (opfContents == nullptr) {
        ESP_LOGE(TAG, "Failed to extract '%s' from archive", contentOpfPath.c_str());
        return {};
    }

    pugi::xml_document doc;
    const auto &result = doc.load_string(opfContents.get());
    if (!result) {
        ESP_LOGE(TAG, "Failed to parse '%s', error: %s", contentOpfPath.c_str(), result.description());
        return {};
    }

    /* Find manifest node */
    const auto &manifestNode = doc.find_node([](const pugi::xml_node &node) {
        return (strcmp(node.name(), opf::manifestNode) == 0);
    });
    if (manifestNode.empty()) {
        ESP_LOGE(TAG, "Failed to find manifest node in '%s'", contentOpfPath.c_str());
        return {};
    }
    
    /* Create manifest map */
    std::map<std::string, std::string> manifestMap;
    std::filesystem::path ncxPath;
    for (const auto &itemNode : manifestNode) {
        const auto id = itemNode.attribute(opf::itemId).as_string();
        const auto href = itemNode.attribute(opf::itemHref).as_string();
        manifestMap.insert({id, href});

        if (strcmp(id, opf::ncxAttrValue) == 0) {
            ncxPath = rootPath / href;
        }
    }
    if (ncxPath.empty()) {
        ESP_LOGE(TAG, "Failed to find NCX path in '%s'", contentOpfPath.c_str());
        return {};
    }

    /* Parse spine */
    const auto &spineNode = doc.find_node([](const pugi::xml_node &node) {
        return (strcmp(node.name(), opf::spineNode) == 0);
    });
    if (spineNode.empty()) {
        ESP_LOGE(TAG, "Failed to find spine node in '%s'", contentOpfPath.c_str());
        return {};
    }

    /* Iterate through every item in spine and add to spine vector */
    for (const auto &itemrefNode : spineNode) {
        const auto idref = itemrefNode.attribute(opf::idrefAttr).as_string();
        const auto &manifestPath = manifestMap[idref];

        spine.emplace_back(rootPath / manifestPath);
    }

    return ncxPath;
}

auto Epub::parseTocNcx(const std::filesystem::path &ncxPath, const std::filesystem::path &rootPath) -> bool
{
    /* Read NCX file and parse it */
    auto ncxRaw = mz_zip_reader_extract_file_to_heap(&zip, ncxPath.c_str(), nullptr, 0);
    auto ncxContents = unique_mptr<char[]>(static_cast<char *>(ncxRaw));
    if (ncxContents == nullptr) {
        ESP_LOGE(TAG, "Failed to extract '%s' from archive", ncxPath.c_str());
        return false;
    }

    pugi::xml_document doc;
    const auto &result = doc.load_string(ncxContents.get());
    if (!result) {
        ESP_LOGE(TAG, "Failed to parse '%s', error: %s", ncxPath.c_str(), result.description());
        return false;
    }

    /* Find navMap node */
    const auto &navMapNode = doc.find_node([](const pugi::xml_node &node) {
        return (strcmp(node.name(), ncx::navMapNode) == 0);
    });
    if (navMapNode.empty()) {
        ESP_LOGE(TAG, "Failed to find manifest node in '%s'", ncxPath.c_str());
        return false;
    }

    /* Iterate through every item in navMap and add to TOC vector */
    for (const auto &navPointNode : navMapNode) {
        const auto &contentPath = std::string{navPointNode.child(ncx::contentNode).attribute(ncx::srcAttr).as_string()};
        auto contentPathNoAnchor = contentPath.substr(0, contentPath.find('#')); // TODO anchor-based navigation is not supported yet
        auto title = std::string{navPointNode.child(ncx::navLabelNode).child(ncx::textNode).child_value()};

        toc.emplace_back(title, rootPath / contentPathNoAnchor);
    }

    return true;
}
