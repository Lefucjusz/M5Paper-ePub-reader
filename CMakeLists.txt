# For more information about build system see
# https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html
# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

set(EXTRA_COMPONENT_DIRS 
    components/bsp
    components/lvgl_task 
    components/utils 
    components/gui 
    components/third_party) # TODO not very clean, but I have no better idea for now

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(M5Paper-LVGL)

