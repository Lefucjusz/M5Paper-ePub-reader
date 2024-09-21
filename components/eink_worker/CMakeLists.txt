idf_component_register(
    SRCS 
        "eink_worker.c"

    INCLUDE_DIRS 
        "."

    PRIV_REQUIRES
        eink
        
    REQUIRES
        lvgl
)
