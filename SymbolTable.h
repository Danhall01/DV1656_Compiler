#pragma once

#include <stdint.h>
#include "settings.h"
#include "Node.h"

typedef struct SymbolTable
{
    Record_u* currentScope;
    Record_u rootScope;
} SymbolTable_s;

enum RecordType 
{
    classRecord,
    methodRecord,
    variablesRecord
};

enum SymbolType
{
    integer,
    intArray,
    boolean
};

typedef union Record
{
    struct Meta
    {
        Record_u* prevScope;
        uint32_t size;
        uint32_t capacity;    
    };
    
    struct Entry 
    {
        RecordType record;
        const char* name;
        SymbolType type;
        Record_u* SubScope;
    };
} Record_u;

SymbolTable GenerateSymboltable(Node_s AST);
