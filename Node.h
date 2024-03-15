#pragma once

#include <stdint.h>
#include "settings.h"
#include "SymbolTable.h"

typedef struct Node
{
    int32_t      id;
    int32_t      lineno;
    int32_t      colno;
    const char*  type;
    const char*  value;
    RecordType_e record;

    struct Node** children;
    uint32_t      size;
    uint32_t      capacity;
} Node_s;

Node_s* initNodeTreeRecord(
    const char* type, const char* value, int32_t lineno, int32_t colno, RecordType_e record);
Node_s* initNodeTree(const char* type, const char* value, int32_t lineno, int32_t colno);
void    addSubTree(Node_s node[static 1], Node_s* newNode);

void printTree(Node_s node[static 1], int depth);
void generateTree(Node_s node[static 1]);
