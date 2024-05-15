#include "threads.h"
#include "list.h"
#include "utils.h"
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

void print_data(ThreadInput *thread_input) {
  printf("t=%ld\tactive: readers: %d, writers: %d, removers: %d\n",
         time(NULL) - thread_input->start_time, *thread_input->active_readers,
         *thread_input->active_writers, *thread_input->active_removers);

  printf("list:\t");
  for (int i = 0; i < list_size(thread_input->list); i++) {
    printf("%d ", list_get(thread_input->list, i));
  }
  printf("\n\n");
}

void *writer_thread(void *args) {
  ThreadInput *thread_input = (ThreadInput *)args;

  while (!*thread_input->jobs_should_stop) {
    pthread_mutex_lock(thread_input->mutex);

    int v = rand_in_range(0, 100);

    printf("t=%ld\twriter %d wants to add value %d to list\n",
           time(NULL) - thread_input->start_time, thread_input->id, v);
    print_data(thread_input);

    *thread_input->waiting_writers += 1;

    while (*thread_input->active_writers + *thread_input->active_removers +
               *thread_input->waiting_removers >
           0) {
      pthread_cond_wait(thread_input->writers_queue, thread_input->mutex);
    }

    *thread_input->waiting_writers -= 1;
    *thread_input->active_writers += 1;

    printf("t=%ld\twriter %d start adding element %d to list\n",
           time(NULL) - thread_input->start_time, thread_input->id, v);
    print_data(thread_input);

    pthread_mutex_unlock(thread_input->mutex);

    sleep(rand_in_range(5, 10));

    pthread_mutex_lock(thread_input->mutex);

    list_append(thread_input->list, v);

    *thread_input->active_writers -= 1;

    printf("t=%ld\twriter %d added element %d to list\n",
           time(NULL) - thread_input->start_time, thread_input->id, v);
    print_data(thread_input);

    if (*thread_input->active_readers == 0 &&
        *thread_input->waiting_removers > 0) {
      pthread_cond_signal(thread_input->removers_queue);
    } else if (*thread_input->waiting_writers > 0) {
      pthread_cond_signal(thread_input->writers_queue);
    }

    pthread_mutex_unlock(thread_input->mutex);

    sleep(rand_in_range(5, 10));
  }

  free(thread_input);

  return NULL;
}

void *reader_thread(void *args) {
  ThreadInput *thread_input = (ThreadInput *)args;

  while (!*thread_input->jobs_should_stop) {
    size_t i = rand_in_range(0, list_size(thread_input->list));

    pthread_mutex_lock(thread_input->mutex);

    printf("t=%ld\treader %d wants to read element %ld of list\n",
           time(NULL) - thread_input->start_time, thread_input->id, i);
    print_data(thread_input);

    *thread_input->waiting_readers += 1;

    while (*thread_input->active_removers + *thread_input->waiting_removers >
           0) {
      pthread_cond_wait(thread_input->readers_queue, thread_input->mutex);
    }

    *thread_input->waiting_readers -= 1;
    *thread_input->active_readers += 1;

    int8_t v = list_get(thread_input->list, i);

    printf("t=%ld\treader %d read element %ld of list (value %d)\n",
           time(NULL) - thread_input->start_time, thread_input->id, i, v);
    print_data(thread_input);

    pthread_mutex_unlock(thread_input->mutex);

    sleep(rand_in_range(5, 10));

    pthread_mutex_lock(thread_input->mutex);

    *thread_input->active_readers -= 1;

    if (*thread_input->active_readers == 0 &&
        *thread_input->waiting_removers > 0) {
      pthread_cond_signal(thread_input->removers_queue);
    }

    printf("t=%ld\treader %d is not using list anymore\n",
           time(NULL) - thread_input->start_time, thread_input->id);
    print_data(thread_input);

    pthread_mutex_unlock(thread_input->mutex);

    sleep(rand_in_range(5, 10));
  }

  free(thread_input);

  return NULL;
}

void *remover_thread(void *args) {
  ThreadInput *thread_input = (ThreadInput *)args;

  while (!*thread_input->jobs_should_stop) {
    size_t i = rand_in_range(0, list_size(thread_input->list));

    pthread_mutex_lock(thread_input->mutex);

    printf("t=%ld\tremover %d wants to remove element %ld of list\n",
           time(NULL) - thread_input->start_time, thread_input->id, i);
    print_data(thread_input);

    *thread_input->waiting_removers += 1;

    while (*thread_input->active_readers + *thread_input->active_writers +
               *thread_input->active_removers >
           0) {
      pthread_cond_wait(thread_input->removers_queue, thread_input->mutex);
    }

    *thread_input->waiting_removers -= 1;
    *thread_input->active_removers += 1;

    printf("t=%ld\tremover %d start removing element %ld of list\n",
           time(NULL) - thread_input->start_time, thread_input->id, i);
    print_data(thread_input);

    pthread_mutex_unlock(thread_input->mutex);

    sleep(rand_in_range(5, 10));

    pthread_mutex_lock(thread_input->mutex);

    int v = list_remove(thread_input->list, i);

    printf("t=%ld\tremover %d removed element %ld of list (value %d)\n",
           time(NULL) - thread_input->start_time, thread_input->id, i, v);
    print_data(thread_input);

    *thread_input->active_removers -= 1;

    if (*thread_input->waiting_removers > 0) {
      pthread_cond_signal(thread_input->removers_queue);
    } else {
      pthread_cond_signal(thread_input->writers_queue);
      pthread_cond_broadcast(thread_input->readers_queue);
    }

    pthread_mutex_unlock(thread_input->mutex);

    sleep(rand_in_range(10, 20));
  }

  free(thread_input);

  return NULL;
}
