idf_component_register(
    SRCS 
        "fatfs_sd.c"
                    
    INCLUDE_DIRS 
        "."
                    
    PRIV_REQUIRES 
        spi 
        fatfs
)
