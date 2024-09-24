# For more information about build system see
# https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html
# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

set(EXTRA_COMPONENT_DIRS 
    components/bsp
    components/drivers
    components/eink_worker
    components/epub
    components/gui
    components/lvgl_task
    components/third_party
    components/utils
)

# Suppress enum bit operation warning from lv_obj_style.h
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-enum-enum-conversion")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(M5Paper-LVGL)
