idf_component_register(SRCS "files_list/gui_files_list.c" "status_bar/gui_status_bar.c"
                    INCLUDE_DIRS "." "./status_bar" "./files_list"
                    PRIV_REQUIRES lvgl eink real_time_clock battery)
