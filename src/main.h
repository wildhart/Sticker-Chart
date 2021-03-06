#pragma once

#include <pebble.h>
#include "jobs.h"
#include "main_menu.h"
#include "job_menu.h"
#include "emoji_menu.h"
#include "treat_window.h"
#include "tertiary_text.h"

#define DISABLE_LOGGING true

#if DISABLE_LOGGING
#define LOG(...)
#define DEBUG(...)
#define INFO(...)
#define WARN(...)
#define ERROR(...)
#else
#define LOG(...) app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define DEBUG(...) app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define INFO(...) app_log(APP_LOG_LEVEL_INFO, __FILE__, __LINE__, __VA_ARGS__)
#define WARN(...) app_log(APP_LOG_LEVEL_WARNING, __FILE__, __LINE__, __VA_ARGS__)
#define ERROR(...) app_log(APP_LOG_LEVEL_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define HEAP(text) APP_LOG(APP_LOG_LEVEL_INFO, "heap: %d, used: %d, free: %d, %s %s",  heap_bytes_used()+heap_bytes_free(), heap_bytes_used(), heap_bytes_free(), __func__, text)
#endif

#ifdef PBL_SDK_3
#define PBL_IF_SDK_3(X)          (X)
#define PBL_IF_SDK_3_ELSE(X, Y)  (X)
  #ifdef PBL_PLATFORM_APLITE
  #define PBL_SDK_3_APLITE                1
  #define PBL_IF_SDK_3_APLITE_ELSE(X, Y)  (X)
  #else
  #define PBL_IF_SDK_3_APLITE_ELSE(X, Y)  (Y)
  #endif
#else // PBL_SDK_3
  #define PBL_IF_SDK_3_APLITE_ELSE(X, Y)  (Y)
#define PBL_IF_SDK_3(X)
#define PBL_IF_SDK_3_ELSE(X, Y)  (Y)
#define PBL_IF_COLOR_ELSE(X, Y)  (Y)
#define PBL_IF_ROUND_ELSE(X, Y)  (Y)
#define PBL_IF_BW_ELSE(X, Y)     (X)
#define gbitmap_set_bounds(bmp, new_bounds) ((bmp)->bounds = (new_bounds))
#define GColorLightGray          GColorBlack
#define GColorDarkGray          GColorBlack
#endif // PBL_SDK_3
#define ROUND_MARGIN          PBL_IF_ROUND_ELSE(2,0)

#define ANIMATED true
#define HIDDEN true

#define END_TIME(JOB) ((time_t) (JOB)->Seconds + (time_t) (JOB)->Repeat_hrs*3600)

#define FONT_GOTHIC_24_BOLD           fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD)
#define FONT_GOTHIC_18_BOLD           fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD)
#define FONT_GOTHIC_14_BOLD           fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD)
#define FONT_GOTHIC_18                fonts_get_system_font(FONT_KEY_GOTHIC_18)
#define FONT_GOTHIC_14                fonts_get_system_font(FONT_KEY_GOTHIC_14)
#define FONT_BITHAM_30_BLACK          fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK)
#define FONT_BITHAM_34_MEDIUM_NUMBERS fonts_get_system_font(FONT_KEY_BITHAM_34_MEDIUM_NUMBERS)

#define ICON_RECT_PLAY        (GRect) { {  0,  0 }, { 16, 16 } }
#define ICON_RECT_PAUSE       (GRect) { { 16,  0 }, { 16, 16 } }
#define ICON_RECT_STOP        (GRect) { { 32,  0 }, { 16, 16 } }
#define ICON_RECT_TICK        (GRect) { { 16, 16 }, { 16, 16 } }
#define ICON_RECT_TIMER       (GRect) { {  0, 16 }, {  8, 16 } }
#define ICON_RECT_STOPWATCH   (GRect) { {  8, 16 }, {  8, 16 } }
#define ICON_RECT_ADJUST      (GRect) { {  0, 16 }, { 16, 16 } }
#define ICON_RECT_ADD         (GRect) { { 48, 16 }, { 16, 16 } }
#define ICON_RECT_RESET       (GRect) { { 48,  0 }, { 16, 16 } }
#define ICON_RECT_DELETE      (GRect) { {  0, 32 }, { 16, 16 } }
#define ICON_RECT_EDIT        (GRect) { { 16, 32 }, { 16, 16 } }
#define ICON_RECT_CONTROLS    (GRect) { {  0, 48 }, { 16, 16 } }
#define ICON_RECT_ABOUT       (GRect) { { 48, 32 }, { 16, 16 } }
#define ICON_RECT_SETTINGS    (GRect) { { 32, 32 }, { 16, 16 } }
#define ICON_RECT_MINUS       (GRect) { { 16, 48 }, { 16, 16 } }
#define ICON_RECT_CLOCK       (GRect) { { 32, 16 }, { 16, 16 } }

enum  {
  BITMAP_MATRIX,
  BITMAP_ADD,
  BITMAP_SETTINGS,
  BITMAP_DELETE,
  BITMAP_EDIT,
  BITMAP_MINUS,
  N_BITMAPS
};

#define MAX_EMOJI_PAGES 5
#define EMOJI_PAGE_COLS 4
#define EMOJI_PAGE_ROWS 5
#define EMOJI_CHILD_COLS  5
#define EMOJI_CHILD_ROWS  4
#define EMOJI_PAGE_EMOJIS (EMOJI_PAGE_COLS*EMOJI_PAGE_ROWS)
#define EMOJI_WIDTH 28
#define EMOJI_HEIGHT 33

#define EMOJI_PAGE_WIDTH (EMOJI_WIDTH*EMOJI_PAGE_COLS)
#define EMOJI_PAGE_HEIGHT (EMOJI_HEIGHT*EMOJI_PAGE_ROWS)

extern GBitmap *bitmaps[N_BITMAPS][PBL_IF_SDK_3_ELSE(2,1)];
#ifdef PBL_ROUND
#define ROUND_TAB_SIZE 22
extern GBitmap *round_tabs[MAX_EMOJI_PAGES];
#endif
  
#define MENU_SECTION_CELL  (cell_index->section * 100 + cell_index->row)
#define MENU_HEIGHT_SINGLE 24
#define MENU_HEIGHT_DOUBLE 42
#define MENU_HEIGHT_JOB    20+EMOJI_HEIGHT+4

// Persistent Storage Keys
#define STORAGE_KEY_VERSION      1
#define STORAGE_KEY_TIMESTAMP    2
#define STORAGE_KEY_FIRST_CHILD  100

#define CURRENT_STORAGE_VERSION 1

extern bool export_after_save;

#define EMOJI_INDEX(I,INV) (main_get_emoji((I)/EMOJI_PAGE_EMOJIS, (I)%EMOJI_PAGE_COLS, ((I)%EMOJI_PAGE_EMOJIS)/EMOJI_PAGE_COLS, (INV)))
GBitmap* main_get_emoji(const uint8_t page, const uint8_t x, const uint8_t y, const bool inverted);
void main_save_data(const uint32_t timestamp);