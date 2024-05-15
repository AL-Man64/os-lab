#include "buffers.h"
#include <errno.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>

// Size of single buffer
int buffer_size = 0;

// Amount of buffers
int input_buffers_size = 0;
int output_buffers_size = 0;

// Buffers
char **input_buffers = NULL;
char **output_buffers = NULL;

// Indices for each buffer
int *input_buffers_write_idxs = NULL;
int *output_buffers_write_idxs = NULL;
int *input_buffers_read_idxs = NULL;
int *output_buffers_read_idxs = NULL;

// Semaphores for each buffer
sem_t *input_buffers_sems = NULL;
sem_t *input_buffers_read_sems = NULL;
sem_t *output_buffers_sems = NULL;
sem_t *output_buffers_read_sems = NULL;

void init_buffers(int work_threads, int output_threads, int buf_size_loc) {
  // Save sizes

  buffer_size = buf_size_loc;

  input_buffers_size = work_threads;
  output_buffers_size = output_threads;

  // Initialize arrays

  input_buffers = (char **)malloc(sizeof(char *[input_buffers_size]));
  output_buffers = (char **)malloc(sizeof(char *[output_buffers_size]));

  input_buffers_write_idxs = (int *)malloc(sizeof(int[input_buffers_size]));
  output_buffers_write_idxs = (int *)malloc(sizeof(int[input_buffers_size]));
  input_buffers_read_idxs = (int *)malloc(sizeof(int[input_buffers_size]));
  output_buffers_read_idxs = (int *)malloc(sizeof(int[input_buffers_size]));

  input_buffers_sems = (sem_t *)malloc(sizeof(sem_t[input_buffers_size]));
  input_buffers_read_sems = (sem_t *)malloc(sizeof(sem_t[input_buffers_size]));
  output_buffers_sems = (sem_t *)malloc(sizeof(sem_t[input_buffers_size]));
  output_buffers_read_sems = (sem_t *)malloc(sizeof(sem_t[input_buffers_size]));

  // Fill arrays with appropriate values

  for (int i = 0; i < input_buffers_size; i++) {
    input_buffers[i] = (char *)malloc(sizeof(char[buffer_size]));
    for (int j = 0; j < buffer_size; j++) {
      input_buffers[i][j] = '-';
    }
    input_buffers_write_idxs[i] = 0;
    input_buffers_read_idxs[i] = 0;
    sem_init(&input_buffers_sems[i], 0, 1);
    sem_init(&input_buffers_read_sems[i], 0, 0);
  }

  for (int i = 0; i < output_buffers_size; i++) {
    output_buffers[i] = (char *)malloc(sizeof(char[buffer_size]));
    for (int j = 0; j < buffer_size; j++) {
      output_buffers[i][j] = '-';
    }
    output_buffers_write_idxs[i] = 0;
    output_buffers_read_idxs[i] = 0;
    sem_init(&output_buffers_sems[i], 0, 1);
    sem_init(&output_buffers_read_sems[i], 0, 0);
  }
}

void clean_up_buffers(void) {
  // Free memory

  for (int i = 0; i < input_buffers_size; i++) {
    free(input_buffers[i]);
    sem_destroy(&input_buffers_sems[i]);
  }

  for (int i = 0; i < output_buffers_size; i++) {
    free(output_buffers[i]);
    sem_destroy(&output_buffers_sems[i]);
  }

  free(input_buffers);
  free(output_buffers);

  free(input_buffers_write_idxs);
  free(input_buffers_read_idxs);
  free(output_buffers_write_idxs);
  free(output_buffers_read_idxs);

  free(input_buffers_sems);
  free(output_buffers_sems);

  // Reset variables

  buffer_size = 0;
  input_buffers_size = 0;
  output_buffers_size = 0;
  input_buffers = NULL;
  output_buffers = NULL;
  input_buffers_write_idxs = NULL;
  output_buffers_write_idxs = NULL;
  input_buffers_read_idxs = NULL;
  output_buffers_read_idxs = NULL;
  input_buffers_sems = NULL;
  output_buffers_sems = NULL;
}

void write_to_input_buffer(int i, char value) {
  if (i >= input_buffers_size) {
    fprintf(stderr, "Index falls outside of allocated space");
    return;
  }

  sem_wait(&input_buffers_sems[i]);

  input_buffers[i][input_buffers_write_idxs[i]] = value;

  input_buffers_write_idxs[i] += 1;
  input_buffers_write_idxs[i] %= buffer_size;

  if (input_buffers_write_idxs[i] == input_buffers_read_idxs[i]) {
    input_buffers_read_idxs[i] += 1;
    input_buffers_read_idxs[i] %= buffer_size;
  } else {
    sem_post(&input_buffers_read_sems[i]);
  }

  sem_post(&input_buffers_sems[i]);
}

char read_from_input_buffer(int i) {
  if (i >= input_buffers_size) {
    fprintf(stderr, "Index falls outside of allocated space");
    return '-';
  }

  sem_wait(&input_buffers_read_sems[i]);
  sem_wait(&input_buffers_sems[i]);

  char c = input_buffers[i][input_buffers_read_idxs[i]];
  input_buffers[i][input_buffers_read_idxs[i]] = '-';

  input_buffers_read_idxs[i] += 1;
  input_buffers_read_idxs[i] %= buffer_size;

  sem_post(&input_buffers_sems[i]);

  return c;
}

void write_to_output_buffer(int i, char value) {
  if (i >= output_buffers_size) {
    fprintf(stderr, "Index falls outside of allocated space");
    return;
  }

  sem_wait(&output_buffers_sems[i]);

  output_buffers[i][output_buffers_write_idxs[i]] = value;

  output_buffers_write_idxs[i] += 1;
  output_buffers_write_idxs[i] %= buffer_size;

  if (output_buffers_write_idxs[i] == output_buffers_read_idxs[i]) {
    output_buffers_read_idxs[i] += 1;
    output_buffers_read_idxs[i] %= buffer_size;
  } else {
    sem_post(&output_buffers_read_sems[i]);
  }

  sem_post(&output_buffers_sems[i]);
}

char read_from_output_buffer(int i) {
  if (i >= output_buffers_size) {
    fprintf(stderr, "Index falls outside of allocated space");
    return '-';
  }

  sem_trywait(&output_buffers_read_sems[i]);
  if (errno == EAGAIN) {
    errno = 0;
    return '-';
  }

  sem_wait(&output_buffers_sems[i]);

  char c = output_buffers[i][output_buffers_read_idxs[i]];
  output_buffers[i][output_buffers_read_idxs[i]] = '-';

  output_buffers_read_idxs[i] += 1;
  output_buffers_read_idxs[i] %= buffer_size;

  sem_post(&output_buffers_sems[i]);

  return c;
}

int get_buffer_size() { return buffer_size; }

int get_input_buffers_size() { return input_buffers_size; }

int get_output_buffers_size() { return output_buffers_size; }

const char **get_input_buffers() { return (const char **)input_buffers; }

const char **get_output_buffers() { return (const char **)output_buffers; }
