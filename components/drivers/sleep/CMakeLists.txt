idf_component_register(
    SRCS 
        "sleep.c"
    
    INCLUDE_DIRS 
        "."

    PRIV_REQUIRES
        touch_panel
        driver
)
