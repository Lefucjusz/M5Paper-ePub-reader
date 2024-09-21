#include "TocListView.hpp"
#include "PageView.hpp"
#include <Epub.hpp>
#include <Fonts.h>
#include <lvgl.h>
#include <esp_log.h>

#define TAG __FILENAME__

namespace gui
{
    namespace
    {
        lv_obj_t *topBar;
        lv_obj_t *tocLabel;
        lv_obj_t *backButton;
        lv_obj_t *backButtonIcon;
        lv_obj_t *tocList;

        std::unique_ptr<Epub> currentEpub;

        auto tocItemClickCallback(lv_event_t *event) -> void
        {
            auto entry = static_cast<const Epub::TocEntry *>(lv_event_get_user_data(event));
            ESP_LOGI(TAG, "Entry title: %s", entry->title.c_str());
            ESP_LOGI(TAG, "Entry href: %s", entry->contentPath.c_str());
        
            pageViewCreate(currentEpub.get(), entry->contentPath);
        }

        auto backButtonClickCallback(lv_event_t *event) -> void
        {
            currentEpub.reset();
            lv_obj_del(topBar);
            lv_obj_del(tocList);
        }
    }

    auto tocListViewCreate(const std::filesystem::path &epubPath) -> void // TODO add error handling
    {
        /* Open epub */
        try {
            currentEpub = std::make_unique<Epub>(epubPath);
        }
        catch (const std::runtime_error &e) {
            ESP_LOGE(TAG, "Failed to open epub file '%s', error: %s", epubPath.c_str(), e.what());
            return;
        }

        /* Create top bar */
        topBar = lv_obj_create(lv_scr_act());
        lv_obj_set_size(topBar, GUI_TOC_BAR_WIDTH, GUI_TOC_BAR_HEIGHT);
        lv_obj_align(topBar, LV_ALIGN_TOP_MID, 0, GUI_TOC_BAR_OFFSET_Y);
        lv_obj_set_style_border_side(topBar, LV_BORDER_SIDE_BOTTOM, LV_PART_MAIN);
        lv_obj_clear_flag(topBar, LV_OBJ_FLAG_SCROLLABLE);

        /* Add list label */
        tocLabel = lv_label_create(topBar);
        lv_label_set_text(tocLabel, "Table of contents");
        lv_obj_set_style_text_font(tocLabel, &gui_montserrat_medium_28, LV_PART_MAIN);
        lv_obj_center(tocLabel);

        /* Add back button */
        backButton = lv_btn_create(topBar);
        lv_obj_set_style_border_side(backButton, LV_BORDER_SIDE_NONE, LV_PART_MAIN);
        lv_obj_set_height(backButton, GUI_TOC_BACK_BUTTON_HEIGHT);
        lv_obj_align(backButton, LV_ALIGN_LEFT_MID, GUI_TOC_BACK_BUTTON_OFFSET_X, 0);
        lv_obj_add_event_cb(backButton, backButtonClickCallback, LV_EVENT_CLICKED, nullptr);
        
        /* Add back button icon */
        backButtonIcon = lv_label_create(backButton);
        lv_label_set_text(backButtonIcon, GUI_SYMBOL_ARROW_LEFT);
        lv_obj_set_style_text_font(backButtonIcon, &gui_montserrat_medium_44, LV_PART_MAIN);
        lv_obj_center(backButtonIcon);

        /* Create list */
        tocList = lv_list_create(lv_scr_act());
        lv_obj_set_size(tocList, GUI_TOC_LIST_WIDTH, GUI_TOC_LIST_HEIGHT);
        lv_obj_align_to(tocList, topBar, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
        lv_obj_clear_flag(tocList, LV_OBJ_FLAG_SCROLL_ELASTIC);

        /* Fill the list with TOC items */
        const auto &toc = currentEpub->getTableOfContent();
        for (const auto &entry : toc) {
            auto listButton = lv_list_add_btn(tocList, GUI_SYMBOL_BOOK_OPEN" ", entry.title.c_str());
            auto rawEntryPtr = static_cast<void *>(const_cast<Epub::TocEntry *>(&entry));
            lv_obj_add_event_cb(listButton, tocItemClickCallback, LV_EVENT_CLICKED, rawEntryPtr);
            lv_obj_set_style_text_font(listButton, &gui_montserrat_medium_36, LV_PART_MAIN);
            lv_obj_set_style_pad_top(listButton, GUI_TOC_LIST_BUTTON_PAD_TOP, LV_PART_MAIN);
            lv_obj_set_style_pad_bottom(listButton, GUI_TOC_LIST_BUTTON_PAD_BOTTOM, LV_PART_MAIN);
            auto listButtonLabel = lv_obj_get_child(listButton, lv_obj_get_child_cnt(listButton) - 1); // Label is created as the last child
            lv_label_set_long_mode(listButtonLabel, LV_LABEL_LONG_WRAP); // Disable scrolling, just wrap the text
        }
    }
}
