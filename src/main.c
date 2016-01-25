#include "main.h"

#include "pebble_process_info.h"
extern const PebbleProcessInfo __pbl_app_info;
#define APP_VERSION_LENGTH 10
char app_version[APP_VERSION_LENGTH];

GBitmap *bitmap_matrix;
//GBitmap *bitmap_pause;
//GBitmap *bitmap_play;
GBitmap *bitmap_add;
GBitmap *bitmap_settings;
GBitmap *bitmap_delete;
GBitmap *bitmap_edit;
//GBitmap *bitmap_adjust;
//GBitmap *bitmap_reset;
GBitmap *bitmap_minus;
//GBitmap *bitmap_tick;

static bool JS_ready = false;
static bool data_loaded_from_watch = false;
static uint32_t data_timestamp = 0;
uint8_t stored_version=0;
bool export_after_save=false;

// *****************************************************************************************************
// MESSAGES
// *****************************************************************************************************

#define KEY_CHILDREN      100
#define KEY_CONFIG_DATA   0
#define KEY_APP_VERSION   1
#define KEY_VERSION       2
#define KEY_EXPORT        3
#define KEY_TIMESTAMP     4

static void send_settings_to_phone() {
  if (!JS_ready) return;
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  int dummy_int;
  
  dict_write_cstring(iter, KEY_APP_VERSION, app_version);
  dummy_int=CURRENT_STORAGE_VERSION;   dict_write_int(iter, KEY_VERSION, &dummy_int, sizeof(int), true);
  dummy_int=data_timestamp;          dict_write_int(iter, KEY_TIMESTAMP, &dummy_int, sizeof(int), true);

  if (export_after_save) {
    dummy_int=true;
    dict_write_int(iter, KEY_EXPORT, &dummy_int, sizeof(int), true);
    export_after_save=false;
  }
  
  jobs_list_write_dict(iter, KEY_CHILDREN);

  dict_write_end(iter);
  LOG("ended, dict_size=%d", (int) dict_size(iter));
  app_message_outbox_send();
}

static void inbox_received_handler(DictionaryIterator *iter, void *context) {
  LOG("Inbox received...");
  
  JS_ready = true;
  Tuple *tuple_t;
  
  ERROR("Ignoring phone data!"); return;

  bool new_data_from_config_page = dict_find(iter, KEY_CONFIG_DATA);
  tuple_t= dict_find(iter, KEY_TIMESTAMP);
  uint32_t inbox_timestamp = tuple_t ? tuple_t->value->uint32 : 0;
  
  LOG("inbox timestamp=%d, data timestamp=%d, new data from config page=%d",(int) inbox_timestamp, (int) data_timestamp, new_data_from_config_page);
  if (new_data_from_config_page || inbox_timestamp > data_timestamp)  {
    LOG("Loading settings from phone...");
    data_timestamp=inbox_timestamp;
    tuple_t=dict_find(iter, KEY_VERSION);  stored_version = (tuple_t) ? tuple_t->value->int32 : 1;
    jobs_delete_all_jobs();
    jobs_list_read_dict(iter, KEY_CHILDREN, stored_version);

    if (stored_version < CURRENT_STORAGE_VERSION) {
      //update_show(stored_version);  
      stored_version = CURRENT_STORAGE_VERSION;
    }
    main_save_data(data_timestamp);
    main_menu_highlight_job(0);
  } else if (inbox_timestamp < data_timestamp) {
    send_settings_to_phone();
  }
  
  LOG("Inbox processed.");
}

// *****************************************************************************************************
// DATA STORAGE
// *****************************************************************************************************

void main_save_data(const uint32_t timestamp) {
  data_loaded_from_watch = true;
  persist_write_int(STORAGE_KEY_VERSION, CURRENT_STORAGE_VERSION);
  if (timestamp!=1) data_timestamp=timestamp ?  timestamp : (uint32_t) time(NULL);
  persist_write_int(STORAGE_KEY_TIMESTAMP, data_timestamp);
  jobs_list_save(STORAGE_KEY_FIRST_CHILD);
  send_settings_to_phone();
}

static void main_load_data(void) {
  //ERROR("ignoring saved data"); return;
  
  stored_version = persist_read_int(STORAGE_KEY_VERSION); // defaults to 0 if key is missing
  
  if (stored_version) {
    data_loaded_from_watch = true;
    if (persist_exists(STORAGE_KEY_TIMESTAMP)) data_timestamp=persist_read_int(STORAGE_KEY_TIMESTAMP);
    jobs_list_load(STORAGE_KEY_FIRST_CHILD, stored_version);
    if (stored_version < CURRENT_STORAGE_VERSION)  {
      LOG("Saving data in new version");
      main_save_data(0);
    }
  } else {
    //ERROR("Loading fake data"); jobs_list_load(STORAGE_KEY_FIRST_CHILD, stored_version);
  }
}

// *****************************************************************************************************
// MAIN
// *****************************************************************************************************

//#ifndef gbitmap_set_bounds
//#define gbitmap_set_bounds(bmp, new_bounds) ((bmp)->bounds = (new_bounds))
//#endif

static uint8_t last_matrix=99;
static uint32_t emoji_images[MAX_EMOJI_PAGES]={RESOURCE_ID_IMAGE_EMOJI_C1,RESOURCE_ID_IMAGE_EMOJI_C2,RESOURCE_ID_IMAGE_EMOJI_C3,RESOURCE_ID_IMAGE_EMOJI_C4,RESOURCE_ID_IMAGE_EMOJI_C5};
static GBitmap *emoji_buffer=NULL;

GBitmap* main_get_emoji(const uint8_t page, const uint8_t x, const uint8_t y) {
  if (page != last_matrix) {
    //LOG("%d",page);
    //HEAP("about to load bitmap...");
    if (emoji_buffer) gbitmap_destroy(emoji_buffer);
    last_matrix=page;
    emoji_buffer=gbitmap_create_with_resource(emoji_images[page]);
    //HEAP("loaded bitmap");
  }
  gbitmap_set_bounds(emoji_buffer, GRect(EMOJI_WIDTH*x,EMOJI_HEIGHT*y,EMOJI_WIDTH,EMOJI_HEIGHT));
  return emoji_buffer;
}

void init(void) {
  snprintf(app_version,APP_VERSION_LENGTH,"%d.%d",__pbl_app_info.process_version.major, __pbl_app_info.process_version.minor);
  
  main_load_data();
  
  bitmap_matrix=gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ICON_MATRIX);
  //bitmap_pause=gbitmap_create_as_sub_bitmap(bitmap_matrix, ICON_RECT_PAUSE);
  //bitmap_play=gbitmap_create_as_sub_bitmap(bitmap_matrix, ICON_RECT_PLAY);
  bitmap_add=gbitmap_create_as_sub_bitmap(bitmap_matrix, ICON_RECT_ADD);
  bitmap_settings=gbitmap_create_as_sub_bitmap(bitmap_matrix, ICON_RECT_SETTINGS);
  bitmap_delete=gbitmap_create_as_sub_bitmap(bitmap_matrix, ICON_RECT_DELETE);
  bitmap_edit=gbitmap_create_as_sub_bitmap(bitmap_matrix, ICON_RECT_EDIT);
  //bitmap_adjust=gbitmap_create_as_sub_bitmap(bitmap_matrix, ICON_RECT_ADJUST);
  //bitmap_reset=gbitmap_create_as_sub_bitmap(bitmap_matrix, ICON_RECT_RESET);
  bitmap_minus=gbitmap_create_as_sub_bitmap(bitmap_matrix, ICON_RECT_MINUS);
  //bitmap_tick=gbitmap_create_as_sub_bitmap(bitmap_matrix, ICON_RECT_TICK);
  
  main_menu_show();

  app_message_register_inbox_received(inbox_received_handler);
  app_message_open(1024, 1024); // should be enough for 20 children (maybe 21).  34 bytes for settings + 47/child
}

void deinit(void) {
  main_menu_hide();
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
