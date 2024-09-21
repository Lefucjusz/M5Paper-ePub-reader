idf_component_register(
    SRCS 
        "Gui.cpp"
        "toc_list/TocListView.cpp"
        # "./status_bar/gui_status_bar.c"
        # "./popups/gui_set_time_popup.c"
        "./popups/PopupUnsupported.cpp"
        "page/PageView.cpp"
        "files_list/FilesListView.cpp"
        
        "fonts/gui_montserrat_medium_20.c"
        "fonts/gui_montserrat_medium_24.c"
        "fonts/gui_montserrat_medium_28.c"
        "fonts/gui_montserrat_medium_30.c"
        "fonts/gui_montserrat_medium_36.c"
        "fonts/gui_montserrat_medium_44.c"

    INCLUDE_DIRS 
        "." 
        "files_list"
        "toc_list"
        # "./status_bar" 
        "popups"
        "page"
        "fonts"

    PRIV_REQUIRES 
        lvgl 
        eink
        directory_iterator
        epub
        utils
)
