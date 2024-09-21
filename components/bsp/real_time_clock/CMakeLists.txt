idf_component_register(
    SRCS 
        "real_time_clock.c"
                    
    INCLUDE_DIRS 
        "."
                    
    PRIV_REQUIRES 
        i2c 
        utils
)
                    