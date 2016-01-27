#include "main.h"

uint8_t job_index;
bool job_changed;

// BEGIN AUTO-GENERATED UI CODE; DO NOT MODIFY
static Window *s_window=NULL;
static MenuLayer *s_menulayer;

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
// END AUTO-GENERATED UI CODE

// *****************************************************************************************************
// MENU CALLBACKS
// *****************************************************************************************************

enum { // main menu structure
  MENU_JOB,
  MENU_RENAME,
  MENU_SUB,
  MENU_DELETE,
  
  NUM_MENU_ROWS
};

static uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  return NUM_MENU_ROWS;
}

static int16_t menu_get_cell_height_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  return (cell_index->row == MENU_JOB) ? MENU_HEIGHT_JOB : MENU_HEIGHT_SINGLE;
}

static void menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
  switch (cell_index->row) {
    case MENU_JOB: menu_cell_draw_job(ctx, cell_layer, job_index); break;
    case MENU_RENAME: menu_cell_draw_other(ctx, cell_layer, "Rename", bitmaps[BITMAP_EDIT]); break;
    case MENU_SUB: menu_cell_draw_other(ctx, cell_layer, "Remove sticker", bitmaps[BITMAP_MINUS]); break;
    case MENU_DELETE: menu_cell_draw_other(ctx, cell_layer, "Delete child", bitmaps[BITMAP_DELETE]); break;
  }
}

static void menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  switch (cell_index->row) {
    case MENU_JOB: emoji_menu_show(cell_index->row); break;
    case MENU_RENAME: jobs_rename_job(job_index); break;
    case MENU_SUB:
      if (jobs_delete_sticker(job_index)) layer_mark_dirty((Layer*) menu_layer);
      break;
    case MENU_DELETE: 
      jobs_delete_job_and_save(job_index);
      job_menu_hide();
      break;
  }
}

// *****************************************************************************************************
// MAIN
// *****************************************************************************************************

static void handle_window_unload(Window* window) {
  destroy_ui();
  s_window=NULL;
  if (job_changed) {
    main_save_data(0);
  }
}

void job_menu_show(uint8_t index) {
  job_index=index;
  job_changed=false;
  initialise_ui();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .unload = handle_window_unload,
  });
  // Set all the callbacks for the menu layer
  menu_layer_set_callbacks(s_menulayer, NULL, (MenuLayerCallbacks){
    .get_num_sections = NULL, //menu_get_num_sections_callback,
    .get_num_rows = menu_get_num_rows_callback,
    .get_header_height = NULL, //menu_get_header_height_callback,
    .get_cell_height = menu_get_cell_height_callback,
    .draw_header = NULL, //menu_draw_header_callback,
    .draw_row = menu_draw_row_callback,
    .select_click = menu_select_callback,
    .select_long_click = NULL //menu_select_long_callback
  });
  window_stack_push(s_window, true);
}

void job_menu_hide(void) {
  window_stack_remove(s_window, true);
}

void job_menu_update(void) {
  if (s_window) menu_layer_reload_data(s_menulayer);
}

bool job_menu_visible(void) {
  return s_window && window_is_loaded(s_window);
}