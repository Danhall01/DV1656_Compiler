#include "SymbolTable.h"
#include "Node.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define START_CAPACITY 5

void STAddEntry(Record_u scope[static 1], RecordType_e record, const char* name, const char* type)
{
    if (scope->Entry.subScope[0].Meta.size >= scope->Entry.subScope[0].Meta.capacity)
    {
        scope->Entry.subScope[0].Meta.capacity *= 2;
        scope->Entry.subScope = realloc(scope->Entry.subScope, sizeof(Record_u) * (1 + scope->Entry.subScope[0].Meta.capacity));
    }
    
    scope->Entry.subScope[0].Meta.size++;
    scope->Entry.subScope[scope->Entry.subScope[0].Meta.size].Entry.record = record;
    scope->Entry.subScope[scope->Entry.subScope[0].Meta.size].Entry.name = name;
    scope->Entry.subScope[scope->Entry.subScope[0].Meta.size].Entry.type = type;
    scope->Entry.subScope[scope->Entry.subScope[0].Meta.size].Entry.subScope = NULL;
}

Record_u* STAddScope(Record_u scope[static 1], RecordType_e record, const char* name, const char* type)
{
    STAddEntry(scope, record, name, type);

    scope->Entry.subScope[scope->Entry.subScope[0].Meta.size].Entry.subScope = (Record_u*)malloc(sizeof(Record_u) * (1 + START_CAPACITY));
    scope->Entry.subScope[scope->Entry.subScope[0].Meta.size].Entry.subScope[0].Meta.prevScope = scope;
    scope->Entry.subScope[scope->Entry.subScope[0].Meta.size].Entry.subScope[0].Meta.size = 0;
    scope->Entry.subScope[scope->Entry.subScope[0].Meta.size].Entry.subScope[0].Meta.capacity = START_CAPACITY;

    return &scope->Entry.subScope[scope->Entry.subScope[0].Meta.size];
}

void GenerateTableRec(Node_s AST[static 1], Record_u scope[static 1])
{
    switch (AST->record)
    {
    case classRecord:
        scope = STAddScope(scope, classRecord, AST->value, AST->value);
        break;

    case methodRecord:
        scope = STAddScope(scope, methodRecord, AST->value, AST->children[0]->value);
        break;

    case variableRecord:
        STAddEntry(scope, variableRecord, AST->value, AST->type);
        break;
    }

    for (int i = 0; i < AST->size; i++)
    {
        GenerateTableRec(AST->children[i], scope);
    }
}

SymbolTable_s GenerateSymboltable(Node_s AST[static 1])
{
    SymbolTable_s table;
    table.rootScope.Entry.record = rootRecord;
    table.rootScope.Entry.name = AST->type;
    table.rootScope.Entry.type = "";

    table.rootScope.Entry.subScope = (Record_u*)malloc(sizeof(Record_u) * (1 + START_CAPACITY));
    table.rootScope.Entry.subScope[0].Meta.prevScope = NULL;
    table.rootScope.Entry.subScope[0].Meta.size = 0;
    table.rootScope.Entry.subScope[0].Meta.capacity = START_CAPACITY;

    for (int i = 0; i < AST->size; i++)
    {
        GenerateTableRec(AST->children[i], &table.rootScope);
    }

    table.currentScope = &table.rootScope;
    return table;
}
/*
static void generateVizContent(Record_u scope[static 1], int count[static 1], FILE file[static 1])
{
    node->id = ++(*count);

    const char* labelFormat = "n%d [label=\"%s:%s\"];\n";
    const char* nnFormat    = "n%d -> n%d\n";

    char*   writebuf  = NULL;
    int32_t writeSize = snprintf(NULL, 0, labelFormat, node->id, node->type, node->value);
    if (writeSize < 0)
    {
        fprintf(stderr, "[-] Unable to generate tree (sprintf).");
        exit(1);
    }
    size_t bufCapacity = sizeof(char[writeSize]) * 2;
    writebuf           = malloc(bufCapacity);

    sprintf(writebuf, labelFormat, node->id, node->type, node->value);
    fwrite(writebuf, sizeof(*writebuf), writeSize, file);

    for (uint32_t i = 0; i < node->size; ++i)
    {
        generateVizContent(node->children[i], count, file);

        writeSize = snprintf(NULL, 0, nnFormat, node->id, node->children[i]->id);
        if (writeSize < 0)
        {
            fprintf(stderr, "[-] Unable to generate tree, recursion\n");
            exit(1);
        }

        if (((size_t)writeSize) > bufCapacity)
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
        sprintf(writebuf, nnFormat, node->id, node->children[i]->id);
        fwrite(writebuf, sizeof(*writebuf), writeSize, file);
    }
    if (writebuf)
        free(writebuf);
}

void STGenerateVisualization(SymbolTable_s* ST)
{
    FILE* file;
    char* fname = "symbolTable.dot";
    file        = fopen(fname, "w");

    int32_t count      = 0;
    char    writebuf[] = "digraph {\n";
    fwrite(writebuf, sizeof(*writebuf), sizeof(writebuf) - 1, file);

    generateVizContent(node, &count, file);

    fwrite("}", 1, 1, file);
    fclose(file);

    printf("\nBuilt a symbol table graph at %s. Use 'make st' to generate the pdf version.\n", fname);
}
*/