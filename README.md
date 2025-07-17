
# Projet ESP-IDF : Carnet de Santé pour Reptiles

## Setup Post-Script :
1. cd reptile_health_book
2. source $IDF_PATH/export.sh  # Activez ESP-IDF env (export.ps1 sur Windows)
3. idf.py add-dependency "lvgl/lvgl^9.0.0"  # Ajoutez LVGL (créé components/lvgl)
4. Copy lv_conf.h from project to components/lvgl/lv_conf.h  # Custom config (if provided in project)
5. git config --global --add safe.directory %cd%  # Fix Git ownership if error
6. idf.py set-target esp32s3  # Set target (génère sdkconfig)
7. idf.py menuconfig  # Configurez (PSRAM, Wi-Fi, lwip, etc.)
8. idf.py build flash monitor  # Build/Flash/Test

## Notes :
- Adapté Waveshare ESP32-S3 3.5" Type B (QSPI AXS15231B).
- Pour Banana Pi BPI-F3 : Portez via Zephyr (remap GPIO RISC-V, PCIe for extensions).
- Ajoutez capteurs (DHT) via GPIO pour monitoring herpétique (temp/humidité terrarium).
- Si Git error, run git config as above or in admin.
