#ifndef INCLUDE_SEMAFORI_BUFFERS_H_
#define INCLUDE_SEMAFORI_BUFFERS_H_

// Initialize buffers with given sizes
void init_buffers(int work_threads, int output_threads, int buf_size);

// Clean up buffers
void clean_up_buffers(void);

// Operations on i-th input buffer
void write_to_input_buffer(int i, char value);
char read_from_input_buffer(int i);

// Operations on i-th output buffer
void write_to_output_buffer(int i, char value);
char read_from_output_buffer(int i);

// Get variables from module
int get_buffer_size();
int get_input_buffers_size();
int get_output_buffers_size();
const char **get_input_buffers();
const char **get_output_buffers();

#endif // INCLUDE_SEMAFORI_BUFFERS_H_
