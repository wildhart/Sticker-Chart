#include "main.h"

struct Job jobs[MAX_JOBS];
uint8_t jobs_count=0;

// *****************************************************************************************************
// JOB LIST FUNCTIONS
// *****************************************************************************************************

static void jobs_list_append_job(const char* name, uint16_t count, char* stickers) {
  if (jobs_count==MAX_JOBS) return;
  struct Job* new_job=&jobs[jobs_count++];
  strncpy(new_job->Name, name, JOB_NAME_SIZE)[JOB_NAME_SIZE-1]=0;
  new_job->Count = count;
  strncpy(new_job->Stickers, stickers ? stickers : "", STICKERS_SIZE)[STICKERS_SIZE-1]=0; // zero pads array always with trailing zero
  LOG("appended job: %s, count=%d", jobs[jobs_count-1].Name, new_job->Count);
}

void jobs_list_save(uint8_t first_key) {
  for (uint8_t a=0; a<jobs_count; a++) persist_write_data(first_key++, &jobs[a], sizeof(jobs[0]));
  // if we've delete a job then need to delete the saved version or it will come back!
  persist_delete(first_key);
}

void jobs_list_write_dict(DictionaryIterator *iter, uint8_t first_key) {
  uint8_t result;
  for (uint8_t a=0; a<jobs_count; a++) {
    result=dict_write_data(iter, first_key++, (void*) &jobs[a], sizeof(jobs[0]));
    LOG("job %d, result %d, not enough storate=%d", (int) first_key, (int) result, (int) DICT_NOT_ENOUGH_STORAGE);
  }
}

void jobs_list_read_dict(DictionaryIterator *iter, uint8_t first_key, const uint8_t version) {
  if (jobs_count) return;
  Tuple *tuple_t;
  while ((tuple_t=dict_find(iter, first_key++))) {
    memcpy((void*) &jobs[jobs_count++], (void*) tuple_t->value->data, tuple_t->length);
    LOG("read job: %s, count=%d", jobs[jobs_count-1].Name, jobs[jobs_count-1].Count);
  }
}

void jobs_list_load2(uint8_t first_key, const uint8_t version) {
  ERROR("Loading fake data");
  LOG("size of job: %d", (int) sizeof(jobs[0]));
  
  jobs_list_append_job("Millie",4, "\x1\x2\x3\x4");
  jobs_list_append_job("Penny",0, NULL);
}

void jobs_list_load(uint8_t first_key, const uint8_t version) {
  LOG("key=%d, exists=%d", first_key, persist_exists(first_key));
  while (persist_exists(first_key)) {
    struct Job* new_job=&jobs[jobs_count++];
    persist_read_data(first_key++, new_job, sizeof(jobs[0]));
    LOG("loaded job: %s, count=%d", new_job->Name, new_job->Count);
  }
  LOG("Loaded %d jobs.",jobs_count);
}

// *****************************************************************************************************
// PUBLIC FUNCTIONS
// *****************************************************************************************************

static void callback(const char* result, size_t result_length, void* extra) {
	// Do something with result
  int index = (int) extra;
  if (index==-1) {
    jobs_list_append_job(result, 0, NULL);
  } else {
    strncpy(jobs[index].Name, result, JOB_NAME_SIZE)[JOB_NAME_SIZE-1]=0;
  }
  main_menu_highlight_job(jobs_count-1);
  main_save_data(0);
  main_menu_update();
}

void jobs_add_job(void) {
  tertiary_text_prompt("New child name?", callback, (void*) -1);
}

void jobs_rename_job(uint8_t index) {
  tertiary_text_prompt(jobs[index].Name, callback, (void*) (int) index);
}

void jobs_delete_all_jobs(void) {
  jobs_count=0;
}

void jobs_delete_job_and_save(uint8_t index) {  
  for (uint8_t a=index; a<jobs_count-1; a++) {
    memcpy((void*) &jobs[a], (void*) &jobs[a+1], sizeof(jobs[0]));
  }
  
  jobs_count--;
  main_save_data(0);
  main_menu_update();
}

uint8_t jobs_add_sticker(uint8_t index, uint8_t sticker) {
  char* stickers = jobs[index].Stickers;
  uint8_t n=0;
  while (stickers[n]) n++;
  if (n == MAX_STICKERS) {
    LOG("shifting stickers");
    for (n=EMOJI_CHILD_COLS; n < MAX_STICKERS; n++) {
      stickers[n-EMOJI_CHILD_COLS]=stickers[n];
    }
    for (n=MAX_STICKERS-EMOJI_CHILD_COLS; n<MAX_STICKERS; n++) stickers[n]=0;
    n=MAX_STICKERS-EMOJI_CHILD_COLS;
  }
  stickers[n] = sticker+1;
  jobs[index].Count++;
  main_save_data(0);
  return jobs[index].Count;
}

bool jobs_delete_sticker(uint8_t index) {
  if (jobs[index].Count == 0) return false;
  char* stickers = jobs[index].Stickers;
  while (*stickers) stickers++;
  *--stickers='\0';
  jobs[index].Count--;
  main_save_data(0);
  return true;
}

#define MAX_COUNT_LENGTH 5
char count_buffer[MAX_COUNT_LENGTH];

char* jobs_get_job_count_as_text(uint8_t index) {
  snprintf(count_buffer,MAX_COUNT_LENGTH,"%d", jobs[index].Count);
  return count_buffer;
}