# Configuration LVGL pour le projet Reptile Management
idf_component_register(
    SRCS 
        "lv_conf.c"
    INCLUDE_DIRS 
        "."
    REQUIRES 
        driver
        esp_timer
        esp_common
)