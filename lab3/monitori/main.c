#include "list.h"
#include "threads.h"
#include <bits/pthreadtypes.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define WRITER_THREADS 4
#define READER_THREADS 10
#define REMOVER_THREADS 2

bool JOBS_SHOULD_STOP = false;
bool GRACEFUL_SHUTDOWN = true;

void sigint_handler(int _) {
  JOBS_SHOULD_STOP = true;

  if (!GRACEFUL_SHUTDOWN) {
    fprintf(stderr, "shutting down forcefully\n");
    exit(EXIT_FAILURE);
  }

  fprintf(stderr, "shutting down gracefully\n");
  GRACEFUL_SHUTDOWN = false;
}

int main(void) {
  srand(time(NULL));

  struct sigaction sa;

  sa.sa_handler = sigint_handler;

  sigaction(SIGINT, &sa, NULL);

  List list = new_list();

  pthread_t writer_threads[WRITER_THREADS];
  pthread_t reader_threads[READER_THREADS];
  pthread_t remover_threads[REMOVER_THREADS];

  int active_writers = 0;
  int active_readers = 0;
  int active_removers = 0;

  int waiting_writers = 0;
  int waiting_readers = 0;
  int waiting_removers = 0;

  pthread_mutex_t mutex;
  pthread_mutex_init(&mutex, NULL);

  pthread_cond_t writers_queue;
  pthread_cond_t readers_queue;
  pthread_cond_t removers_queue;

  pthread_cond_init(&writers_queue, NULL);
  pthread_cond_init(&readers_queue, NULL);
  pthread_cond_init(&removers_queue, NULL);

  time_t start_time = time(NULL);

  ThreadInput thread_input;

  thread_input.jobs_should_stop = &JOBS_SHOULD_STOP;
  thread_input.start_time = start_time;

  thread_input.list = list;

  thread_input.active_writers = &active_writers;
  thread_input.active_readers = &active_readers;
  thread_input.active_removers = &active_removers;

  thread_input.waiting_writers = &waiting_writers;
  thread_input.waiting_readers = &waiting_readers;
  thread_input.waiting_removers = &waiting_removers;

  thread_input.mutex = &mutex;

  thread_input.writers_queue = &writers_queue;
  thread_input.readers_queue = &readers_queue;
  thread_input.removers_queue = &removers_queue;

  for (int i = 0; i < WRITER_THREADS; i++) {
    ThreadInput *curr_thread_input = (ThreadInput *)malloc(sizeof(ThreadInput));

    *curr_thread_input = thread_input;
    curr_thread_input->id = i;

    pthread_create(&writer_threads[i], NULL, writer_thread, curr_thread_input);
  }

  sleep(10);

  for (int i = 0; i < READER_THREADS; i++) {
    ThreadInput *curr_thread_input = (ThreadInput *)malloc(sizeof(ThreadInput));

    *curr_thread_input = thread_input;
    curr_thread_input->id = i;

    pthread_create(&reader_threads[i], NULL, reader_thread, curr_thread_input);
  }

  sleep(10);

  for (int i = 0; i < REMOVER_THREADS; i++) {
    ThreadInput *curr_thread_input = (ThreadInput *)malloc(sizeof(ThreadInput));

    *curr_thread_input = thread_input;
    curr_thread_input->id = i;

    pthread_create(&remover_threads[i], NULL, remover_thread,
                   curr_thread_input);
  }

  while (!JOBS_SHOULD_STOP) {
    sleep(1);
  }

  for (int i = 0; i < WRITER_THREADS; i++) {
    pthread_join(writer_threads[i], NULL);
  }
  for (int i = 0; i < READER_THREADS; i++) {
    pthread_join(reader_threads[i], NULL);
  }
  for (int i = 0; i < REMOVER_THREADS; i++) {
    pthread_join(remover_threads[i], NULL);
  }

  pthread_mutex_destroy(&mutex);

  list_delete(list);

  return EXIT_SUCCESS;
}
