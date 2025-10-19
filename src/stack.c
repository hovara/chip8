#include "stack.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

void stack_init(Stack *s) {
  s->top = NULL;
  s->size = 0;
}

bool stack_push(Stack *s, uint16_t addr) {
  Node *n = (Node *)malloc(sizeof(Node));
  if (!n) {
    printf("Error in func \"stack_push()\" : not enough memory to allocate!\n");
    return false;
  }
  n->addr = addr;
  n->next = s->top;
  s->top = n;
  s->size++;
  return true;
}

bool stack_pop(Stack *s, uint16_t *out_addr) {
  if (!s->top)
    return false;
  Node *n = s->top;
  if (out_addr)
    *out_addr = n->addr;
  s->top = n->next;
  free(n);
  s->size--;
  return true;
}

bool stack_peek(Stack *s, uint16_t *out_addr) {
  if (!s->top)
    return false;
  if (out_addr)
    *out_addr = s->top->addr;
  return true;
}
bool stack_is_empty(Stack *s) { return s->top == NULL; }

size_t stack_size(Stack *s) { return s->size; }

void stack_clear(Stack *s) {
  while (stack_pop(s, NULL)) {
  }
}

