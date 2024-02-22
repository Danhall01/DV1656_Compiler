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
} RecordType_e;

typedef union Record
{
    struct Meta
    {
        union Record* prevScope;
        uint32_t      size;
        uint32_t      capacity;
    } Meta;

    struct Entry
    {
        RecordType_e  record;
        const char*   name;
        const char*   type;
        union Record* subScope;
        int           id;
    } Entry;
} Record_u;

typedef struct SymbolTable
{
    Record_u* currentScope;
    Record_u  rootScope;
} SymbolTable_s;

typedef struct Node Node_s;
SymbolTable_s       GenerateSymboltable(Node_s* AST);

void STGenerateVisualization(SymbolTable_s* ST);

// Searches current scope for identifier. returns first record if found. returns NULL if not
Record_u* STLookUp(SymbolTable_s ST[static 1], const char* identifier, int32_t* refcount);

// Searches current subtree (current and underlying scopes) for identifier. returns record if found.
// returns NULL if not
Record_u* STDeepLookUp(SymbolTable_s ST[static 1], const char* identifier);

// Searches current subtree (current and underlying scopes) for identifier. if found sets current
// scope to scope containing record and returns record. returns NULL if not
Record_u* STGoTo(SymbolTable_s ST[static 1], const char* identifier);


// Returns the current scope record
Record_u* STCurrentScope(SymbolTable_s ST[static 1]);

// Set the current scope, returns 0 if successful and the record was a scope
int STSetScope(SymbolTable_s ST[static 1], Record_u scope[static 1]);

// Sets the current scope to the root scope
void STResetScope(SymbolTable_s ST[static 1]);

// searches current scope for identifier and set it as the new current scope. returns the record for
// the new scope if successful. returns null if identifier could not be a scope
Record_u* STEnterScope(SymbolTable_s ST[static 1], const char* identifier);

// Exits current scope, sets the current scope to be the parent scope. returns the new current
// scope. if called in root, current scope is unchanged and NULL is returned
Record_u* STExitScope(SymbolTable_s ST[static 1]);


// setters/getters
const char*   STName(Record_u record[static 1]);
const char*   STType(Record_u record[static 1]);
RecordType_e* STRecordType(Record_u record[static 1]);
