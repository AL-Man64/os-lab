#include "list.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

struct node {
  uint8_t value;
  struct node *next;
};

struct list {
  struct node *head;
  size_t size;
};

List new_list() {
  struct list *self = (struct list *)malloc(sizeof(struct list));

  self->head = NULL;
  self->size = 0;

  return (List)self;
}

void list_append(List l, uint8_t value) {
  struct list *self = (struct list *)l;

  struct node *new_node = (struct node *)malloc(sizeof(struct node));

  new_node->value = value;
  new_node->next = NULL;

  if (self->head == NULL) {
    self->head = new_node;
    self->size = 1;

    return;
  }

  struct node *current = self->head;

  while (current->next != NULL) {
    current = current->next;
  }

  current->next = new_node;
  self->size += 1;
}

uint8_t list_remove(List l, size_t index) {
  struct list *self = (struct list *)l;

  struct node *prev = NULL;
  struct node *current = self->head;

  for (int i = 0; i < index && current != NULL; i++) {
    prev = current;
    current = current->next;
  }

  if (current == NULL) {
    return 0;
  }

  if (prev == NULL) {
    self->head = current->next;
  } else {
    prev->next = current->next;
  }
  uint8_t value = current->value;

  free(current);

  self->size -= 1;

  return value;
}

void list_delete(List l) {
  struct list *self = (struct list *)l;

  struct node *current = self->head;

  while (current != NULL) {
    struct node *next = current->next;
    free(current);
    current = next;
  }

  free(self);
}

uint8_t list_get(List l, size_t index) {
  struct list *self = (struct list *)l;

  struct node *prev = NULL;
  struct node *current = self->head;

  for (int i = 0; i < index && current != NULL; i++) {
    prev = current;
    current = current->next;
  }

  if (current == NULL) {
    return 0;
  }

  return current->value;
}

size_t list_size(List l) {
  struct list *self = (struct list *)l;

  return self->size;
}
