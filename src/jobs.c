#include "main.h"

#define STICKERS_SIZE MAX_STICKERS+1
typedef struct {
  char Name[JOB_NAME_SIZE];
  uint16_t Count;
  char Stickers[STICKERS_SIZE];
} Job;

typedef struct Job_ptr {
  Job* Job;
  struct Job_ptr* Next_ptr;
} Job_ptr ;

static Job_ptr* first_job_ptr=NULL;
uint8_t jobs_count=0;

// *****************************************************************************************************
// JOB LIST FUNCTIONS
// *****************************************************************************************************

Job* jobs_list_get_index(uint8_t index) {
  if (index>=jobs_count) return NULL;
  Job_ptr* job_ptr = first_job_ptr;
  while (index--) job_ptr=job_ptr->Next_ptr;
  return job_ptr->Job;
}

static void jobs_list_append_job(const char* name, uint16_t count, char* stickers) {
  Job* new_job = malloc(sizeof(Job));
  Job_ptr* new_job_ptr = malloc(sizeof(Job_ptr));
  
  new_job_ptr->Job = new_job;
  new_job_ptr->Next_ptr = NULL;
  
  strncpy(new_job->Name, name, JOB_NAME_SIZE)[JOB_NAME_SIZE-1]=0;
  new_job->Count = count;
  strncpy(new_job->Stickers, stickers ? stickers : "", STICKERS_SIZE)[STICKERS_SIZE-1]=0;
  
  if (first_job_ptr) {
    Job_ptr* last_job_ptr = first_job_ptr;
    while (last_job_ptr->Next_ptr) last_job_ptr=last_job_ptr->Next_ptr;
    last_job_ptr->Next_ptr = new_job_ptr;
  } else {
    first_job_ptr = new_job_ptr;
  }
  LOG("appended job: %s, count=%d", new_job->Name, new_job->Count);
  jobs_count++;
}

void jobs_list_save(uint8_t first_key) {
  Job_ptr* job_ptr = first_job_ptr;
  while (job_ptr) {
    persist_write_data(first_key++, job_ptr->Job, sizeof(Job));
    job_ptr=job_ptr->Next_ptr;
  }
  // if we've delete a job then need to delete the saved version or it will come back!
  persist_delete(first_key);
}

void jobs_list_write_dict(DictionaryIterator *iter, uint8_t first_key) {
  Job_ptr* job_ptr = first_job_ptr;
  uint8_t result;
  while (job_ptr) {
    result=dict_write_data(iter,first_key++, (void*) job_ptr->Job, sizeof(Job));
    LOG("job %d, result %d, not enough storate=%d", (int) first_key, (int) result, (int) DICT_NOT_ENOUGH_STORAGE);
    job_ptr=job_ptr->Next_ptr;
  }
}

void jobs_list_read_dict(DictionaryIterator *iter, uint8_t first_key, const uint8_t version) {
  if (first_job_ptr!=NULL) return;
  
  Tuple *tuple_t;
  Job* job=malloc(sizeof(Job));
  while ((tuple_t=dict_find(iter, first_key++))) {
    memcpy((void*) job, (void*) tuple_t->value->data, tuple_t->length);
    jobs_list_append_job(job->Name, job->Count, job->Stickers);
  }
  free(job);
}

void jobs_list_load2(uint8_t first_key, const uint8_t version) {
  ERROR("Loading fake data");
  LOG("size of job: %d", (int) sizeof(Job));
  
  jobs_list_append_job("Millie",1, "\x13");
  jobs_list_append_job("Penny",0, NULL);
}

void jobs_list_load(uint8_t first_key, const uint8_t version) {
  Job* new_job;
  Job_ptr* new_job_ptr;
  Job_ptr* prev_job_ptr=NULL;
  LOG("key=%d, exists=%d", first_key, persist_exists(first_key));
  while (persist_exists(first_key)) {
    new_job = malloc(sizeof(Job));
    persist_read_data(first_key, new_job, sizeof(Job));
    
    LOG("loaded job: %s, count=%d", new_job->Name, new_job->Count);
    new_job_ptr = malloc(sizeof(Job_ptr));
    new_job_ptr->Job = new_job;
    new_job_ptr->Next_ptr = NULL;
    if (prev_job_ptr) prev_job_ptr->Next_ptr = new_job_ptr;
    prev_job_ptr = new_job_ptr;
    if (NULL==first_job_ptr) first_job_ptr = new_job_ptr;
    jobs_count++;
    first_key++;
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
    strncpy(jobs_list_get_index(index)->Name, result, JOB_NAME_SIZE)[JOB_NAME_SIZE-1]=0;
  }
  main_menu_highlight_job(jobs_count-1);
  main_save_data(0);
  main_menu_update();
}

void jobs_add_job(void) {
  tertiary_text_prompt("New child name?", callback, (void*) -1);
}

void jobs_rename_job(uint8_t index) {
  tertiary_text_prompt(jobs_get_job_name(index), callback, (void*) (int) index);
}

void jobs_delete_all_jobs(void) {
  while (first_job_ptr) {
    Job_ptr * next_job=first_job_ptr->Next_ptr;
    free(first_job_ptr->Job);
    free(first_job_ptr);
    first_job_ptr=next_job;
  }
  jobs_count=0;
}

void jobs_delete_job_and_save(uint8_t index) {
  if (index>=jobs_count) return;
  
  Job_ptr* job_ptr = first_job_ptr;
  
  if (index) {
    Job_ptr* prev_job_ptr = NULL;
    while (index--) {
      prev_job_ptr=job_ptr;
      job_ptr=job_ptr->Next_ptr;
    }
    prev_job_ptr->Next_ptr = job_ptr->Next_ptr;
  } else {
    first_job_ptr = job_ptr->Next_ptr;
  }
  free(job_ptr->Job);
  free(job_ptr);
  
  jobs_count--;
  main_save_data(0);
  main_menu_update();
}

char* jobs_get_job_name(uint8_t index) {
  Job* job=jobs_list_get_index(index);
  return (job) ? job->Name : NULL;
}

char* jobs_get_job_stickers(uint8_t index) {
  return jobs_list_get_index(index)->Stickers;
}

uint8_t jobs_add_sticker(uint8_t index, uint8_t sticker) {
  Job* job=jobs_list_get_index(index);
  char* stickers = job->Stickers;
  uint8_t n=0;
  while (stickers[n]) n++;
  if (n == MAX_STICKERS) {
    LOG("shifting stickers");
    for (n=EMOJI_PAGE_COLS; n < MAX_STICKERS; n++) {
      stickers[n]=stickers[n-EMOJI_PAGE_COLS];
    }
    for (n=MAX_STICKERS-EMOJI_PAGE_COLS; n<MAX_STICKERS; n++) stickers[n]=0;
    n=MAX_STICKERS-EMOJI_PAGE_COLS;
  }
  stickers[n] = sticker+1;
  job->Count++;
  main_save_data(0);
  return job->Count;
}

bool jobs_delete_sticker(uint8_t index) {
  Job* job=jobs_list_get_index(index);
  if (job->Count == 0) return false;
  char* stickers = job->Stickers;
  while (*stickers) stickers++;
  *--stickers='\0';
  job->Count--;
  main_save_data(0);
  return true;
}

#define MAX_COUNT_LENGTH 5
char count_buffer[MAX_COUNT_LENGTH];

char* jobs_get_job_count_as_text(uint8_t index) {
  Job* job=jobs_list_get_index(index);
  snprintf(count_buffer,MAX_COUNT_LENGTH,"%d", job->Count);
  return count_buffer;
}

uint8_t jobs_get_job_count(uint8_t index) {
  return jobs_list_get_index(index)->Count;
}