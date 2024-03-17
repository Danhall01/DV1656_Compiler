#include "BytecodeGeneration.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

uint32_t LabelsCap;
long* Labels;

typedef struct Caller
{
    uint32_t calledBlock;
    long filePosition;
} Caller_s;
uint32_t CallersCap;
uint32_t CallersSize;
Caller_s* Callers;

void RegisterLabel(uint32_t blockId, long linePosition)
{
    while (blockId >= LabelsCap)
    {
        LabelsCap *= 2;
        Labels = realloc(Labels, sizeof(long) * LabelsCap);
    }
    Labels[blockId] = linePosition;
}

void RegisterCaller(uint32_t calledBlock, long filePosition)
{
    if (CallersSize >= CallersCap)
    {
        CallersCap *= 2;
        Callers = realloc(Callers, sizeof(Caller_s) * CallersCap);
    }
    Callers[CallersSize].calledBlock = calledBlock;
    Callers[CallersSize].filePosition = filePosition;
    CallersSize++;
}

void WriteLine(const char* line, size_t characters, FILE* file)
{
    fwrite(line, 1, characters, file);
}

uint32_t BufCap;
char* Buf;
void WriteInstruction(FILE* file, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    
    uint32_t lineSize = vsnprintf(NULL, 0, format, args); 
    
    while (BufCap <= (lineSize + 1))
    {
        BufCap *= 2;
        Buf = realloc((void*) Buf, BufCap);
    }

    va_start(args, format);
    vsprintf(Buf, format, args);
    WriteLine(Buf, lineSize, file);
}

uint32_t VarCount;
uint32_t TempVarCap;
uint32_t* TempVariables;
uint32_t VarIndex(const char* var, SymbolTable_s* ST)
{
    uint32_t index = 0;
    if (strncmp(var, "__t", 3) == 0)
    {
        index = atoi(var + 3);

        if (TempVariables[index] == 0xffffffff)
        {
            while(index >= TempVarCap)
            {
                uint32_t* temp = malloc(sizeof(uint32_t*) * TempVarCap * 2);
                memset(temp + sizeof(uint32_t*) * TempVarCap, 0xff, sizeof(uint32_t*) * TempVarCap);
                memcpy(temp, TempVariables, sizeof(uint32_t*) * TempVarCap);
                TempVarCap *= 2;
                TempVariables = temp;
            }

            if (VarCount == 0xffffffff)
            {
                fprintf(stderr, "Error! code generation overflow variable count.\n");
                exit(1);
            }
            TempVariables[index] = VarCount++;
        }
        
        return TempVariables[index];
    }

    Record_u* startScope = ST->currentScope; 
    do
    {
        for (uint32_t i = 0; i < ST->currentScope->Entry.subScope[0].Meta.size; i++)
        {
            if ((ST->currentScope->Entry.subScope[1 + i].Entry.record == variableRecord) && 
                (strcmp(ST->currentScope->Entry.subScope[1 + i].Entry.name, var) == 0))
            {
                if (ST->currentScope->Entry.subScope[1 + i].Entry.varIndex == 0xffffffff)
                {
                    if (VarCount == 0xffffffff)
                    {
                        fprintf(stderr, "Error! code generation overflow variable count.\n");
                        exit(1);
                    }
                    index = VarCount++;
                    ST->currentScope->Entry.subScope[1 + i].Entry.varIndex = index;
                    STSetScope(ST, startScope);
                    return index;
                }
                else
                {
                    index = ST->currentScope->Entry.subScope[1 + i].Entry.varIndex;
                    STSetScope(ST, startScope);
                    return index;
                }
            }
        }
    } while (STExitScope(ST) != NULL);
    
    STSetScope(ST, startScope);
    return 0xffffffff;
}

int TranslateInput(const char* exp, SymbolTable_s* ST, int64_t* translated)
{
    if ((exp[0] == '-') || ((exp[0] >= '0') && (exp[0] <= '9')))
    {
        *translated = atoi(exp);
        return 0;
    }

    if (strcmp(exp, "true") == 0)
    {
        *translated = 1;
        return 0;
    }
    if (strcmp(exp, "false") == 0)
    {
        *translated = 0;
        return 0;
    }
        
    *translated = VarIndex(exp, ST);
    return 1;
}


#define LOAD(exp) WriteInstruction(file, "%s%ld\n", TranslateInput(exp, ST, &translated) ? "iload #" : "iconst ", translated)
#define STORE(exp) if (strcmp(exp, "") != 0) WriteInstruction(file, "istore #%lu\n", VarIndex(exp, ST))

#define SIMPLEEXPRESSION(op) {             \
    LOAD(tac->src1);                    \
    LOAD(tac->src2);                    \
    WriteLine(op, strlen(op), file);    \
    STORE(tac->dst);                    \
}

const char* ClassCaller;
TAC_s* ClassCallerTac;
void TranslateInstruction(FILE* file, TAC_s* tac, SymbolTable_s* ST, CFG_s* CFG, const char* className)
{
    int64_t translated;
    char*   blockName;
    const char* format;
    int32_t nameSize;
    switch (tac->op)
    {
    case addTac:
        SIMPLEEXPRESSION("iadd\n");
        break;
    case subTac:
        SIMPLEEXPRESSION("isub\n");
        break;
    case mulTac:
        SIMPLEEXPRESSION("imul\n");
        break;
    case divTac:
        SIMPLEEXPRESSION("idiv\n");
        break;
    case lesserTac:
        SIMPLEEXPRESSION("ilt\n");
        break;
    case greaterTac:
        LOAD(tac->src2);
        LOAD(tac->src1);
        WriteLine("ilt\n", 4, file);
        STORE(tac->dst);
        break;
    case andTac:
        SIMPLEEXPRESSION("iand\n");
        break;
    case orTac:
        SIMPLEEXPRESSION("ior\n");
        break;

    case equalTac:
        LOAD(tac->src1);
        LOAD(tac->src2);
        WriteLine("isub\n", 4, file);
        WriteLine("inot\n", 4, file);
        STORE(tac->dst);
        break;
    case negTac:
        LOAD(tac->src1);
        WriteLine("inot\n", 4, file);
        STORE(tac->dst);
        break;

    case assignTac:
        LOAD(tac->src1);
        STORE(tac->dst);
        break;

    case printTac:
        LOAD(tac->src1);
        WriteLine("print\n", 6, file);
        break;
    case returnTac:
        LOAD(tac->src1);
        WriteLine("ireturn\n", 8, file);
        break;

    case callerTac:
        ClassCaller = tac->src1;
        ClassCallerTac = tac;
        break;
    case paramTac:
        LOAD(tac->src1);
        break;
    case functionTac:
        blockName = NULL;
        format = "%s-%s";
        if (strncmp(ClassCaller, "__t", 3) == 0)
        {
            ClassCaller = (ClassCallerTac - 1)->src1; 
        }
        if (strcmp(ClassCaller, "this") == 0)
        {
            ClassCaller = className;
        }
        nameSize = snprintf(NULL, 0, format, ClassCaller, tac->src1);
        blockName = malloc(nameSize);
        sprintf(blockName, format, ClassCaller, tac->src1);
        for (uint32_t i = 0; i < CFG->size; i++)
        {
            if (strcmp(blockName, CFG->graphNames[i]) == 0)
            {
                RegisterCaller(CFG->graphList[i].identifier, ftell(file) + 14);
                WriteLine("invokevirtual 00000000000000000000\n", 35, file);
                STORE(tac->dst);
                free(blockName);
                return;
            }
        }
        free(blockName);
        fprintf(stderr,"Error! missing function called\n");
        exit(1);
    }
}

void RecCFGTraverse(FILE* file, CFGBlock_s* block, SymbolTable_s* ST, CFG_s* CFG, const char* className, int8_t first)
{
    block->visited = 1;
    if (!first) RegisterLabel(block->identifier, ftell(file));

    for (uint32_t i = 0; i < block->tacSize; i++)
    {
        TranslateInstruction(file, &block->tacArray[i], ST, CFG, className);
    }
    
    if ((block->blockCondition.op == assignTac) && (strcmp(block->blockCondition.src1, "true") == 0))
    {
        if (block->trueExit == NULL)
        {
            if (block->tacArray[block->tacSize - 1].op != returnTac)
                WriteLine("stop\n", 5, file);
        }
        else
        {
            RegisterCaller(block->trueExit->identifier, ftell(file) + 5);
            WriteLine("goto 00000000000000000000\n", 26, file);
            if (!block->trueExit->visited) RecCFGTraverse(file, block->trueExit, ST, CFG, className, 0);
        }
    }
    else
    {
        TranslateInstruction(file, &block->blockCondition, ST, CFG, className);
        RegisterCaller(block->falseExit->identifier, ftell(file) + 13);
        WriteLine("iffalse goto 00000000000000000000\n", 34, file);
        if (!block->trueExit->visited) RecCFGTraverse(file, block->trueExit, ST, CFG, className, 0);
        if (!block->falseExit->visited) RecCFGTraverse(file, block->falseExit, ST, CFG, className ,0);
    }
}

void CFGParse(FILE* file, const char* className, const char* methodName, CFGBlock_s* method, SymbolTable_s* ST, CFG_s* CFG)
{
    VarCount = 0;
    RegisterLabel(method->identifier, ftell(file));

    STResetScope(ST);
    if (STEnterScope(ST, className) == NULL)
    {
        fprintf(stderr, "Error! intermediate representation and symbol table has class mismatch\n");
        exit(1);
    }
    if (STEnterScope(ST, methodName) == NULL)
    {
        fprintf(stderr, "Error! intermediate representation and symbol table has method mismatch\n");
        exit(1);
    }
    
    Record_u* scope = ST->currentScope;
    for (uint32_t i = 0; i < scope->Entry.subScope[0].Meta.paramc; i++)
    {
        WriteInstruction(file, "istore #%lu\n", VarIndex(scope->Entry.subScope[1 + i].Entry.name, ST));
    }

    RecCFGTraverse(file, method, ST, CFG, className, 1);

    WriteLine("\n", 1, file);
}

int GenerateJavaBytecode(const char* savePath, CFG_s* CFG, SymbolTable_s* ST)
{
    FILE* file;
    file = fopen(savePath, "w");

    uint32_t methodCap = 40;
    char* methodBuf = malloc(methodCap);
    char* saveptr;

    TempVarCap = 10;
    TempVariables = malloc(sizeof(uint32_t*) * TempVarCap);
    memset(TempVariables, 0xff, sizeof(uint32_t*) * TempVarCap);

    BufCap = 50;
    Buf = malloc(BufCap);

    LabelsCap = 20;
    Labels = malloc(sizeof(size_t) * LabelsCap);

    CallersSize = 0;
    CallersCap = 40;
    Callers = malloc(sizeof(Caller_s) * CallersCap);

    for (uint32_t i = 0; i < CFG->size; i++)
    {
        while (strlen(CFG->graphNames[i]) >= methodCap)
        {
            methodCap *= 2;
            methodBuf = realloc(methodBuf, methodCap);
        }
        strcpy(methodBuf, CFG->graphNames[i]);
        
        CFGParse(file, strtok_r(methodBuf, "-", &saveptr), strtok_r(NULL, "-", &saveptr), &(CFG->graphList[i]), ST, CFG);
    }
    free(methodBuf);
    free(TempVariables);
    for (uint32_t i = 0; i < CallersSize; i++)
    {
        int numerals = sprintf(Buf, "%lu", Labels[Callers[i].calledBlock]);
        fseek(file, Callers[i].filePosition + 20 - numerals, SEEK_SET);
        fwrite(Buf, 1, numerals, file);
    }
    free(Buf);
    free(Callers);
    free(Labels);
    fclose(file);
    return 0;
}