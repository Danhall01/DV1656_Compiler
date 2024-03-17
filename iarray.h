#ifndef _IARRAY_H_
#define _IARRAY_H_

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct _integerArray
{
    size_t  capacity;
    int32_t arr[];
} iarray_s;
iarray_s* CreateArray()
{
    size_t    capacity = 64;
    iarray_s* ret      = (iarray_s*) malloc(sizeof(iarray_s) + sizeof(int32_t[capacity]));
    *ret               = (iarray_s){ .capacity = capacity };
    return ret;
}
void DestroyArray(iarray_s** array) { free((*array)); }
void Insert(iarray_s** array, size_t index, int32_t value)
{
    if (index >= (*array)->capacity)
    {
        (*array)->capacity = index * 1.4 + 1;
        (*array)           = (iarray_s*) realloc((*array), sizeof(iarray_s) + (*array)->capacity);
        if ((*array) == NULL)
            abort();
    }
    (*array)->arr[index] = value;
}

#endif  // _IARRAY_H_
