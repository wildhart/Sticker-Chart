#include "main.h"

#define JOB_NAME_EXCLAIMED_LENGTH JOB_NAME_SIZE+4
static char child_name_exclaimed[JOB_NAME_EXCLAIMED_LENGTH]; // include space for " !!!"

// BEGIN AUTO-GENERATED UI CODE; DO NOT MODIFY
static Window *s_window;
static GFont s_res_gothic_28_bold;
static GFont s_res_roboto_condensed_21;
static TextLayer *s_textlayer_title;
static TextLayer *s_textlayer_name;
static TextLayer *s_textlayer_msg;

static void initialise_ui(void) {
  s_window = window_create();
  #ifndef PBL_SDK_3
    window_set_fullscreen(s_window, true);
  #endif
  
  s_res_gothic_28_bold = fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD);
  s_res_roboto_condensed_21 = fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21);
  // s_textlayer_title
  s_textlayer_title = text_layer_create(GRect(0, 0, 144, 30));
  text_layer_set_text(s_textlayer_title, "Well Done");
  text_layer_set_text_alignment(s_textlayer_title, GTextAlignmentCenter);
  text_layer_set_font(s_textlayer_title, s_res_gothic_28_bold);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_textlayer_title);
  
  // s_textlayer_name
  s_textlayer_name = text_layer_create(GRect(0, 30, 144, 30));
  text_layer_set_text(s_textlayer_name, child_name_exclaimed);
  text_layer_set_text_alignment(s_textlayer_name, GTextAlignmentCenter);
  text_layer_set_font(s_textlayer_name, s_res_gothic_28_bold);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_textlayer_name);
  
  // s_textlayer_msg
  s_textlayer_msg = text_layer_create(GRect(10, 68, 121, 100));
  text_layer_set_text(s_textlayer_msg, "Another row is complete!\nYou deserve\na treat !!");
  text_layer_set_text_alignment(s_textlayer_msg, GTextAlignmentCenter);
  text_layer_set_font(s_textlayer_msg, s_res_roboto_condensed_21);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_textlayer_msg);
}

static void destroy_ui(void) {
  window_destroy(s_window);
  text_layer_destroy(s_textlayer_title);
  text_layer_destroy(s_textlayer_name);
  text_layer_destroy(s_textlayer_msg);
}
// END AUTO-GENERATED UI CODE

static void handle_window_unload(Window* window) {
  destroy_ui();
}

static void vibrate(void) {
  // Vibe pattern: ON for 200ms, OFF for 100ms, ON for 400ms:
  LOG("VIBRATE!!!");
  static const uint32_t segments[] = { 400,200, 400,200, 400,200, 400,200, 400 };
  VibePattern pat = {
    .durations = segments,
    .num_segments = ARRAY_LENGTH(segments),
  };
  vibes_enqueue_custom_pattern(pat);
}

static void select_click_handler(void) {
  treat_window_hide();
}

static void click_config_provider(void) {
  window_single_click_subscribe(BUTTON_ID_SELECT, (ClickHandler) select_click_handler);
}

void treat_window_show(char *name) {
  snprintf(child_name_exclaimed,JOB_NAME_EXCLAIMED_LENGTH,"%s!!!",name);
  initialise_ui();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .unload = handle_window_unload,
  });
  window_set_click_config_provider(s_window, (ClickConfigProvider) click_config_provider);
  window_stack_push(s_window, true);
  vibrate();
}

void treat_window_hide(void) {
  window_stack_remove(s_window, true);
}
