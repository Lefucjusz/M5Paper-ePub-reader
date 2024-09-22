#include "PageView.hpp"
#include "style/Style.hpp"
#include "Fonts.h"
#include <lvgl.h>
#include <utils.h>
#include <esp_log.h>

#define TAG __FILENAME__

namespace gui
{
    namespace
    {
        enum class PageDirection
        {
            First,
            Previous,
            Next
        };

        constexpr auto noExcessChar = std::numeric_limits<std::size_t>::max();

        const Epub *currentEpub;
        std::size_t pageIndex;
        std::size_t spineIndex;
        std::vector<lv_obj_t *> pages;

        auto renderNextSection(PageDirection direction) -> void;

        auto isAtFirstPage() -> bool
        {
            return pageIndex == 0;
        }

        auto getLastPageIndex() -> std::size_t
        {
            return pages.size() - 1;
        }

        auto isAtLastPage() -> bool
        {
            return pageIndex >= getLastPageIndex();
        }

        auto isSectionEnd(const EpubSection &section, std::size_t blockIndex) -> bool
        {
            return blockIndex >= section.getBlocks().size();
        }

        auto isBookBeginning() -> bool
        {
            /* TODO this is probably bad idea */
            const auto &firstTocItem = currentEpub->getTableOfContent().at(0);
            return spineIndex == currentEpub->getSpineEntryIndex(firstTocItem.contentPath);
        }

        auto isBookEnd() -> bool
        {
            return spineIndex == (currentEpub->getSpineItemsCount() - 1);
        }

        auto cleanupSection() -> void
        {
            for (auto page : pages) {
                lv_obj_del_async(page);
            }
            pages.clear();
        }

        auto handlePreviousAtFirstSectionPage() -> void
        {
            if (isBookBeginning()) {
                ESP_LOGW(TAG, "Reached beginning of the book!");
            }
            else {
                ESP_LOGI(TAG, "First page - rendering previous section");
                renderNextSection(PageDirection::Previous);
                pageIndex = getLastPageIndex();
                lv_obj_clear_flag(pages[pageIndex], LV_OBJ_FLAG_HIDDEN);
            }
        }

        auto handleNextAtLastSectionPage() -> void
        {
            if (isBookEnd()) {
                ESP_LOGW(TAG, "Reached end of the book!");
            }
            else {
                ESP_LOGI(TAG, "Last page - rendering next section");
                renderNextSection(PageDirection::Next);
                pageIndex = 0;
                lv_obj_clear_flag(pages[pageIndex], LV_OBJ_FLAG_HIDDEN);
            }
        }

        auto turnPage(PageDirection direction) -> void
        {
            switch (direction) {
                case PageDirection::First:
                    lv_obj_add_flag(pages[pageIndex], LV_OBJ_FLAG_HIDDEN);
                    pageIndex = 0;
                    lv_obj_clear_flag(pages[pageIndex], LV_OBJ_FLAG_HIDDEN);
                    break;
                case PageDirection::Previous:
                    if (isAtFirstPage()) {
                        handlePreviousAtFirstSectionPage();
                    }
                    else {
                        lv_obj_add_flag(pages[pageIndex], LV_OBJ_FLAG_HIDDEN);
                        pageIndex--;
                        lv_obj_clear_flag(pages[pageIndex], LV_OBJ_FLAG_HIDDEN);
                    }
                    break;
                case PageDirection::Next:
                    if (isAtLastPage()) {
                        handleNextAtLastSectionPage();
                    }
                    else {
                        lv_obj_add_flag(pages[pageIndex], LV_OBJ_FLAG_HIDDEN);
                        pageIndex++;
                        lv_obj_clear_flag(pages[pageIndex], LV_OBJ_FLAG_HIDDEN);
                    }
                    break;
                default:
                    break;    
            }
        }

        auto onSwipeCallback(lv_event_t *event) -> void
        {
            const auto swipeDirection = lv_indev_get_gesture_dir(lv_indev_get_act());
            switch (swipeDirection) {
                case LV_DIR_LEFT: 
                    turnPage(PageDirection::Next);
                    break;
                case LV_DIR_RIGHT:
                    turnPage(PageDirection::Previous);
                    break;
                case LV_DIR_BOTTOM:
                    cleanupSection();
                    break;
                default:
                    break;
            }
        }

        auto createNewPage() -> lv_obj_t *
        {
            auto page = lv_obj_create(lv_scr_act());

            lv_obj_set_size(page, style::width, style::height);
            lv_obj_align(page, LV_ALIGN_TOP_MID, 0, style::offsetY);
            lv_obj_set_style_pad_all(page, 0, LV_PART_MAIN);
            lv_obj_set_style_border_side(page, LV_BORDER_SIDE_NONE, LV_PART_MAIN);
            lv_obj_add_event_cb(page, onSwipeCallback, LV_EVENT_GESTURE, nullptr);
            lv_obj_clear_flag(page, LV_OBJ_FLAG_SCROLLABLE);
            lv_obj_clear_flag(page, LV_OBJ_FLAG_GESTURE_BUBBLE);
            lv_obj_add_flag(page, LV_OBJ_FLAG_HIDDEN); // By default page should be invisible after creation

            return page;
        }

        auto addBlockToPage(lv_obj_t *page, const TextBlock &textBlock, std::size_t blockOffsetBytes) -> lv_obj_t *
        {
            /* Add new block */
            auto label = lv_label_create(page);
            lv_label_set_text(label, &textBlock.text[blockOffsetBytes]);
            lv_obj_set_width(label, style::width);
            lv_obj_set_style_pad_bottom(label, 0, LV_PART_MAIN); // Needed to maintain line spacing between blocks
            lv_obj_set_style_text_line_space(label, style::lineSpacing, LV_PART_MAIN);
            switch (textBlock.font) {
                case Font::Normal:
                    lv_obj_set_style_text_font(label, &gui_montserrat_medium_28, LV_PART_MAIN);
                    break;

                case Font::Bold:
                    lv_obj_set_style_text_font(label, &gui_montserrat_medium_36, LV_PART_MAIN);
                    break;

                default:
                    break;
            }
            
            /* Align to previous block if exists */
            if (lv_obj_get_child_cnt(page) > 1) {
                lv_obj_set_style_pad_top(label, style::lineSpacing, LV_PART_MAIN); // Maintain line spacing between blocks
                auto previousLabelIndex = lv_obj_get_child_cnt(page) - 2; // TODO magic number?
                auto previousLabel = lv_obj_get_child(page, previousLabelIndex);
                lv_obj_align_to(label, previousLabel, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
            }

            /* Force update coordinates */
            lv_obj_update_layout(page);

            return label;
        }

        /* TODO this algorithm probably won't work for all cases */
        auto getFirstExcessCharIndex(const lv_obj_t *block) -> std::size_t
        {
            lv_area_t area;
            lv_obj_get_coords(block, &area);

            const auto blockStartY = area.y1;
            const auto blockEndY = area.y2;

            /* The text ends before end of main area */
            if (blockEndY <= style::main_area::maxY) {
                return noExcessChar;
            }

            /* The text starts after the end of main area */
            if (blockStartY >= style::main_area::maxY) {
                return 0;
            }

            /* Compute first point not fitting on screen in local block coordinates 
             * Assume the text is always fully aligned to the left -> x coordinate always zero */
            lv_point_t firstExcessPointCoords = {
                .x = 0,
                .y = static_cast<lv_coord_t>(style::main_area::maxY - blockStartY)
            };

            if (!lv_label_is_char_under_pos(block, &firstExcessPointCoords)) {
                return noExcessChar;
            }
            return lv_label_get_letter_on(block, &firstExcessPointCoords);
        }

        auto renderSection(const EpubSection &section) -> void
        {
            std::size_t blockIndex = 0;
            std::size_t blockOffsetBytes = 0;
            std::size_t blockBytesLeft = 0;

            const auto &blocks = section.getBlocks();
            auto page = createNewPage();

            /* Create LVGL objects with rendered text blocks for entire section */
            while (true) {
                const auto &block = blocks[blockIndex];
                auto newGuiBlock = addBlockToPage(page, block, blockOffsetBytes);

                const auto excessCharIndex = getFirstExcessCharIndex(newGuiBlock);
                if (excessCharIndex == noExcessChar) { // Entire block fits on page
                    blockBytesLeft = 0;
                    if (isSectionEnd(section, blockIndex + 1)) { // Handle single page per block case
                        pages.push_back(page);
                    }
                }
                else if (excessCharIndex == 0) { // Not a single char fits
                    /* Delete new block from page */
                    lv_obj_del(newGuiBlock);

                    /* Add current page to pages vector and create new page */
                    pages.push_back(page);
                    page = createNewPage();
                }
                else if (excessCharIndex > 0) { // Block fits partially
                    /* Remove excess part */
                    lv_label_cut_text(newGuiBlock, excessCharIndex, LV_LABEL_POS_LAST);

                    /* Convert char index to byte offset and update sizes */
                    const auto bytesAdded = _lv_txt_encoded_get_byte_id(block.text.c_str(), excessCharIndex);
                    blockOffsetBytes += bytesAdded;
                    blockBytesLeft -= bytesAdded;
                    
                    /* Add current page to pages vector and create new page */
                    pages.push_back(page);
                    page = createNewPage();
                }

                /* Entire block rendered, get next if possible */
                if (blockBytesLeft == 0) {
                    blockIndex++;
                    if (isSectionEnd(section, blockIndex)) {
                        ESP_LOGI(TAG, "Reached end of section");
                        break;
                    }
                    else {
                        blockOffsetBytes = 0;
                        const auto &nextSectionBlock = blocks[blockIndex];
                        blockBytesLeft = nextSectionBlock.text.length();
                    }
                }
            }
        }

        auto renderNextSection(PageDirection direction) -> void
        {
            /* Remove previous section */
            cleanupSection();

            /* Get next section */
            switch (direction) {
                case PageDirection::First:
                    break;
                case PageDirection::Previous:
                    spineIndex--;
                    break;
                case PageDirection::Next:
                    spineIndex++;
                    break;
                default:
                    ESP_LOGE(TAG, "Invalid PageDirection!");
                    break;
            }

            /* Render section */
            EpubSection section;
            try {
                section = currentEpub->getSection(spineIndex);
            } catch (std::exception &e) {
                ESP_LOGE(TAG, "Exception for section@%zu: '%s'", spineIndex, e.what());
                return;
            }
            ESP_LOGI(TAG, "Rendering started...");
            auto start = lv_tick_get();
            renderSection(section);
            auto end = lv_tick_get();
            ESP_LOGW(TAG, "Rendering time %lums", end - start);
        }
    }

    auto pageViewCreate(const Epub *epub, std::size_t spineEntryIndex) -> void
    {
        /* Sanity check */
        if (epub == nullptr) {
            return;
        }

        /* Initialize context */
        currentEpub = epub;
        pageIndex = 0;
        spineIndex = spineEntryIndex;

        /* Render pages for first section */
        renderNextSection(PageDirection::First);
            
        /* Show first page */
        turnPage(PageDirection::First);
    }
}
