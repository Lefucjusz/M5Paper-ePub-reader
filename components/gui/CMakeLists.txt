idf_component_register(
    SRCS 
    "./gui.c"
    "./files_list/gui_files_list.c"
    "./toc_list/gui_toc_list.c"
    "./status_bar/gui_status_bar.c"
    "./popups/gui_set_time_popup.c"
    "./popups/gui_unsupported_popup.c"
    "./page/gui_page.c"
    "./fonts/gui_montserrat_medium_20.c"
    "./fonts/gui_montserrat_medium_24.c"
    "./fonts/gui_montserrat_medium_28.c"
    "./fonts/gui_montserrat_medium_30.c"
    "./fonts/gui_montserrat_medium_36.c"
    "./fonts/gui_montserrat_medium_44.c"

    INCLUDE_DIRS 
    "." 
    "./files_list"
    "./toc_list"
    "./status_bar" 
    "./popups"
    "./page"
    "./fonts"

    PRIV_REQUIRES 
    lvgl 
    eink 
    real_time_clock 
    battery
    dir
    utils
    epub
)

target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-format-overflow)
