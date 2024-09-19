idf_component_register(
    SRCS
    Epub.cpp
    EpubSection.cpp

    INCLUDE_DIRS 
    "."
    "${CMAKE_SOURCE_DIR}/components/third_party" # Hack to prevent including wrong miniz header
    
    PRIV_REQUIRES 
    pugixml
    miniz
)
    
target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-format-overflow)
