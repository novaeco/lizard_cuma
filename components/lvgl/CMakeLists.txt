# CMakeLists.txt pour LVGL
set(LVGL_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# Collecte de tous les fichiers sources LVGL
file(GLOB_RECURSE SOURCES 
    ${LVGL_ROOT_DIR}/src/*.c
)

# Exclusion des fichiers de démonstration et tests
list(FILTER SOURCES EXCLUDE REGEX ".*lv_demo.*")
list(FILTER SOURCES EXCLUDE REGEX ".*tests.*")

idf_component_register(
    SRCS ${SOURCES}
    INCLUDE_DIRS 
        ${LVGL_ROOT_DIR}
        ${LVGL_ROOT_DIR}/src
        ../lvgl_config
    REQUIRES 
        driver
        esp_timer
        esp_common
)