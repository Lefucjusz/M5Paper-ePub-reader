idf_component_register(SRCS "lvgl_task.c"
                    INCLUDE_DIRS "."
                    REQUIRES eink
                    PRIV_REQUIRES lvgl utils touch_panel)
