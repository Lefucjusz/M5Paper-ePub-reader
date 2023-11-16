idf_component_register(SRCS "main_window.c"
                    INCLUDE_DIRS "."
                    PRIV_REQUIRES esp_wifi lvgl lvgl_task real_time_clock battery)