#include "SymbolTable.h"
#include "Node.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define START_CAPACITY 5

void STAddEntry(Record_u     scope[static 1],
                RecordType_e record,
                const char*  name,
                const char*  type,
                int32_t      lineno)
{
    if (scope->Entry.subScope[0].Meta.size >= scope->Entry.subScope[0].Meta.capacity)
    {
        scope->Entry.subScope[0].Meta.capacity *= 2;
        scope->Entry.subScope = realloc(
            scope->Entry.subScope, sizeof(Record_u) * (1 + scope->Entry.subScope[0].Meta.capacity));
    }

    scope->Entry.subScope[0].Meta.size++;
    scope->Entry.subScope[scope->Entry.subScope[0].Meta.size].Entry.record   = record;
    scope->Entry.subScope[scope->Entry.subScope[0].Meta.size].Entry.name     = name;
    scope->Entry.subScope[scope->Entry.subScope[0].Meta.size].Entry.type     = type;
    scope->Entry.subScope[scope->Entry.subScope[0].Meta.size].Entry.lineno   = lineno;
    scope->Entry.subScope[scope->Entry.subScope[0].Meta.size].Entry.subScope = NULL;
}

Record_u* STAddScope(Record_u     scope[static 1],
                     RecordType_e record,
                     const char*  name,
                     const char*  type,
                     int32_t      lineno)
{
    STAddEntry(scope, record, name, type, lineno);

    scope->Entry.subScope[scope->Entry.subScope[0].Meta.size].Entry.subScope
        = (Record_u*) malloc(sizeof(Record_u) * (1 + START_CAPACITY));
    scope->Entry.subScope[scope->Entry.subScope[0].Meta.size].Entry.subScope[0].Meta.prevScope
        = scope;
    scope->Entry.subScope[scope->Entry.subScope[0].Meta.size].Entry.subScope[0].Meta.size = 0;
    scope->Entry.subScope[scope->Entry.subScope[0].Meta.size].Entry.subScope[0].Meta.capacity
        = START_CAPACITY;

    return &scope->Entry.subScope[scope->Entry.subScope[0].Meta.size];
}

void GenerateTableRec(Node_s AST[static 1], Record_u scope[static 1])
{
    switch (AST->record)
    {
        case classRecord:
            scope = STAddScope(scope, classRecord, AST->value, AST->value, AST->lineno);
            break;

        case methodRecord:
            scope
                = STAddScope(scope, methodRecord, AST->value, AST->children[0]->value, AST->lineno);
            break;

        case variableRecord:
            STAddEntry(scope, variableRecord, AST->value, AST->type, AST->lineno);
            break;

        case noneRecord:
        case rootRecord:
            break;
    }

    for (uint32_t i = 0; i < AST->size; i++)
    {
        GenerateTableRec(AST->children[i], scope);
    }
}

SymbolTable_s GenerateSymboltable(Node_s AST[static 1])
{
    SymbolTable_s table;
    table.rootScope.Entry.record = rootRecord;
    table.rootScope.Entry.name   = "Program";
    table.rootScope.Entry.type   = "";

    table.rootScope.Entry.subScope = (Record_u*) malloc(sizeof(Record_u) * (1 + START_CAPACITY));
    table.rootScope.Entry.subScope[0].Meta.prevScope = NULL;
    table.rootScope.Entry.subScope[0].Meta.size      = 0;
    table.rootScope.Entry.subScope[0].Meta.capacity  = START_CAPACITY;

    for (uint32_t i = 0; i < AST->size; i++)
    {
        GenerateTableRec(AST->children[i], &table.rootScope);
    }

    table.currentScope = &table.rootScope;
    return table;
}

static void generateVizContent(Record_u scope[static 1], int count[static 1], FILE file[static 1])
{
    scope->Entry.id = ++(*count);

    const char* labelFormat = "n%d [label=\"%s:%s\n%s\"];\n";
    const char* nnFormat    = "n%d -> n%d\n";

    const char* labelSubFormat = "%s name:%s type:%s\n";
    uint32_t    lableCapacity  = 100;
    uint32_t    lableSize      = 0;
    char*       labelBuf       = malloc(lableCapacity);
    memset((void*) labelBuf, 0, lableCapacity);
    char lableRecord[9];

    if (scope->Entry.subScope != NULL)
        for (uint32_t i = 0; i < scope->Entry.subScope[0].Meta.size; i++)
        {
            switch (scope->Entry.subScope[i + 1].Entry.record)
            {
                case classRecord:
                    strcpy(lableRecord, "Class");
                    break;

                case methodRecord:
                    strcpy(lableRecord, "Method");
                    break;

                case variableRecord:
                    strcpy(lableRecord, "Variable");
                    break;

                case noneRecord:
                case rootRecord:
                    break;
            }
            uint32_t lineSize = snprintf(NULL,
                                         0,
                                         labelSubFormat,
                                         lableRecord,
                                         scope->Entry.subScope[i + 1].Entry.name,
                                         scope->Entry.subScope[i + 1].Entry.type);
            while (lableCapacity <= (lableSize + lineSize + 1))
            {
                lableCapacity *= 2;
                labelBuf = realloc((void*) labelBuf, lableCapacity);
            }

            sprintf(labelBuf + lableSize,
                    labelSubFormat,
                    lableRecord,
                    scope->Entry.subScope[i + 1].Entry.name,
                    scope->Entry.subScope[i + 1].Entry.type);
            lableSize += lineSize;
        }

    switch (scope->Entry.record)
    {
        case rootRecord:
            strcpy(lableRecord, "Root");
            break;
        case classRecord:
            strcpy(lableRecord, "Class");
            break;

        case methodRecord:
            strcpy(lableRecord, "Method");
            break;

        case variableRecord:
            strcpy(lableRecord, "Variable");
            break;

        case noneRecord:
            break;
    }
    char*   writebuf = NULL;
    int32_t writeSize
        = snprintf(NULL, 0, labelFormat, scope->Entry.id, lableRecord, scope->Entry.name, labelBuf);

    if (writeSize < 0)
    {
        fprintf(stderr, "[-] Unable to generate tree (sprintf).");
        exit(1);
    }
    size_t bufCapacity = sizeof(char[writeSize]) * 2;
    writebuf           = malloc(bufCapacity);

    sprintf(writebuf, labelFormat, scope->Entry.id, lableRecord, scope->Entry.name, labelBuf);
    fwrite(writebuf, sizeof(*writebuf), writeSize, file);

    if (labelBuf)
        free(labelBuf);

    if (scope->Entry.subScope != NULL)
        for (uint32_t i = 0; i < scope->Entry.subScope[0].Meta.size; ++i)
        {
            if ((scope->Entry.subScope[i + 1].Entry.record != classRecord)
                && (scope->Entry.subScope[i + 1].Entry.record != methodRecord))
                continue;
            generateVizContent(&(scope->Entry.subScope[i + 1]), count, file);

            writeSize = snprintf(
                NULL, 0, nnFormat, scope->Entry.id, scope->Entry.subScope[i + 1].Entry.id);
            if (writeSize < 0)
            {
                fprintf(stderr, "[-] Unable to generate st viz, recursion\n");
                exit(1);
            }

            if (((size_t) writeSize) > bufCapacity)
            {
                bufCapacity = writeSize * 2;
                writebuf    = realloc(writebuf, bufCapacity);
                if (writebuf == NULL)
                {
                    fprintf(stderr, "[-] Failed to realloc memory.\n");
                    exit(1);
                }
            }
            memset(writebuf, 0, bufCapacity);
            sprintf(writebuf, nnFormat, scope->Entry.id, scope->Entry.subScope[i + 1].Entry.id);
            fwrite(writebuf, sizeof(*writebuf), writeSize, file);
        }
    if (writebuf)
        free(writebuf);
}

void STGenerateVisualization(SymbolTable_s* ST)
{
    FILE* file;
    char* fname = "st.dot";
    file        = fopen(fname, "w");

    int32_t count      = 0;
    char    writebuf[] = "digraph {\n";
    fwrite(writebuf, sizeof(*writebuf), sizeof(writebuf) - 1, file);
    generateVizContent(&(ST->rootScope), &count, file);
    fwrite("}", 1, 1, file);
    fclose(file);

    printf("\nBuilt a symbol table graph at %s. Use 'make st' to generate the pdf version.\n",
           fname);
}


Record_u* STLookUp(SymbolTable_s ST[static 1], const char* identifier, int32_t* refcount)
{
    Record_u* retAddr = NULL;
    int32_t   refc    = 0;
    for (uint32_t i = 0; i < ST->currentScope->Entry.subScope[0].Meta.size; i++)
    {
        if (strcmp(identifier, ST->currentScope->Entry.subScope[1 + i].Entry.name) == 0)
        {
            ++refc;
            if (retAddr == NULL)
                retAddr = &(ST->currentScope->Entry.subScope[1 + i]);
        }
    }
    if (refcount != NULL)
        *refcount = refc;
    return retAddr;
}

Record_u* STDeepLookUp(SymbolTable_s ST[static 1], const char* identifier)
{
    Record_u* record = STLookUp(ST, identifier, NULL);
    if (record != NULL)
        return record;

    Record_u* scope = ST->currentScope;
    for (uint32_t i = 0; i < ST->currentScope->Entry.subScope[0].Meta.size; i++)
    {
        if (STSetScope(ST, &(scope->Entry.subScope[i + 1])) == 0)
        {
            record = STDeepLookUp(ST, identifier);
            if (record != NULL)
                break;
        }
    }
    ST->currentScope = scope;
    return record;
}

Record_u* STGoTo(SymbolTable_s ST[static 1], const char* identifier)
{
    Record_u* record = STLookUp(ST, identifier, NULL);
    if (record != NULL)
        return record;

    Record_u* scope = ST->currentScope;
    for (uint32_t i = 0; i < ST->currentScope->Entry.subScope[0].Meta.size; i++)
    {
        if (STSetScope(ST, &(scope->Entry.subScope[i + 1])) == 0)
        {
            record = STGoTo(ST, identifier);
            if (record != NULL)
                break;
        }
    }
    return record;
}


Record_u* STCurrentScope(SymbolTable_s ST[static 1]) { return ST->currentScope; }

int STSetScope(SymbolTable_s ST[static 1], Record_u scope[static 1])
{
    if (scope->Entry.subScope == NULL)
        return -1;
    ST->currentScope = scope;
    return 0;
}

void STResetScope(SymbolTable_s ST[static 1]) { ST->currentScope = &(ST->rootScope); }

Record_u* STEnterScope(SymbolTable_s ST[static 1], const char* identifier)
{
    Record_u* scope = STLookUp(ST, identifier, NULL);
    if ((scope == NULL) || (scope->Entry.subScope == NULL))
        return NULL;
    ST->currentScope = scope;
    return scope;
}

Record_u* STExitScope(SymbolTable_s ST[static 1])
{
    if (ST->currentScope == &(ST->rootScope))
        return NULL;
    ST->currentScope = ST->currentScope->Entry.subScope[0].Meta.prevScope;
    return ST->currentScope;
}


const char*   STName(Record_u record[static 1]) { return record->Entry.name; }
const char*   STType(Record_u record[static 1]) { return record->Entry.type; }
RecordType_e* STRecordType(Record_u record[static 1]) { return &(record->Entry.record); }
