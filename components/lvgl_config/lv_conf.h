/**
 * @file lv_conf.h
 * Configuration LVGL pour le système de gestion reptiles ESP32-S3
 */

#ifndef LV_CONF_H
#define LV_CONF_H

#include <stdint.h>

/*====================
   COLOR SETTINGS
 *====================*/

/* Color depth: 1 (1 bpp), 8 (RGB332), 16 (RGB565), 32 (ARGB8888) */
#define LV_COLOR_DEPTH 16

/* Swap the 2 bytes of RGB565 color. Useful if the display has an 8-bit interface (e.g. SPI) */
#define LV_COLOR_16_SWAP 1

/* Enable more complex drawing routines to manage screens transparency.
 * Can be used if the UI is above another layer, e.g. an OSD menu or video player.
 * Requires `LV_COLOR_DEPTH = 32` colors and the screen's `bg_opa` should be set to non LV_OPA_COVER value */
#define LV_COLOR_SCREEN_TRANSP 0

/* Images pixels with this color will not be drawn if they are chroma keyed) */
#define LV_COLOR_CHROMA_KEY lv_color_hex(0x00ff00)         /* pure green */

/*=========================
   MEMORY SETTINGS
 *=========================*/

/* 1: use custom malloc/free, 0: use the built-in `lv_mem_alloc()` and `lv_mem_free()` */
#define LV_MEM_CUSTOM 1
#if LV_MEM_CUSTOM == 0
    /* Size of the memory available for `lv_mem_alloc()` in bytes (>= 2kB) */
    #define LV_MEM_SIZE (64U * 1024U)          /* [bytes] */
    /* Set an address for the memory pool instead of allocating it as a normal array. Can be in external SRAM too. */
    #define LV_MEM_ADR 0     /* 0: unused */
    /* Instead of an address give a memory allocator that will be called to get a memory pool for LVGL. E.g. my_malloc */
    #if LV_MEM_ADR == 0
        #undef LV_MEM_POOL_INCLUDE
        #undef LV_MEM_POOL_ALLOC
    #endif
#else       /* LV_MEM_CUSTOM */
    #define LV_MEM_CUSTOM_INCLUDE <stdlib.h>   /* Header for the dynamic memory function */
    #define LV_MEM_CUSTOM_ALLOC   malloc
    #define LV_MEM_CUSTOM_FREE    free
    #define LV_MEM_CUSTOM_REALLOC realloc
#endif     /* LV_MEM_CUSTOM */

/* Number of the intermediate memory buffer used during rendering and other internal processing.
 * You will see an error log message if there wasn't enough buffers. */
#define LV_MEM_BUF_MAX_NUM 16

/* Use the standard `memcpy` and `memset` instead of LVGL's own functions. (Might or might not be faster). */
#define LV_MEMCPY_MEMSET_STD 1

/*====================
   HAL SETTINGS
 *====================*/

/* Default display refresh period. LVG will redraw changed areas with this period time */
#define LV_DISP_DEF_REFR_PERIOD 16      /* [ms] */

/* Input device read period in milliseconds */
#define LV_INDEV_DEF_READ_PERIOD 30     /* [ms] */

/* Use a custom tick source that tells the elapsed time in milliseconds.
 * It removes the need to manually update the tick with `lv_tick_inc()`) */
#define LV_TICK_CUSTOM 1
#if LV_TICK_CUSTOM
    #define LV_TICK_CUSTOM_INCLUDE "esp_timer.h"         /* Header for the system time function */
    #define LV_TICK_CUSTOM_SYS_TIME_EXPR (esp_timer_get_time() / 1000)    /* Expression evaluating to current system time in ms */
#endif   /* LV_TICK_CUSTOM */

/* Default Dot Per Inch. Used to initialize default sizes such as widgets sized, style paddings.
 * (Not so important, you can adjust it to modify default sizes and spaces) */
#define LV_DPI_DEF 130     /* [px/inch] */

/*======================
 * FEATURE CONFIGURATION
 *======================*/

/* Montserrat fonts with various styles and sizes */
#define LV_FONT_MONTSERRAT_8  0
#define LV_FONT_MONTSERRAT_10 0
#define LV_FONT_MONTSERRAT_12 1
#define LV_FONT_MONTSERRAT_14 1
#define LV_FONT_MONTSERRAT_16 1
#define LV_FONT_MONTSERRAT_18 1
#define LV_FONT_MONTSERRAT_20 1
#define LV_FONT_MONTSERRAT_22 1
#define LV_FONT_MONTSERRAT_24 1
#define LV_FONT_MONTSERRAT_26 0
#define LV_FONT_MONTSERRAT_28 0
#define LV_FONT_MONTSERRAT_30 0
#define LV_FONT_MONTSERRAT_32 0
#define LV_FONT_MONTSERRAT_34 0
#define LV_FONT_MONTSERRAT_36 0
#define LV_FONT_MONTSERRAT_38 0
#define LV_FONT_MONTSERRAT_40 0
#define LV_FONT_MONTSERRAT_42 0
#define LV_FONT_MONTSERRAT_44 0
#define LV_FONT_MONTSERRAT_46 0
#define LV_FONT_MONTSERRAT_48 0

/* Demonstrate special features */
#define LV_FONT_MONTSERRAT_12_SUBPX      0
#define LV_FONT_MONTSERRAT_28_COMPRESSED 0  /* bpp = 3 */
#define LV_FONT_DEJAVU_16_PERSIAN_HEBREW 0  /* Hebrew, Arabic, Persian letters and all their forms */
#define LV_FONT_SIMSUN_16_CJK            0  /* 1000 most common CJK radicals */

/* Pixel perfect monospace fonts */
#define LV_FONT_UNSCII_8  0
#define LV_FONT_UNSCII_16 0

/* Optionally declare custom fonts here.
 * You can use these fonts as default font too and they will be available globally.
 * E.g. #define LV_FONT_CUSTOM_DECLARE   LV_FONT_DECLARE(my_font_1) LV_FONT_DECLARE(my_font_2) */
#define LV_FONT_CUSTOM_DECLARE

/* Always set a default font */
#define LV_FONT_DEFAULT &lv_font_montserrat_14

/* Enable handling large font and/or fonts with a lot of characters.
 * The limit depends on the font size, font face and bpp but with > 10,000 characters
 * if the font needs to be compressed more compactly.
 * [0: disable, 1: enable] */
#define LV_FONT_FMT_TXT_LARGE 0

/* Enables/disables support for compressed fonts. */
#define LV_USE_FONT_COMPRESSED 0

/* Enable subpixel rendering */
#define LV_USE_FONT_SUBPX 0
#if LV_USE_FONT_SUBPX
    /* Set the pixel order of the display. Physical order of RGB channels. Doesn't matter with "normal" fonts. */
    #define LV_FONT_SUBPX_BGR 0  /* 0: RGB; 1:BGR order */
#endif

/* Enable drawing placeholders when glyph dsc is not found */
#define LV_USE_FONT_PLACEHOLDER 1

/*=================
 * TEXT SETTINGS
 *=================*/

/**
 * Select a character encoding for strings.
 * Your IDE or editor should have the same character encoding
 * - LV_TXT_ENC_UTF8
 * - LV_TXT_ENC_ASCII
 */
#define LV_TXT_ENC LV_TXT_ENC_UTF8

/* Can break (wrap) texts on these chars */
#define LV_TXT_BREAK_CHARS " ,.;:-_"

/* If a word is at least this long, will break wherever "prettiest"
 * To disable, set to a value <= 0 */
#define LV_TXT_LINE_BREAK_LONG_LEN 0

/* Minimum number of characters in a long word to put on a line before a break.
 * Depends on LV_TXT_LINE_BREAK_LONG_LEN. */
#define LV_TXT_LINE_BREAK_LONG_PRE_MIN_LEN 3

/* Minimum number of characters in a long word to put on a line after a break.
 * Depends on LV_TXT_LINE_BREAK_LONG_LEN. */
#define LV_TXT_LINE_BREAK_LONG_POST_MIN_LEN 3

/* The control character to use for signalling text recoloring. */
#define LV_TXT_COLOR_CMD "#"

/* Support bidirectional texts. Allows mixing Left-to-Right and Right-to-Left texts.
 * The direction will be processed according to the Unicode Bidirectional Algorithm:
 * https://www.unicode.org/reports/tr9/*/
#define LV_USE_BIDI 0
#if LV_USE_BIDI
    /* Set the default direction. Supported values:
     * `LV_BASE_DIR_LTR` Left-to-Right
     * `LV_BASE_DIR_RTL` Right-to-Left
     * `LV_BASE_DIR_AUTO` detect texts base direction */
    #define LV_BIDI_BASE_DIR_DEF LV_BASE_DIR_AUTO
#endif

/* Enable Arabic/Persian processing
 * In these languages characters should be replaced with an other form based on their position in the text */
#define LV_USE_ARABIC_PERSIAN_CHARS 0

/*===================
 * WIDGET USAGE
 *===================*/

/* Documentation of the widgets: https://docs.lvgl.io/latest/en/html/widgets/index.html */

#define LV_USE_ARC        1
#define LV_USE_BAR        1
#define LV_USE_BTN        1
#define LV_USE_BTNMATRIX  1
#define LV_USE_CANVAS     1
#define LV_USE_CHECKBOX   1
#define LV_USE_DROPDOWN   1   /* Requires: lv_label */
#define LV_USE_IMG        1   /* Requires: lv_label */
#define LV_USE_LABEL      1
#define LV_USE_LINE       1
#define LV_USE_ROLLER     1   /* Requires: lv_label */
#define LV_USE_SLIDER     1   /* Requires: lv_bar */
#define LV_USE_SWITCH     1
#define LV_USE_TEXTAREA   1   /* Requires: lv_label */
#define LV_USE_TABLE      1   /* Requires: lv_label */

/*==================
 * EXTRA COMPONENTS
 *==================*/

/* 1: Enable the extra "components" */
#define LV_USE_EXTRA_COMPONENTS 1

#if LV_USE_EXTRA_COMPONENTS

    /*-----------
     * Widgets
     *----------*/
    #define LV_USE_ANIMIMG    1
    #define LV_USE_CALENDAR   1
    #define LV_USE_CHART      1
    #define LV_USE_COLORWHEEL 1
    #define LV_USE_IMGBTN     1
    #define LV_USE_KEYBOARD   1
    #define LV_USE_LED        1
    #define LV_USE_LIST       1
    #define LV_USE_MENU       1
    #define LV_USE_METER      1
    #define LV_USE_MSGBOX     1
    #define LV_USE_SPINBOX    1
    #define LV_USE_SPINNER    1
    #define LV_USE_TABVIEW    1
    #define LV_USE_TILEVIEW   1
    #define LV_USE_WIN        1
    #define LV_USE_SPAN       1

    /*-----------
     * Themes
     *----------*/
    #define LV_USE_THEME_DEFAULT 1
    #if LV_USE_THEME_DEFAULT
        /*0: Light mode; 1: Dark mode*/
        #define LV_THEME_DEFAULT_DARK 1
        /*1: Enable grow on press*/
        #define LV_THEME_DEFAULT_GROW 1
        /*Default transition time in [ms]*/
        #define LV_THEME_DEFAULT_TRANSITION_TIME 80
    #endif /*LV_USE_THEME_DEFAULT*/

    #define LV_USE_THEME_BASIC 1
    #define LV_USE_THEME_MONO  1

    /*-----------
     * Layouts
     *----------*/
    #define LV_USE_FLEX 1
    #define LV_USE_GRID 1

    /*---------------------
     * 3rd party libraries
     *--------------------*/
    #define LV_USE_FS_STDIO 1
    #define LV_USE_FS_POSIX 0
    #define LV_USE_FS_WIN32 0
    #define LV_USE_FS_FATFS 1

    #define LV_USE_PNG 0
    #define LV_USE_BMP 0
    #define LV_USE_SJPG 0
    #define LV_USE_GIF 0
    #define LV_USE_QRCODE 0
    #define LV_USE_FREETYPE 0
    #define LV_USE_RLOTTIE 0
    #define LV_USE_FFMPEG 0

    /*-----------
     * Others
     *----------*/
    #define LV_USE_SNAPSHOT 1
    #define LV_USE_MONKEY   0
    #define LV_USE_GRIDNAV  0
    #define LV_USE_FRAGMENT 0
    #define LV_USE_IMGFONT  0
    #define LV_USE_MSG      0
    #define LV_USE_IME_PINYIN 0

    /*==================
    * EXAMPLES
    *==================*/
    #define LV_BUILD_EXAMPLES 0

#endif /*LV_USE_EXTRA_COMPONENTS*/

/*--END OF LV_CONF_H--*/

#endif /*LV_CONF_H*/