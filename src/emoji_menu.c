#include "main.h"

static Window *s_window;
static Layer *graphics_layer;

static uint8_t child_index;
static int8_t page_selected;
static uint8_t tab_selected;
static uint8_t emoji_selected;
enum {
  PAGE_CHILD,
  PAGE_TABS,
  PAGE_EMOJIS,
  NUM_PAGES
};

static void graphics_layer_update_callback(Layer *layer, GContext *ctx) {
  uint8_t w,h,ox,oy,tw,tox,toy;
  GRect bounds = layer_get_frame(layer);
  graphics_context_set_compositing_mode(ctx, GCompOpSet); // transparency
  
  // tabs
  tw = bounds.size.w / MAX_EMOJI_PAGES;
  tox = 2+(tw - EMOJI_WIDTH)/2;
  toy = 2;
  // emojis
  w = bounds.size.w / EMOJI_PAGE_COLS;
  h = (bounds.size.h) / (EMOJI_PAGE_ROWS+1);
  ox = 2+(w - EMOJI_WIDTH)/2;
  oy = toy + 1 + h + (h - EMOJI_HEIGHT)/2;
  
  if (page_selected==PAGE_CHILD) {
    // Draw Childs name and # stickers;
      graphics_context_set_text_color(ctx, GColorBlack);
      graphics_draw_text(ctx, jobs[child_index].Name, FONT_GOTHIC_24_BOLD, GRect(4, -4, bounds.size.w-8-16, 4+18), GTextOverflowModeFill, GTextAlignmentLeft, NULL);
      graphics_draw_text(ctx, jobs_get_job_count_as_text(child_index), FONT_GOTHIC_18, GRect(4, 2, bounds.size.w-8, 14), GTextOverflowModeFill, GTextAlignmentRight, NULL);
      graphics_draw_text(ctx,"Press any button to add \U00002192", FONT_GOTHIC_14, GRect(0, 20, bounds.size.w-2, 14), GTextOverflowModeFill, GTextAlignmentRight, NULL);
  } else if (page_selected==PAGE_TABS) {
    // Draw all tab icons
    for (uint8_t x=0; x<MAX_EMOJI_PAGES; x++) graphics_draw_bitmap_in_rect(ctx, main_get_emoji(x,0,0), GRect(tox+tw*x, toy, EMOJI_WIDTH, EMOJI_HEIGHT));
  } else if (page_selected==PAGE_EMOJIS) {
    // Draw selected tab
    graphics_draw_bitmap_in_rect(ctx, main_get_emoji(tab_selected,0,0), GRect(tox+tw*tab_selected, toy, EMOJI_WIDTH, EMOJI_HEIGHT));
  }
  
  // Draw emoji grid
  w = bounds.size.w / EMOJI_PAGE_COLS;
  h = (bounds.size.h) / (EMOJI_PAGE_ROWS+1);
  ox = 2+(w - EMOJI_WIDTH)/2;
  oy = toy + 1 + h + (h - EMOJI_HEIGHT)/2;
  
  if (page_selected==PAGE_CHILD) {
    // draw childs stickers
    char* stickers=jobs[child_index].Stickers;
    uint8_t emoji;
    uint8_t i=0;
    while (*stickers) {
      emoji = *stickers-1;
      graphics_draw_bitmap_in_rect(ctx, EMOJI_INDEX(emoji), GRect(ox+w*(i%EMOJI_PAGE_COLS), oy+h*(i/EMOJI_PAGE_COLS), EMOJI_WIDTH, EMOJI_HEIGHT));
      stickers++;
      i++;
    }
  } else {
    // draw emoji grid
    for (uint8_t y=0; y<EMOJI_PAGE_ROWS; y++) {
      for (uint8_t x=0; x<EMOJI_PAGE_COLS; x++) {
        graphics_draw_bitmap_in_rect(ctx, main_get_emoji(tab_selected,x,y), GRect(ox+w*x, oy+h*y, EMOJI_WIDTH, EMOJI_HEIGHT));
      }
    }
  }
  
  if (page_selected!=PAGE_CHILD) {
    // Draw tab lines
    graphics_context_set_stroke_color(ctx,GColorBlack);
    graphics_draw_round_rect(ctx, GRect(tox+tw*tab_selected-2, 0, EMOJI_WIDTH+5, toy+EMOJI_HEIGHT+1),3);
    graphics_draw_round_rect(ctx, GRect(tox+tw*tab_selected-1, 1, EMOJI_WIDTH+3, toy+EMOJI_HEIGHT),2);
    graphics_draw_rect(ctx, GRect(0, toy+EMOJI_HEIGHT-1, bounds.size.w, 2));
    graphics_context_set_stroke_color(ctx,GColorWhite);
    graphics_draw_rect(ctx, GRect(tox+tw*tab_selected, toy+EMOJI_HEIGHT-1, EMOJI_WIDTH+1, 2));
  }  
    
  if (page_selected==PAGE_EMOJIS) {    
    // draw gray box behind tabs
    graphics_context_set_fill_color(ctx,GColorLightGray);
    graphics_fill_rect(ctx, GRect(0, 0, tox+tw*tab_selected-1, toy+EMOJI_HEIGHT), 0, GCornerNone);
    graphics_fill_rect(ctx, GRect(tox+tw*tab_selected-1+EMOJI_WIDTH+3, 0, bounds.size.w, toy+EMOJI_HEIGHT), 0, GCornerNone);
    // highlight selected emoji
    uint8_t x = emoji_selected % EMOJI_PAGE_COLS;
    uint8_t y = emoji_selected / EMOJI_PAGE_COLS;
    graphics_context_set_stroke_color(ctx,GColorBlack);
    graphics_draw_round_rect(ctx, GRect(ox+w*x-2, oy+h*y, EMOJI_WIDTH+4, EMOJI_HEIGHT),3);     // outer square
    graphics_draw_round_rect(ctx, GRect(ox+w*x-1, oy+h*y+1, EMOJI_WIDTH+2, EMOJI_HEIGHT-2),3); // inner square
  }
}

static void select_click_handler(void) {
  if (page_selected==PAGE_EMOJIS) {
    if (jobs_add_sticker(child_index, tab_selected * EMOJI_PAGE_EMOJIS + emoji_selected) % EMOJI_PAGE_COLS == 0) {
      treat_window_show(jobs[child_index].Name);
    }
    tab_selected=emoji_selected=0;
  }
  page_selected=(page_selected+1) % NUM_PAGES;
  layer_mark_dirty(graphics_layer);
}

static void back_click_handler(void) {
  page_selected--;
  if (page_selected<PAGE_CHILD) {
    emoji_menu_hide();
  } else {
    layer_mark_dirty(graphics_layer);
  }
}

static void up_click_handler(void) {
  if (page_selected==PAGE_CHILD) {
    page_selected++;
  } else if (page_selected==PAGE_TABS) {
    tab_selected=(tab_selected+MAX_EMOJI_PAGES-1) % MAX_EMOJI_PAGES;
  } else {
    emoji_selected=(emoji_selected+EMOJI_PAGE_EMOJIS-1) % EMOJI_PAGE_EMOJIS;
  }
  layer_mark_dirty(graphics_layer);
}

static void down_click_handler(void) {
  if (page_selected==PAGE_CHILD) {
    page_selected++;
  } else if (page_selected==PAGE_TABS) {
    tab_selected=(tab_selected+1) % MAX_EMOJI_PAGES;
  } else {
    emoji_selected=(emoji_selected+1) % EMOJI_PAGE_EMOJIS;
  }
  layer_mark_dirty(graphics_layer);
}

static void click_config_provider(void) {
  window_single_click_subscribe(BUTTON_ID_SELECT, (ClickHandler) select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, (ClickHandler) up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, (ClickHandler) down_click_handler);
  window_single_click_subscribe(BUTTON_ID_BACK, (ClickHandler) back_click_handler);
  window_single_repeating_click_subscribe(BUTTON_ID_UP, 100, (ClickHandler) up_click_handler);
  window_single_repeating_click_subscribe(BUTTON_ID_DOWN, 100, (ClickHandler) down_click_handler);
}
  
static void initialise_ui(void) {
  s_window = window_create();
  #ifndef PBL_SDK_3
    window_set_fullscreen(s_window, true);
  #endif
  
  GRect bounds = layer_get_bounds(window_get_root_layer(s_window));
  // Initialise the general graphics layer
  
  graphics_layer = layer_create(bounds);
  layer_set_update_proc(graphics_layer, graphics_layer_update_callback);
  layer_add_child(window_get_root_layer(s_window), (Layer *)graphics_layer);
}

static void destroy_ui(void) {
  layer_destroy(graphics_layer);
  window_destroy(s_window);
}

static void handle_window_unload(Window* window) {
  destroy_ui();
}

void emoji_menu_show(uint8_t index) {
  child_index=index;
  page_selected=PAGE_CHILD;
  tab_selected=emoji_selected=0;
  initialise_ui();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .unload = handle_window_unload,
  });
  window_set_click_config_provider(s_window, (ClickConfigProvider) click_config_provider);
  window_stack_push(s_window, true);
}

void emoji_menu_hide(void) {
  window_stack_remove(s_window, true);
}
