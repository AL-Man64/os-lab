#ifndef INCLUDE_MONITORI_THREADS_H_
#define INCLUDE_MONITORI_THREADS_H_

#include "list.h"
#include <pthread.h>
#include <stdbool.h>
#include <time.h>

typedef struct {
  int id;
  bool *jobs_should_stop;
  time_t start_time;

  List list;

  int *active_writers;
  int *active_readers;
  int *active_removers;

  int *waiting_writers;
  int *waiting_readers;
  int *waiting_removers;

  pthread_mutex_t *mutex;

  pthread_cond_t *writers_queue;
  pthread_cond_t *readers_queue;
  pthread_cond_t *removers_queue;
} ThreadInput;

void *writer_thread(void *args);

void *reader_thread(void *args);

void *remover_thread(void *args);

#endif // INCLUDE_MONITORI_THREADS_H_
