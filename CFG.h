#pragma once

#include <stdint.h>

typedef enum ThreeAdressCodeOperators
{
    noneTac,

    // used for tree traversal check
    intExp,
    trueExp,
    falseExp,
    varExp,
    thisExp,

    // used for tree traversal check and denoting different TAC operators
    andTac,
    orTac,
    greaterTac,
    lesserTac,
    equalTac,
    addTac,
    subTac,
    mulTac,
    divTac,
    indexTac,
    lengthTac,
    functionTac,

    negTac,

    assignTac
} TACOp_e;

typedef struct ThreeAdressCode
{
    TACOp_e op;
    const char* src1;
    const char* src2;
    const char* dst;
} TAC_s;

typedef struct ControlFlowGraphBlock
{
    uint32_t identifier;

    uint32_t tacSize;
    uint32_t tacCapacity;
    TAC_s* tacArray;
    uint32_t tempCount;

    TAC_s blockCondition;
    struct ControlFlowGraphBlock* trueExit;
    struct ControlFlowGraphBlock* falseExit;
} CFGBlock_s;

typedef struct ControlFlowGraph
{
    const char** graphNames;
    uint32_t size;
    uint32_t capacity;
    CFGBlock_s* graphList;
} CFG_s;
typedef struct Node Node_s;
CFG_s GenerateControlFlowGraphs(Node_s* AST);

void CFGGenerateVisualization(CFG_s CFG[static 1]);
