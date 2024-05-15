#include "buffers.h"
#include "threads.h"
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

void sigint_handler(int _) {
  threads_cleanup();
  clean_up_buffers();

  exit(EXIT_SUCCESS);
}

int main(int argc, char **argv) {
  if (argc != 5) {
    fprintf(stderr, "Expected arguments: <input_threads> <work_threads> "
                    "<output_threads> <buf_size>\n");
    return EXIT_FAILURE;
  }

  if (atoi(argv[1]) <= 0 || atoi(argv[2]) <= 0 || atoi(argv[3]) <= 0 ||
      atoi(argv[4]) <= 0) {
    fprintf(stderr, "All arguments should be numbers greater than 0");
    return EXIT_FAILURE;
  }

  signal(SIGINT, sigint_handler);

  srand(time(NULL));

  init_buffers(atoi(argv[2]), atoi(argv[3]), atoi(argv[4]));

  threads_init(atoi(argv[1]), atoi(argv[2]), atoi(argv[3]));
}
