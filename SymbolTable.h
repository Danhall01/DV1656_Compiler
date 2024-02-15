#pragma once

#include <stdint.h>
#include "settings.h"

typedef enum RecordType 
{
    noneRecord,
    classRecord,
    methodRecord,
    variableRecord
}RecordType_e;

typedef enum SymbolType
{
    noneType,
    integerType,
    intArrayType,
    booleanType,
    voidType,
    stringArrayType
}SymbolType_e;

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
        SymbolType_e type;
        union Record* SubScope;
    }Entry;
} Record_u;

typedef struct SymbolTable
{
    Record_u* currentScope;
    Record_u rootScope;
} SymbolTable_s;

typedef struct Node Node_s;
SymbolTable_s GenerateSymboltable(Node_s* AST);

void STAddEntry(Record_u scope[static 1], RecordType_e record, const char* name, SymbolType_e type);

Record_u* STAddScope(Record_u scope[static 1], RecordType_e record, const char* name, SymbolType_e type);
