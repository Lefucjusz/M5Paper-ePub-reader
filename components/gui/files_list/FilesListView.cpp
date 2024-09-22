#include "FilesListView.hpp"
#include "style/Style.hpp"
#include "DirectoryIterator.hpp"
#include "PopupUnsupported.hpp"
#include "TocListView.hpp"
#include "Fonts.h"
#include <lvgl.h>
#include <esp_log.h>
#include <vector>

#define TAG __FILENAME__

namespace gui
{
    namespace
    {
        lv_obj_t *filesList;

        std::filesystem::path rootPath;
        std::filesystem::path currentPath;
        std::vector<std::filesystem::path> currentEntries;

        auto reloadList() -> void;

        auto isCurrentPathRoot() -> bool
        {
            return currentPath == rootPath;
        }

        auto isSupportedFile(const std::filesystem::path &path) -> bool
        {
            return path.extension() == ".epub";
        }

        auto upClickCallback(lv_event_t *event) -> void
        {
            if (!isCurrentPathRoot()) {
                currentPath = currentPath.parent_path();
                reloadList();
            }
        }

        auto directoryClickCallback(lv_event_t *event) -> void
        {
            const auto entryIndex = reinterpret_cast<std::size_t>(lv_event_get_user_data(event));
            currentPath /= currentEntries[entryIndex];

            reloadList();
        }

        auto supportedFileClickCallback(lv_event_t *event) -> void
        {
            const auto entryIndex = reinterpret_cast<std::size_t>(lv_event_get_user_data(event));
            tocListViewCreate(currentPath / currentEntries[entryIndex]);
        }

        auto unsupportedFileClickCallback(lv_event_t *event) -> void
        {
            const auto entryIndex = reinterpret_cast<std::size_t>(lv_event_get_user_data(event));
            const auto &filename = currentEntries[entryIndex];
            createPopupUnsupported(filename);
        }

        auto reloadList() -> void
        {
            /* Delete current list contents */
            lv_obj_clean(filesList);

            /* Create directory up button */
            if (!isCurrentPathRoot()) {
                auto up_button = lv_list_add_btn(filesList, LV_SYMBOL_DIRECTORY, "..");
                lv_obj_set_style_text_font(up_button, &gui_montserrat_medium_36, LV_PART_MAIN);
                lv_obj_set_style_pad_top(up_button, style::button::padTop, LV_PART_MAIN);
                lv_obj_set_style_pad_bottom(up_button, style::button::padBottom, LV_PART_MAIN);
                lv_obj_add_event_cb(up_button, upClickCallback, LV_EVENT_CLICKED, nullptr);
            }

            /* Create new entries */
            std::size_t entryIndex = 0;
            currentEntries.clear();
            for (const auto &entry : fs::DirectoryIterator(currentPath)) {
                currentEntries.emplace_back(entry.path().filename());

                lv_obj_t *entryButton;
                if (entry.is_directory()) {
                    entryButton = lv_list_add_btn(filesList, LV_SYMBOL_DIRECTORY, entry.path().filename().c_str());
                    lv_obj_add_event_cb(entryButton, directoryClickCallback, LV_EVENT_CLICKED, reinterpret_cast<void *>(entryIndex)); // This is very ugly hack...
                }
                else if (isSupportedFile(entry.path())) {
                    entryButton = lv_list_add_btn(filesList, GUI_SYMBOL_BOOK, currentEntries.back().c_str());
                    lv_obj_add_event_cb(entryButton, supportedFileClickCallback, LV_EVENT_CLICKED, reinterpret_cast<void *>(entryIndex));
                }
                else {
                    entryButton = lv_list_add_btn(filesList, LV_SYMBOL_FILE, currentEntries.back().c_str());
                    lv_obj_add_event_cb(entryButton, unsupportedFileClickCallback, LV_EVENT_CLICKED, reinterpret_cast<void *>(entryIndex));
                }

                lv_obj_set_style_text_font(entryButton, &gui_montserrat_medium_36, LV_PART_MAIN);
                lv_obj_set_style_pad_top(entryButton, style::button::padTop, LV_PART_MAIN);
                lv_obj_set_style_pad_bottom(entryButton, style::button::padBottom, LV_PART_MAIN);
                auto entryButtonLabel = lv_obj_get_child(entryButton, lv_obj_get_child_cnt(entryButton) - 1); // Label is created as a last child
                lv_label_set_long_mode(entryButtonLabel, LV_LABEL_LONG_WRAP); // Disable scrolling, just wrap the text

                entryIndex++;
            }
        }
    }

    auto filesListViewCreate(const std::filesystem::path &path) -> void
    {
        rootPath = path;
        currentPath = rootPath;

        filesList = lv_list_create(lv_scr_act());
        lv_obj_set_size(filesList, style::width, style::height);
        lv_obj_align(filesList, LV_ALIGN_TOP_MID, 0, style::offsetY);
        lv_obj_clear_flag(filesList, LV_OBJ_FLAG_SCROLL_ELASTIC);

        reloadList();
    }
}
