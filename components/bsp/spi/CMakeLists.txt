idf_component_register(
    SRCS 
        "spi.c"
                    
    INCLUDE_DIRS 
        "."
    
    REQUIRES
        driver
    
    PRIV_REQUIRES 
        utils
)
