#pragma once

#define MAX_JOBS 20
#define JOB_NAME_SIZE 15 
#define MAX_STICKERS 20

#define STICKERS_SIZE MAX_STICKERS+1
struct Job {
  char Name[JOB_NAME_SIZE];
  uint16_t Count;
  char Stickers[STICKERS_SIZE];
};

extern struct Job jobs[MAX_JOBS];
extern uint8_t jobs_count;

void jobs_list_sort(void);
void jobs_list_save(uint8_t first_key);
void jobs_list_load(uint8_t first_key, const uint8_t version);
void jobs_list_write_dict(DictionaryIterator *iter, uint8_t first_key);
void jobs_list_read_dict(DictionaryIterator *iter, uint8_t first_key, const uint8_t version);

void jobs_delete_all_jobs(void);
void jobs_delete_job_and_save(uint8_t index);
void jobs_add_job(void);
void jobs_rename_job(uint8_t index);
uint8_t jobs_add_sticker(uint8_t index, uint8_t sticker);
bool jobs_delete_sticker(uint8_t index);
char* jobs_get_job_count_as_text(uint8_t index);