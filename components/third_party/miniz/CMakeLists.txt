idf_component_register(
    SRCS 
        "miniz.c"
        "miniz_zip.c"
        "miniz_tdef.c"
        "miniz_tinfl.c"

    INCLUDE_DIRS 
        "."
)

# Suppress type limits warning
target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-type-limits)
