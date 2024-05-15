#ifndef INCLUDE_SEMAFORI_THREADS_H_
#define INCLUDE_SEMAFORI_THREADS_H_

void threads_init(int input_threads_no, int work_threads_no,
                  int output_threads_no);

void threads_cleanup(void);

#endif // INCLUDE_SEMAFORI_THREADS_H_
