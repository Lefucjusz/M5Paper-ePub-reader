idf_component_register(SRCS "touch_panel.c"
                    INCLUDE_DIRS "."
                    REQUIRES i2c
                    PRIV_REQUIRES utils)
