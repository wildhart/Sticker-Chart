#include "main.h"

typedef struct {
  char Name[JOB_NAME_LENGTH];
  uint16_t Count;
  char Stickers[MAX_STICKERS];
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

static void jobs_list_append_job(const char* name, uint16_t count, char* stickers) {
  Job* new_job = malloc(sizeof(Job));
  Job_ptr* new_job_ptr = malloc(sizeof(Job_ptr));
  
  new_job_ptr->Job = new_job;
  new_job_ptr->Next_ptr = NULL;
  strncpy(new_job->Name, name, JOB_NAME_LENGTH);
  new_job->Name[JOB_NAME_LENGTH-1]=0;
  new_job->Count = count;
  for (uint8_t i=0; i<MAX_STICKERS; i++) new_job->Stickers[i]=0;
  if (stickers) strncpy(new_job->Stickers, stickers, MAX_STICKERS);
    
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
  Job * job;
  char buffer[JOB_NAME_LENGTH+MAX_STICKERS+30];
  while (job_ptr) {
    job=job_ptr->Job;
    snprintf(buffer,JOB_NAME_LENGTH+MAX_STICKERS+30,"%s|%d|%s",job->Name, job->Count, job->Stickers);
    dict_write_cstring(iter, first_key++, buffer);
    job_ptr=job_ptr->Next_ptr;
  }
}

void jobs_list_read_dict(DictionaryIterator *iter, uint8_t first_key, const uint8_t version) {
  if (first_job_ptr!=NULL) return;
  
  Tuple *tuple_t;
  uint8_t fields=3;
  char buffer[fields][JOB_NAME_LENGTH+MAX_STICKERS];
  
  while ((tuple_t=dict_find(iter, first_key++))) {
    char *source = tuple_t->value->cstring;
    for (int c=0; c<fields; c++) {
      uint d=0; // destination offset
      while (*source && *source!='|' && d<JOB_NAME_LENGTH+MAX_STICKERS) {
        buffer[c][d++]=*source++;
      }
      while (*source && *source!='|') source++;
      buffer[c][d]=0;
      source++;
    }
    jobs_list_append_job(buffer[0], atoi(buffer[1]), buffer[2]);
  }
}

void jobs_list_load2(int8_t first_key, const uint8_t version) {
  ERROR("Loading fake data");
  jobs_list_append_job("Millie",19,"ABCDEFGHIJKLMNOPQRS");
  jobs_list_append_job("Penny",0,"");
}

void jobs_list_load(uint8_t first_key, const uint8_t version) {
  Job* new_job;
  Job_ptr* new_job_ptr;
  Job_ptr* prev_job_ptr=NULL;
  LOG("key=%d, exists=%d", first_key, persist_exists(first_key));
  while (persist_exists(first_key)) {
    new_job = malloc(sizeof(Job));
    persist_read_data(first_key, new_job, sizeof(Job));
    
    LOG("loaded job: %s, count=%d stickers=%s", new_job->Name, new_job->Count, new_job->Stickers);
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

Job* jobs_list_get_index(uint8_t index) {
  if (index>=jobs_count) return NULL;
  Job_ptr* job_ptr = first_job_ptr;
  while (index--) job_ptr=job_ptr->Next_ptr;
  return job_ptr->Job;
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
    snprintf(jobs_list_get_index(index)->Name,JOB_NAME_LENGTH, result);
  }
  main_menu_highlight_job(jobs_count-1);
  main_save_data();
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
  main_save_data();
  main_menu_update();
}

char* jobs_get_job_name(uint8_t index) {
  Job* job=jobs_list_get_index(index);
  return (job) ? job->Name : NULL;
}

char* jobs_get_job_stickers(uint8_t index) {
  Job* job=jobs_list_get_index(index);
  return (job) ? job->Stickers : NULL;
}

uint8_t jobs_add_sticker(uint8_t index, uint8_t sticker) {
  Job* job=jobs_list_get_index(index);
  char* stickers = job->Stickers;
  while (*stickers) stickers++;
  if (stickers - job->Stickers == MAX_STICKERS) {
    LOG("shifting stickers");
    for (stickers=job->Stickers; stickers-job->Stickers < MAX_STICKERS-EMOJI_PAGE_COLS; stickers++) {
      *stickers=*(stickers+EMOJI_PAGE_COLS);
    }
    for (uint8_t a=0; a<EMOJI_PAGE_COLS; a++) *(stickers+a)='\0';
  }
  *stickers = sticker + FIRST_ASCII;
  job->Count++;
  main_save_data();
  return job->Count;
}

bool jobs_delete_sticker(uint8_t index) {
  Job* job=jobs_list_get_index(index);
  if (job->Count == 0) return false;
  char* stickers = job->Stickers;
  while (*stickers) stickers++;
  *--stickers='\0';
  job->Count--;
  main_save_data();
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