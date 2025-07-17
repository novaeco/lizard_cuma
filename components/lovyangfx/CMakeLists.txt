# CMakeLists.txt pour LovyanGFX
set(LOVYANGFX_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# Collecte des fichiers sources LovyanGFX
file(GLOB_RECURSE SOURCES 
    ${LOVYANGFX_ROOT_DIR}/src/*.cpp
    ${LOVYANGFX_ROOT_DIR}/src/*.c
)

# Exclusion des exemples et tests
list(FILTER SOURCES EXCLUDE REGEX ".*examples.*")
list(FILTER SOURCES EXCLUDE REGEX ".*test.*")

idf_component_register(
    SRCS ${SOURCES}
    INCLUDE_DIRS 
        ${LOVYANGFX_ROOT_DIR}/src
        ${LOVYANGFX_ROOT_DIR}
    REQUIRES 
        driver
        spi_flash
        esp_lcd
        esp_timer
        nvs_flash
        efuse
)