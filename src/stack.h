#ifndef STACK_H
#define STACK_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct Node {
  struct Node *next;
  uint16_t addr;
} Node;

typedef struct Stack {
  Node *top;
  size_t size;
} Stack;

void stack_init(Stack *s);
bool stack_push(Stack *s, uint16_t addr);
bool stack_pop(Stack *s, uint16_t *out_addr);
bool stack_peek(Stack *s, uint16_t *out_addr);
bool stack_is_empty(Stack *s);
size_t stack_size(Stack *s);
void stack_clear(Stack *s);

#endif
