#ifndef INCLUDE_MONITORI_LIST_H_
#define INCLUDE_MONITORI_LIST_H_

#include <stdint.h>
#include <stdlib.h>

typedef void *List;

List new_list();
void list_append(List self, uint8_t value);
uint8_t list_remove(List self, size_t index);
uint8_t list_get(List self, size_t index);
size_t list_size(List self);
void list_delete(List self);

#endif // INCLUDE_MONITORI_LIST_H_
