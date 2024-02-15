#pragma once

#include <stdint.h>
#include "settings.h"

typedef enum RecordType 
{
    noneRecord,
    rootRecord,
    classRecord,
    methodRecord,
    variableRecord
}RecordType_e;

typedef union Record
{
    struct Meta
    {
        union Record* prevScope;
        uint32_t size;
        uint32_t capacity;    
    }Meta;
    
    struct Entry 
    {
        RecordType_e record;
        const char* name;
        const char* type;
        union Record* subScope;
    }Entry;
} Record_u;

typedef struct SymbolTable
{
    Record_u* currentScope;
    Record_u rootScope;
} SymbolTable_s;

typedef struct Node Node_s;
SymbolTable_s GenerateSymboltable(Node_s* AST);

void STGenerateVisualization(SymbolTable_s* ST);
