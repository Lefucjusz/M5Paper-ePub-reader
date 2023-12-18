idf_component_register(
    SRCS 
    "epub.c" 
    "epub_toc_entry.c"
    "epub_text_block.c"
    "epub_section.c"

    INCLUDE_DIRS 
    "."
    
    PRIV_REQUIRES 
    miniz 
    map 
    vec 
    cvec 
    mxml 
    utils
)

target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-format-overflow)
