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

#ifdef PBL_ROUND
static uint8_t round_xy[EMOJI_PAGE_EMOJIS][2];
#define ROUND_TAB_INSET  (ROUND_TAB_SIZE/2 + 4)
#define ROUND_TAB_ANGLE  (19)
#define POLAR_RECT(R,A)  grect_centered_from_polar(GRect((R).origin.x+ROUND_TAB_INSET,(R).origin.y+ROUND_TAB_INSET,(R).size.w-ROUND_TAB_INSET*2,(R).size.h-ROUND_TAB_INSET*2), GOvalScaleModeFitCircle, DEG_TO_TRIGANGLE(90-2*ROUND_TAB_ANGLE+(A)*ROUND_TAB_ANGLE), GSize(ROUND_TAB_SIZE, ROUND_TAB_SIZE))
#endif

static void graphics_layer_update_callback(Layer *layer, GContext *ctx) {
  uint8_t w,h,ox,oy,th,tox,toy;
  GRect bounds = layer_get_frame(layer);
  // http://newscentral.exsees.com/item/ac0cacd0083161de2ffe8161eb40fa51-15e3726b28defcbc9eb59ade232b5de3
  #ifdef PBL_SDK_3
    graphics_context_set_compositing_mode(ctx, GCompOpSet); // transparency
  #endif
  
  if (page_selected==PAGE_CHILD) {  
    // Draw Childs name and # stickers;
    graphics_context_set_text_color(ctx, GColorBlack);
    graphics_context_set_fill_color(ctx,GColorLightGray);
    #ifdef PBL_ROUND
      graphics_draw_text(ctx, jobs[child_index].Name, FONT_GOTHIC_24_BOLD, GRect(0, 8, bounds.size.w, 8+18), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
      graphics_draw_text(ctx, jobs_get_job_count_as_text(child_index), FONT_GOTHIC_18, GRect(0, 150, bounds.size.w, 14), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
      
      graphics_context_set_fill_color(ctx,GColorLightGray);
      graphics_context_set_text_color(ctx, GColorBlack);
      graphics_fill_rect(ctx, GRect(162,0,bounds.size.w-162,bounds.size.h), 0, GCornerNone);
      graphics_draw_text(ctx, "+", FONT_GOTHIC_24_BOLD, GRect(162,bounds.size.h/2-16,bounds.size.w-162,30),GTextOverflowModeFill, GTextAlignmentCenter, NULL);
    #else
      graphics_draw_text(ctx, jobs[child_index].Name, FONT_GOTHIC_24_BOLD, GRect(4, -4, bounds.size.w-8-16, 4+18), GTextOverflowModeFill, GTextAlignmentLeft, NULL);
      graphics_draw_text(ctx, jobs_get_job_count_as_text(child_index), FONT_GOTHIC_18, GRect(4, 2, bounds.size.w-8, 14), GTextOverflowModeFill, GTextAlignmentRight, NULL);
      graphics_draw_text(ctx,PBL_IF_COLOR_ELSE("Press any button to add \U00002192","Press any button to add ->"), FONT_GOTHIC_14, GRect(0, 20, bounds.size.w-2, 14), GTextOverflowModeFill, GTextAlignmentRight, NULL);
    #endif
  } 
  // tabs
  #ifndef PBL_ROUND
    th = bounds.size.h / MAX_EMOJI_PAGES;
    tox = bounds.size.w - EMOJI_WIDTH;
    toy = 2+(th - EMOJI_HEIGHT)/2; 
  // grid 
    w = bounds.size.w / (EMOJI_PAGE_COLS+1);
    h = bounds.size.h / EMOJI_PAGE_ROWS;
    ox = 2+(w - EMOJI_WIDTH)/2;
    oy = toy + 1 + h + (h - EMOJI_HEIGHT)/2;
  #endif
  
  if (page_selected==PAGE_TABS) {
    // Draw all tab icons
    #ifdef PBL_ROUND
      graphics_context_set_fill_color(ctx, GColorDarkGray);
      graphics_fill_radial(ctx, bounds, GOvalScaleModeFitCircle, 30, DEG_TO_TRIGANGLE(90-50), DEG_TO_TRIGANGLE(90+50));
      graphics_context_set_fill_color(ctx, GColorLightGray);
      graphics_fill_radial(ctx, bounds, GOvalScaleModeFitCircle, 30, DEG_TO_TRIGANGLE(90-50+20*tab_selected), DEG_TO_TRIGANGLE(90-50+20*(tab_selected+1)));
      for (uint8_t a=0; a<MAX_EMOJI_PAGES; a++) graphics_draw_bitmap_in_rect(ctx, round_tabs[a], POLAR_RECT(bounds,a));
    #else
      for (uint8_t a=0; a<MAX_EMOJI_PAGES; a++) graphics_draw_bitmap_in_rect(ctx, main_get_emoji(a,0,0,0), GRect(tox-1*(tab_selected==a), toy+th*a, EMOJI_WIDTH, EMOJI_HEIGHT));
    #endif
  } else if (page_selected==PAGE_EMOJIS) {
    #ifdef PBL_ROUND
      // draw grey box behind inactive tabs
      graphics_context_set_fill_color(ctx, GColorDarkGray);
      graphics_fill_radial(ctx, bounds, GOvalScaleModeFitCircle, 30, DEG_TO_TRIGANGLE(90-50), DEG_TO_TRIGANGLE(90+50));
      // Draw selected tab
      graphics_context_set_fill_color(ctx, GColorLightGray);
      graphics_fill_radial(ctx, bounds, GOvalScaleModeFitCircle, 30, DEG_TO_TRIGANGLE(90-50+20*tab_selected), DEG_TO_TRIGANGLE(90-50+20*(tab_selected+1)));
      graphics_draw_bitmap_in_rect(ctx, round_tabs[tab_selected], POLAR_RECT(bounds,tab_selected));
    #else
      // draw grey box behind inactive tabs
      graphics_context_set_fill_color(ctx,GColorLightGray);
      graphics_fill_rect(ctx, GRect(tox, 0, bounds.size.w, toy+th*tab_selected-1), 0, GCornerNone);
      graphics_fill_rect(ctx, GRect(tox, toy+th*tab_selected-1+EMOJI_HEIGHT+2, bounds.size.w, bounds.size.h), 0, GCornerNone);
      // Draw selected tab
      graphics_draw_bitmap_in_rect(ctx, main_get_emoji(tab_selected,0,0,0), GRect(tox-1, toy+th*tab_selected, EMOJI_WIDTH, EMOJI_HEIGHT));
    #endif
  }
  
  if (page_selected==PAGE_CHILD) {
    // draw childs stickers - grid = 5 wide x 4 high  
    char* stickers=jobs[child_index].Stickers;
    #ifdef PBL_ROUND
      w = EMOJI_WIDTH+1;
      h = EMOJI_HEIGHT-1;
      ox = (bounds.size.w - w*EMOJI_CHILD_COLS)/2;
      oy = (bounds.size.h - h*EMOJI_CHILD_ROWS)/2;
    #endif
    uint8_t emoji;
    uint8_t i=0;
    while (*stickers) {
      emoji = *stickers-1;
      graphics_draw_bitmap_in_rect(ctx, EMOJI_INDEX(emoji,0), GRect(ox+w*(i%EMOJI_CHILD_COLS), oy+h*(i/EMOJI_CHILD_COLS), EMOJI_WIDTH, EMOJI_HEIGHT));
      stickers++;
      i++;
    }
  } else {
    // draw emoji grid - grid = 4 wide x 5 high
    #ifndef PBL_ROUND
      w = bounds.size.w / (EMOJI_PAGE_COLS+1);
      h = bounds.size.h / EMOJI_PAGE_ROWS;
      ox = 2+(w - EMOJI_WIDTH)/2;
      oy = 2+(h - EMOJI_HEIGHT)/2;
    #endif 
    uint8_t i=0;
    for (uint8_t y=0; y<EMOJI_PAGE_ROWS; y++) {
      for (uint8_t x=0; x<EMOJI_PAGE_COLS; x++) {
        graphics_draw_bitmap_in_rect(ctx, main_get_emoji(tab_selected,x,y,0), GRect(PBL_IF_ROUND_ELSE(round_xy[i][0],ox+w*x), PBL_IF_ROUND_ELSE(round_xy[i][1],oy+h*y), EMOJI_WIDTH, EMOJI_HEIGHT));
        i++;
      }
    }
  }
  #ifndef PBL_ROUND
    if (page_selected!=PAGE_CHILD) {
      // Draw tab lines
      graphics_context_set_stroke_color(ctx,GColorBlack);
      // rounded square around selected tab
      graphics_draw_round_rect(ctx, GRect(tox-2, toy+th*tab_selected-2, EMOJI_WIDTH+3, EMOJI_HEIGHT+4),3); // outer box
      graphics_draw_round_rect(ctx, GRect(tox-2, toy+th*tab_selected-1, EMOJI_WIDTH+2, EMOJI_HEIGHT+2),2); // inner box
      // remove left edge
      graphics_context_set_stroke_color(ctx,GColorWhite);
      graphics_draw_rect(ctx,       GRect(tox-2, toy+th*tab_selected-1, 1,             EMOJI_HEIGHT+2));
      graphics_context_set_stroke_color(ctx,GColorBlack);
      // vertical black line between tabs and grid
      graphics_draw_rect(ctx,       GRect(tox-2, 0,                     2,             toy+th*tab_selected-1));
      graphics_draw_rect(ctx,       GRect(tox-2, toy+th*tab_selected-1+EMOJI_HEIGHT+2, 2, bounds.size.h));
    }
  #endif
  if (page_selected==PAGE_EMOJIS) {
    // highlight selected emoji
    #ifndef PBL_ROUND
    uint8_t x = emoji_selected % EMOJI_PAGE_COLS;
    uint8_t y = emoji_selected / EMOJI_PAGE_COLS;
    #endif
    graphics_context_set_stroke_color(ctx,GColorBlack);
    graphics_draw_round_rect(ctx, GRect(PBL_IF_ROUND_ELSE(round_xy[emoji_selected][0],ox+w*x)-2, PBL_IF_ROUND_ELSE(round_xy[emoji_selected][1],oy+h*y)-1, EMOJI_WIDTH+4, EMOJI_HEIGHT+2),3);     // outer square
    graphics_draw_round_rect(ctx, GRect(PBL_IF_ROUND_ELSE(round_xy[emoji_selected][0],ox+w*x)-1, PBL_IF_ROUND_ELSE(round_xy[emoji_selected][1],oy+h*y)  , EMOJI_WIDTH+2, EMOJI_HEIGHT  ),2); // inner square
  }  
}

static void select_click_handler(void) {
  if (page_selected==PAGE_EMOJIS) {
    if (jobs_add_sticker(child_index, tab_selected * EMOJI_PAGE_EMOJIS + emoji_selected) % EMOJI_PAGE_ROWS == 0) {
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
  
  #ifdef PBL_ROUND
  uint8_t a=0;
  uint8_t w=EMOJI_WIDTH - 1;
  uint8_t h=EMOJI_HEIGHT-2;
  uint8_t ox=EMOJI_WIDTH-2;
  uint8_t oy=(bounds.size.h - EMOJI_PAGE_ROWS*h)/2;
  for (uint8_t y=0; y<EMOJI_PAGE_ROWS; y++) {
    for (uint8_t x=0; x<EMOJI_PAGE_COLS; x++) {
      if ( x==0 && (y==0 || y==EMOJI_PAGE_ROWS-1) ) continue;
      round_xy[a][0]  = x*w + ox + (y==2?w/2:0);
      round_xy[a++][1]= y*h + oy;
      if (a==7 || a==12) { // shift these two behind rest and up down half
        round_xy[a][0]  = -w/2 + ox -w/4;
        round_xy[a][1]= y*h + oy + h/2 + (a==7?-1:+1)*h*0.15 ;
        a++;
      } 
    }
  }
  round_xy[3][1]-=h/2;
  round_xy[13][1]+=h/2;
  #endif
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

void emoji_menu_redraw(void) {
  if (s_window && window_is_loaded(s_window)) layer_mark_dirty(graphics_layer);
}