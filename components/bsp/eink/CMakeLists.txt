idf_component_register(SRCS "eink.c"
                    INCLUDE_DIRS "."
                    REQUIRES spi
                    PRIV_REQUIRES driver utils)
                    