#include "main.h"

static Window *s_window;
static MenuLayer *s_menulayer;

static bool check_phone_message = false;

static void initialise_ui(void) {
  s_window = window_create();
  #ifndef PBL_SDK_3
    window_set_fullscreen(s_window, true);
  #endif
  
  GRect bounds = layer_get_bounds(window_get_root_layer(s_window));
  
  // s_menulayer
  s_menulayer = menu_layer_create(GRect(0, 0, bounds.size.w, bounds.size.h));
  menu_layer_set_click_config_onto_window(s_menulayer, s_window);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_menulayer);
}

static void destroy_ui(void) {
  window_destroy(s_window);
  menu_layer_destroy(s_menulayer);
}

// *****************************************************************************************************
// MENU CALLBACKS
// *****************************************************************************************************

enum { // main menu structure
  MENU_SECTION_JOBS,
  MENU_SECTION_SETTINGS,
  
  NUM_MENU_SECTIONS,
  
  MENU_SETTINGS_ADD=MENU_SECTION_SETTINGS*100,
  MENU_SETTINGS_CONFIG,
  NUM_MENU_ITEMS_SETTINGS=2
};

static uint16_t menu_get_num_sections_callback(MenuLayer *menu_layer, void *data) {
  return NUM_MENU_SECTIONS;
}

static uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  switch (section_index) {
    case MENU_SECTION_JOBS: return jobs_count;
    case MENU_SECTION_SETTINGS: return NUM_MENU_ITEMS_SETTINGS;
    default:
      return 0;
  }
}

#ifndef PBL_ROUND
static int16_t menu_get_header_height_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  // This is a define provided in pebble.h that you may use for the default height
  return (section_index==MENU_SECTION_SETTINGS) ? MENU_CELL_BASIC_HEADER_HEIGHT : 0;
}
#endif

static void menu_draw_header_callback(GContext* ctx, const Layer *cell_layer, uint16_t section_index, void *data) {
  // Determine which section we're working with
  if (section_index==MENU_SECTION_SETTINGS) menu_cell_basic_header_draw(ctx, cell_layer, "Options");
}

static int16_t menu_get_cell_height_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  if (cell_index->section == MENU_SECTION_JOBS) {
  //  #ifdef PBL_ROUND
  //  MenuIndex si=menu_layer_get_selected_index(menu_layer);
  //  return (cell_index->section == si.section && cell_index->row==si.row) ? MENU_HEIGHT_JOB : MENU_HEIGHT_SINGLE;
  //  #else
    return MENU_HEIGHT_JOB;
  //  #endif
  }
  return MENU_HEIGHT_SINGLE;
}

void menu_cell_draw_job(GContext* ctx, const Layer *cell_layer, const uint8_t index) {
  GRect bounds = layer_get_frame(cell_layer);
  
  // http://newscentral.exsees.com/item/ac0cacd0083161de2ffe8161eb40fa51-15e3726b28defcbc9eb59ade232b5de3
  #ifdef PBL_SDK_3
  graphics_context_set_compositing_mode(ctx, GCompOpSet); // transparency
  #else
  graphics_context_set_text_color(ctx, GColorBlack);
  #endif
  
  #ifdef PBL_ROUND
  uint8_t name_width=graphics_text_layout_get_content_size(jobs[index].Name, FONT_GOTHIC_24_BOLD, GRect(4+ROUND_MARGIN, -4, bounds.size.w-8-16-2*ROUND_MARGIN, 4+18), GTextOverflowModeFill, GTextAlignmentLeft).w;
  uint8_t num_width =graphics_text_layout_get_content_size(jobs_get_job_count_as_text(index), FONT_GOTHIC_18, GRect(4+ROUND_MARGIN, 2, bounds.size.w-8-2*ROUND_MARGIN, 14), GTextOverflowModeFill, GTextAlignmentRight).w;
  uint8_t total_width=name_width+num_width+12;
  graphics_draw_text(ctx, jobs[index].Name, FONT_GOTHIC_24_BOLD, GRect((bounds.size.w-total_width)/2, -4, name_width, 4+18), GTextOverflowModeFill, GTextAlignmentLeft, NULL);
  graphics_draw_text(ctx, jobs_get_job_count_as_text(index), FONT_GOTHIC_18, GRect((bounds.size.w-total_width)/2, 0, total_width, 14), GTextOverflowModeFill, GTextAlignmentRight, NULL);
  #else
  graphics_draw_text(ctx, jobs[index].Name, FONT_GOTHIC_24_BOLD, GRect(4+ROUND_MARGIN, -4, bounds.size.w-8-16-2*ROUND_MARGIN, 4+18), GTextOverflowModeFill, GTextAlignmentLeft, NULL);
  graphics_draw_text(ctx, jobs_get_job_count_as_text(index), FONT_GOTHIC_18, GRect(4+ROUND_MARGIN, 0, bounds.size.w-8-2*ROUND_MARGIN, 14), GTextOverflowModeFill, GTextAlignmentRight, NULL);
  #endif
  
  char* stickers=jobs[index].Stickers; // always returns a zero padded string
  if (*stickers) {
    while (*(stickers+EMOJI_CHILD_COLS)) stickers+=EMOJI_CHILD_COLS;
    #ifdef PBL_ROUND
    uint8_t n=0;
    while (stickers[n]) n++;
    #endif
    
    uint8_t cell = (bounds.size.w-2*ROUND_MARGIN) / EMOJI_CHILD_COLS;
    uint8_t marg = (cell - EMOJI_WIDTH)/2+1+PBL_IF_ROUND_ELSE(ROUND_MARGIN+(EMOJI_CHILD_COLS-n)*cell/2, 0);
    uint8_t i=0;
    while (*stickers) {
      uint8_t emoji=*stickers-1;
      graphics_draw_bitmap_in_rect(ctx, EMOJI_INDEX(emoji, PBL_IF_SDK_3_APLITE_ELSE(menu_cell_layer_is_highlighted(cell_layer), 0)), GRect(marg+cell*(i++),23, EMOJI_WIDTH, EMOJI_HEIGHT));
      stickers++;
    }
  } else {
    graphics_draw_text(ctx, "PRESS to add sticker", FONT_GOTHIC_14, GRect(4, 20, bounds.size.w-8, 16), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
    if (!job_menu_visible()) graphics_draw_text(ctx, "HOLD to edit", FONT_GOTHIC_14, GRect(4, 36, bounds.size.w-8, 16), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
  }
}

#define MENU_ICON_SPACE PBL_IF_ROUND_ELSE(0,28)

void menu_cell_draw_other(GContext* ctx, const Layer *cell_layer, const char *title, GBitmap ** icon) {
  GRect bounds = layer_get_frame(cell_layer);
  
  #ifndef PBL_SDK_3
  graphics_context_set_text_color(ctx, GColorBlack);
  #endif
  
  graphics_draw_text(ctx, title, FONT_GOTHIC_24_BOLD, GRect(MENU_ICON_SPACE, -4, bounds.size.w-MENU_ICON_SPACE, 4+18), GTextOverflowModeFill, PBL_IF_ROUND_ELSE(GTextAlignmentCenter, GTextAlignmentLeft), NULL);
  #ifdef PBL_ROUND
  uint8_t text_width=graphics_text_layout_get_content_size(title, FONT_GOTHIC_24_BOLD, GRect(MENU_ICON_SPACE, -4, bounds.size.w-MENU_ICON_SPACE, 4+18), GTextOverflowModeFill, PBL_IF_ROUND_ELSE(GTextAlignmentCenter, GTextAlignmentLeft)).w;  
  if (icon) graphics_draw_bitmap_in_rect(ctx, icon[PBL_IF_SDK_3_ELSE(menu_cell_layer_is_highlighted(cell_layer), 0)], GRect((bounds.size.w-text_width)/2-16-6,(bounds.size.h-16)/2, 16, 16));
  #else
  if (icon) graphics_draw_bitmap_in_rect(ctx, icon[PBL_IF_SDK_3_ELSE(menu_cell_layer_is_highlighted(cell_layer), 0)], GRect(6,(bounds.size.h-16)/2, 16, 16));
  #endif
}

static void menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
  switch (cell_index->section) {
    case MENU_SECTION_JOBS:
      menu_cell_draw_job(ctx, cell_layer, cell_index->row);
      break;

    default:
      switch (MENU_SECTION_CELL) {
        case MENU_SETTINGS_ADD: menu_cell_draw_other(ctx, cell_layer, "Add Child", bitmaps[BITMAP_ADD]); break;
        case MENU_SETTINGS_CONFIG: menu_cell_draw_other(ctx, cell_layer, check_phone_message ? "Check phone..." : "Config/Donate", bitmaps[BITMAP_SETTINGS]); break;
      }
  }
}

static void timer_callback(void *data) {
    check_phone_message=false;
    menu_layer_reload_data(s_menulayer);
}

static void menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  switch (cell_index->section) {
    case MENU_SECTION_JOBS:
      emoji_menu_show(cell_index->row);
      break;
    default:
      switch (MENU_SECTION_CELL) {
        case MENU_SETTINGS_ADD: jobs_add_job(); break;
        case MENU_SETTINGS_CONFIG:
          export_after_save=true;
          main_save_data(1);
          check_phone_message=true;
          menu_layer_reload_data(s_menulayer);
          app_timer_register(2000 /* milliseconds */, timer_callback, NULL);
          break;
      }
  }
}

static void menu_select_long_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  if (cell_index->section == MENU_SECTION_JOBS) job_menu_show(cell_index->row);
}

// *****************************************************************************************************
// MAIN
// *****************************************************************************************************

static void handle_window_unload(Window* window) {
  destroy_ui();
  s_window=NULL;
}

void main_menu_update(void) {
  if (s_window) menu_layer_reload_data(s_menulayer);
}

void main_menu_show(void) {
  initialise_ui();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .unload = handle_window_unload,
  });
  // Set all the callbacks for the menu layer
  menu_layer_set_callbacks(s_menulayer, NULL, (MenuLayerCallbacks){
    .get_num_sections = menu_get_num_sections_callback,
    .get_num_rows = menu_get_num_rows_callback,
    .get_header_height = PBL_IF_ROUND_ELSE(NULL,menu_get_header_height_callback),
    .get_cell_height = menu_get_cell_height_callback,
    .draw_header = menu_draw_header_callback,
    .draw_row = menu_draw_row_callback,
    .select_click = menu_select_callback,
    .select_long_click = menu_select_long_callback
  });
  window_stack_push(s_window, ANIMATED);
}

void main_menu_hide(void) {
  window_stack_remove(s_window, ANIMATED);
}

void main_menu_highlight_job(const int index) {
  menu_layer_reload_data(s_menulayer);
  menu_layer_set_selected_index(s_menulayer, MenuIndex(0,index), MenuRowAlignTop, ANIMATED);
}