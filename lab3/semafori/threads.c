#include "threads.h"
#include "buffers.h"
#include "utils.h"
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

sem_t sem_stdout;

// Amount of threads
int input_threads_size = 0;
int work_threads_size = 0;
int output_threads_size = 0;

// Thread handles
pthread_t *input_threads = 0;
pthread_t *work_threads = 0;
pthread_t *output_threads = 0;

int start_time = 0;

bool threads_should_join = false;

void print_buffers() {
  const char **input_buffers = get_input_buffers();
  const char **output_buffers = get_output_buffers();

  printf("t=%d\tinput_buffers[]: ", (int)difftime(time(NULL), start_time));
  for (int i = 0; i < get_input_buffers_size(); i++) {
    for (int j = 0; j < get_buffer_size(); j++) {
      printf("%c", input_buffers[i][j]);
    }
    printf(" ");
  }
  printf("\n");

  printf("t=%d\toutput_buffers[]: ", (int)difftime(time(NULL), start_time));
  for (int i = 0; i < get_output_buffers_size(); i++) {
    for (int j = 0; j < get_buffer_size(); j++) {
      printf("%c", output_buffers[i][j]);
    }
    printf(" ");
  }
  printf("\n\n");
}

char fetch_input(int i) {
  srand(rand() + i);
  return rand_in_range('A', 'Z' + 1);
}

int handle_input(int i, char c) {
  srand(rand() + i + c);
  return (rand()) % get_input_buffers_size();
}

void handle(int j, char p, char *r, int *t) {
  srand(rand() + j + p);
  *r = rand_in_range('A', 'Z' + 1);
  *t = rand() % get_output_buffers_size();
}

void *input_thread(void *thread_id) {
  int id = *((int *)thread_id);
  free(thread_id);

  while (!threads_should_join) {
    char u = fetch_input(id);
    int t = handle_input(id, u);

    write_to_input_buffer(t, u);

    sem_wait(&sem_stdout);

    printf(
        "t=%d\tI%d: fetch_input(%d) => '%c'; handle_input('%c') => %d; '%c' => "
        "input_buffers[%d]\n",
        (int)difftime(time(NULL), start_time), id, id, u, u, t, u, t);

    print_buffers();

    sem_post(&sem_stdout);

    sleep(rand_in_range(5, 11));
  }

  return NULL;
}

void *work_thread(void *thread_id) {
  int id = *((int *)thread_id);
  free(thread_id);

  while (!threads_should_join) {
    char p = read_from_input_buffer(id);
    char r;
    int t;
    handle(id, p, &r, &t);

    write_to_output_buffer(t, r);

    sem_wait(&sem_stdout);

    printf("t=%d\tW%d: handle('%c') => '%c', %d; '%c' => output_buffers[%d]\n",
           (int)difftime(time(NULL), start_time), id, p, r, t, r, t);

    print_buffers();

    sem_post(&sem_stdout);

    sleep(rand_in_range(2, 4));
  }

  return NULL;
}

void *output_thread(void *thread_id) {
  int id = *((int *)thread_id);
  free(thread_id);

  char c = '0';

  while (!threads_should_join) {
    char new = read_from_output_buffer(id);
    if (new != '-') {
      c = new;
    }

    sem_wait(&sem_stdout);

    printf("t=%d\tO%d: printing %c\n", (int)difftime(time(NULL), start_time),
           id, c);
    print_buffers();

    sem_post(&sem_stdout);

    sleep(5);
  }

  return NULL;
}

void threads_init(int input_threads_no, int work_threads_no,
                  int output_threads_no) {

  input_threads_size = input_threads_no;
  work_threads_size = work_threads_no;
  output_threads_size = output_threads_no;

  input_threads = (pthread_t *)malloc(sizeof(pthread_t[input_threads_no]));
  work_threads = (pthread_t *)malloc(sizeof(pthread_t[work_threads_no]));
  output_threads = (pthread_t *)malloc(sizeof(pthread_t[output_threads_no]));

  sem_init(&sem_stdout, 0, 1);

  start_time = time(NULL);

  for (int i = 0; i < input_threads_size; i++) {
    int *thread_id = (int *)malloc(sizeof(int));
    *thread_id = i;
    pthread_create(&input_threads[i], NULL, input_thread, thread_id);
  }
  for (int i = 0; i < output_threads_size; i++) {
    int *thread_id = (int *)malloc(sizeof(int));
    *thread_id = i;
    pthread_create(&output_threads[i], NULL, output_thread, thread_id);
  }
  sleep(60);
  for (int i = 0; i < work_threads_size; i++) {
    int *thread_id = (int *)malloc(sizeof(int));
    *thread_id = i;
    pthread_create(&work_threads[i], NULL, work_thread, thread_id);
  }

  while (true) {
    sleep(5);
  }
}

void threads_cleanup(void) {
  threads_should_join = true;

  for (int i = 0; i < input_threads_size; i++) {
    pthread_join(input_threads[i], NULL);
  }
  for (int i = 0; i < work_threads_size; i++) {
    pthread_join(work_threads[i], NULL);
  }
  for (int i = 0; i < output_threads_size; i++) {
    pthread_join(output_threads[i], NULL);
  }

  free(input_threads);
  free(work_threads);
  free(output_threads);
  sem_destroy(&sem_stdout);
}
