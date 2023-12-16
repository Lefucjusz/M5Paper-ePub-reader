idf_component_register(
    SRCS 
    "./files_list/gui_files_list.c"
    "./toc_list/gui_toc_list.c"
    "./status_bar/gui_status_bar.c"
    "./popups/gui_set_time_popup.c"
    "./popups/gui_unsupported_popup.c"

    INCLUDE_DIRS 
    "." 
    "./files_list" 
    "./toc_list"
    "./status_bar" 
    "./popups"

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
