idf_component_register(
    SRCS 
        "lvgl_task.c"
                    
    INCLUDE_DIRS 
        "."

    REQUIRES 
        eink
        
    PRIV_REQUIRES 
        lvgl 
        touch_panel
        sleep
        utils
        eink_worker
        gui
        real_time_clock
)
