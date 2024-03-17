#ifndef _STACK_H_
#define _STACK_H_

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct _DynamicStack
{
    size_t  size;
    size_t  capacity;
    int32_t stack[];
} stack_s;
stack_s* CreateStack()
{
    size_t   capacity = 64;
    stack_s* stack    = (stack_s*) malloc(sizeof(*stack) + sizeof(int32_t[capacity]));
    if (stack)
        *stack = (stack_s){ .size = 0, .capacity = capacity };
    return stack;
}
void DestroyStack(stack_s** stack) { free(*stack); }
void Push(stack_s** stack, int32_t value)
{
    if (stack == NULL)
        return;

    if ((*stack)->size + 1 >= (*stack)->capacity)
    {
        (*stack)->capacity *= 1.4;
        (*stack) = (stack_s*) realloc((*stack), sizeof(stack_s) + (*stack)->capacity);
        if ((*stack) == NULL)
            abort();
    }
    (*stack)->stack[(*stack)->size++] = value;
}
int32_t Pop(stack_s** stack)
{
    if (stack == NULL || (*stack)->size == 0)
        return -1;
    return (*stack)->stack[--(*stack)->size];
}

#endif  // _STACK_H_
