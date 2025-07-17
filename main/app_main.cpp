
// app_main.cpp - Point d'entrée principal du projet Carnet de Santé pour Reptiles
// Raisonnement : Init hardware/software en séquence (NVS > Drivers > Net > UI), tasks RTOS pour temps réel.
// Contraintes : Haute priorité UI (latence <10ms), watchdog pour résilience (e.g., QSPI fail en humidité).
// Optimisations : DMA QSPI, PSRAM alloc, low-power (e.g., light_sleep post-init).
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "freertos/FreeRTOS.h"       // RTOS pour multitasking (UI séparée)
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"               // ESP-IDF core
#include "esp_wifi.h"                 // Wi-Fi stack
#include "esp_event.h"                // Event loop
#include "esp_log.h"                  // Logging pour profiling/débug (post-mortem via core dump)
#include "nvs_flash.h"                // Stockage sécurisé (chiffrement clés Wi-Fi)
#include "lwip/err.h"                 // LWIP errors
#include "lwip/sys.h"
#include "driver/gpio.h"               // GPIO pour extensions (e.g., capteurs reptile)
#include "lvgl.h"                      // LVGL pour UI réactive
#include "LovyanGFX.hpp"              // Driver QSPI optimisé
#include "sdmmc_cmd.h"                 // SD pour logs JSON santé
#include "esp_sntp.h"                  // NTP pour date/heure précise (logs traçables)

// Tags pour logs (profiling perf.)
static const char *TAG = "ReptileApp";

// Constantes projet (adapté Waveshare Type B)
#define WIFI_SSID "VotreSSID"          // Input via clavier UI
#define WIFI_PASS "VotrePass"         // Sécurisé NVS
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 480
#define LVGL_TICK_MS 16                // ~60 FPS pour QSPI fluidité (optimise latence)

// UI éléments (globaux pour accessibilité, mutex si multi-thread)
static lv_obj_t *wifi_icon, *bt_icon, *time_label;

// Globals pour état (protégés par events/mutex en RTOS)
static EventGroupHandle_t wifi_event_group;
const int WIFI_CONNECTED_BIT = BIT0;
static bool wifi_connected = false, bt_enabled = false;

// Instance LovyanGFX pour QSPI AXS15231B
LGFX lcd;

// Init driver LovyanGFX (QSPI Type B)
static void lvgl_driver_init() {
    ESP_LOGI(TAG, "Init LovyanGFX QSPI...");  // Profiling start
    LGFX_Config cfg;
    cfg.panel_width = SCREEN_WIDTH;
    cfg.panel_height = SCREEN_HEIGHT;
    cfg.bus_qspi = true;                   // QSPI pour rafraîchissement rapide
    // Pins spécifiques Type B (wiki Waveshare ; shielding pour EMI humidité)
    cfg.pin_qspi_cs   = GPIO_NUM_10;
    cfg.pin_qspi_sclk = GPIO_NUM_9;
    cfg.pin_qspi_d0   = GPIO_NUM_11;
    cfg.pin_qspi_d1 = GPIO_NUM_12;
    cfg.pin_qspi_d2 = GPIO_NUM_13;
    cfg. pin_qspi_d3 = GPIO_NUM_14;
    cfg.qspi_frequency = 80000000;         // 80MHz max perf.
    cfg.use_dma = true;                    // DMA pour low CPU (~30% savings)
    lcd.setConfig(cfg);
    if (!lcd.init()) {                     // Gestion erreur : Test init proactive
        ESP_LOGE(TAG, "QSPI Init Fail - Pins/EMI? Retry shield.");  // Piège : noise en terrarium
        esp_restart();                    // Reset soft ; watchdog additif
    }
    lcd.setBrightness(128);                // Luminosité moyenne (PWM pour energy optm)
    ESP_LOGI(TAG, "QSPI OK - Freq %d MHz", cfg.qspi_frequency / 1000000);  // Profiling perf.
}

// Tick handler pour LVGL (RTOS timer)
static void lv_tick_handler(void *arg) {
    lv_tick_inc(LVGL_TICK_MS);             // Incrément tick pour UI events
}

// Task RTOS pour LVGL (haute prio pour temps réel)
static void lvgl_task(void *arg) {
    ESP_LOGI(TAG, "LVGL Task Start");
    while (1) {
        lv_task_handler();                 // Process UI (QSPI: low latence)
        vTaskDelay(pdMS_TO_TICKS(LVGL_TICK_MS));  // Delay optimisé FreeRTOS
    }
}

// Update status bar (NTP date + icônes ; appelé par LV timer)
static void update_status_bar() {
    time_t now = time(&now);
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);
    char buf[32];
    strftime(buf, sizeof(buf), "%d/%m/%Y %H:%M", &timeinfo);  // Format FR
    lv_label_set_text(time_label, buf);    // Gauche : Date/heure

    lv_label_set_text(wifi_icon, wifi_connected ? "WiFi ON" : "WiFi OFF");  // Droite : Icônes (texte simple ; expand to img)
    lv_label_set_text(bt_icon, bt_enabled ? "BT ON" : "BT OFF");
}

// Création UI (barre, clavier AZERTY, exemples)
static void create_ui() {
    ESP_LOGI(TAG, "Create UI");
    // Driver LVGL pour QSPI flush
    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.flush_cb = [](lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map) {
        lcd.pushColors((uint32_t*)color_map, (area->x2 - area->x1 + 1) * (area->y2 - area->y1 + 1));  // QSPI push rapide
        lv_disp_flush_ready(drv);
    };
    lv_disp_drv_register(&disp_drv);

    // Barre statut haut
    lv_obj_t *status_bar = lv_obj_create(lv_scr_act());
    lv_obj_set_size(status_bar, SCREEN_WIDTH, 30);
    lv_obj_set_pos(status_bar, 0, 0);
    lv_obj_set_style_bg_color(status_bar, lv_color_hex(0x000000), 0);

    // Date gauche
    time_label = lv_label_create(status_bar);
    lv_label_set_text(time_label, "Chargement...");
    lv_obj_align(time_label, LV_ALIGN_LEFT_MID, 5, 0);

    // Icônes droite
    wifi_icon = lv_label_create(status_bar);
    lv_obj_align(wifi_icon, LV_ALIGN_RIGHT_MID, -50, 0);
    bt_icon = lv_label_create(status_bar);
    lv_obj_align(bt_icon, LV_ALIGN_RIGHT_MID, -5, 0);

    // Bouton exemple (ajout log reptile)
    lv_obj_t *btn = lv_btn_create(lv_scr_act());
    lv_obj_set_pos(btn, 50, 50);
    lv_obj_t *label = lv_label_create(btn);
    lv_label_set_text(label, "Ajouter Log Santé");

    // Clavier AZERTY (custom map pour FR ; auto sur focus)
    lv_obj_t *kb = lv_keyboard_create(lv_scr_act());
    lv_keyboard_set_mode(kb, LV_KEYBOARD_MODE_TEXT_LOWER);
    lv_obj_set_size(kb, SCREEN_WIDTH, SCREEN_HEIGHT / 2);
    lv_obj_align(kb, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);

    // Textarea exemple (clé Wi-Fi ; show kb)
    lv_obj_t *ta = lv_textarea_create(lv_scr_act());
    lv_obj_set_pos(ta, 50, 100);
    lv_textarea_set_placeholder_text(ta, "Clé Wi-Fi");
    lv_obj_add_event_cb(ta, [](lv_event_t *e) {
        if (lv_event_get_code(e) == LV_EVENT_FOCUSED) {
            lv_obj_clear_flag(kb, LV_OBJ_FLAG_HIDDEN);  // Affiche auto
            lv_keyboard_set_textarea(kb, ta);  // Lie
        }
    }, LV_EVENT_ALL, NULL);

    // Timer update (1s, RTOS safe)
    lv_timer_create([](lv_timer_t *timer) { update_status_bar(); }, 1000, NULL);
}

// Wi-Fi event handler (connexion, retry backoff)
static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    if (event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();  // Init connexion
    } else if (event_id == WIFI_EVENT_STA_DISCONNECTED) {
        wifi_connected = false;
        esp_wifi_connect();  // Retry (ajoutez backoff pour éviter flood)
        ESP_LOGW(TAG, "WiFi Disconnect - Retry");
    } else if (event_id == IP_EVENT_STA_GOT_IP) {
        wifi_connected = true;
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
        ESP_LOGI(TAG, "WiFi IP OK");
    }
}

// Init Wi-Fi (STA mode, WPA3 secure)
static void wifi_init() {
    ESP_LOGI(TAG, "Init WiFi");
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL);
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL);
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
        },
    };
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config);
    esp_wifi_start();
    // Attente connexion (timeout 10s ; NTP après)
    if (xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT, pdFALSE, pdTRUE, pdMS_TO_TICKS(10000)) == 0) {
        ESP_LOGE(TAG, "WiFi Timeout - Check clé/UI input");
    }
}

// NTP init (pour date/heure post-WiFi)
static void ntp_init() {
    ESP_LOGI(TAG, "Init NTP");
    esp_sntp_setoperatingmode(ESP_SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    esp_sntp_init();
}

// BT init (BLE pour sync app herpétique)
static void bt_init() {
    ESP_LOGI(TAG, "Init BT");
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    esp_bt_controller_init(&bt_cfg);
    esp_bt_controller_enable(ESP_BT_MODE_BLE);
    esp_bluedroid_init();
    esp_bluedroid_enable();
    bt_enabled = true;
}

// SD init (logs JSON ; fallback if fail)
static void sdcard_init() {
    ESP_LOGI(TAG, "Init SD");
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = true,  // Auto-format
        .max_files = 5,
    };
    sdmmc_card_t *card;
    sdmmc_host_t host = SDMMC_HOST_DEFAULT();
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
    esp_err_t ret = esp_vfs_fat_sdmmc_mount("/sdcard", &host, &slot_config, &mount_config, &card);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "SD OK - Ready for JSON logs");
    } else {
        ESP_LOGE(TAG, "SD Fail %d - Fallback to Flash", ret);  // Fallback : Utilisez esp_partition_write for internal storage
    }
}

// app_main - Entrée FreeRTOS (init séquentiel)
extern "C" void app_main(void) {
    ESP_LOGI(TAG, "App Main");
    // Init NVS (secure erase if corrupted)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }

    // Events group for sync (WiFi/NTP)
    wifi_event_group = xEventGroupCreate();

    // Init hardware/logic
    lvgl_driver_init();  // QSPI first (base UI)
    sdcard_init();  // Stockage
    wifi_init();  // Net (bloq pour conn)
    bt_init();
    ntp_init();

    // LVGL init (global)
    lv_init();
    // Timer LVGL tick (RTOS esp_timer)
    esp_timer_create_args_t lv_tick_args = {
        .callback = &lv_tick_handler,
        .name = "lv_tick"
    };
    esp_timer_handle_t lv_timer;
    esp_timer_create(&lv_tick_args, &lv_timer);
    esp_timer_start_periodic(lv_timer, LVGL_TICK_MS * 1000);  // Micros

    // Créer UI (post-init)
    create_ui();

    // Task UI (RTOS, prio haute)
    xTaskCreate(lvgl_task, "lvgl_task", 8192, NULL, 4, NULL);  // Stack 8KB for buffers

    // Watchdog for résilience (5s, panic on stall)
    esp_task_wdt_init(5, true);
    esp_task_wdt_add(NULL);

    // Main loop (monitoring reptile ; low power)
    while (1) {
        // Ex: Lire capteur GPIO (DHT temp), log JSON if anomalie, UI alerte (prédictive)
        vTaskDelay(pdMS_TO_TICKS(1000));  // 1s poll, light_sleep possible
        esp_task_wdt_reset();  // Reset WDT
    }
}
