#pragma once

#include <stdint.h>
#include "settings.h"

typedef struct Node
{
    int32_t id;
    int32_t lineno;
    const char*   type;
    const char*   value;

    struct Node** children;
    uint32_t      size;
    uint32_t      capacity;
} Node_s;

Node_s* initNodeTree(const char* type, const char* value, int32_t lineno);
void    addSubTree(Node_s node[static 1], Node_s* newNode);

void printTree(Node_s node[static 1], int depth);
void generateTree(Node_s node[static 1]);
