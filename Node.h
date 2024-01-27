#pragma once

#include <stdint.h>
#include "settings.h"

typedef struct Node
{
    int32_t id;
    int32_t lineno;
    char*   type;
    char*   value;

    struct Node** children;
    uint32_t      size;
    uint32_t      capacity;
} Node_s;

Node_s* initNodeTree(char* type, char* value, int32_t lineno);
void    addSubTree(Node_s node[static 1], Node_s newNode[static 1]);

void printTree(Node_s node[static 1], int depth);
void generateTree(Node_s node[static 1]);
