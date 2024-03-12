#include "CFG.h"
#include "SymbolTable.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Node.h"

CFGBlock_s NewBlock(uint32_t* id)
{
    CFGBlock_s block;
    block.identifier = (*id)++;
    
    block.tacCapacity = 5;
    block.tacSize = 0;
    block.tacArray = malloc(sizeof(TAC_s) * block.tacCapacity);
    block.tempCount = 0;
    
    memset(&(block.blockCondition), 0, sizeof(TAC_s));
    block.trueExit = NULL;
    block.falseExit = NULL;
    return block;
}

void AddTAC(CFGBlock_s block[static 1], TACOp_e op, const char* dst, const char* src1, const char* src2)
{
    if (block->tacCapacity <= block->tacSize)
    {
        block->tacCapacity *= 2;
        block->tacArray = realloc(block->tacArray, sizeof(TAC_s) * block->tacCapacity);
    }
    block->tacArray[block->tacSize].op = op;
    block->tacArray[block->tacSize].dst = dst;
    block->tacArray[block->tacSize].src1 = src1;
    block->tacArray[block->tacSize].src2 = src2;
    block->tacSize++;
}

const char* RecGenerateGraph(CFGBlock_s* block[static 1], Node_s AST[static 1], uint32_t* id)
{
    char* buf = NULL;
    switch (AST->tacOp)
    {
    case intExp:
    case varExp:
        return AST->value;
    case trueExp:
        return "true";
    case falseExp:
        return "false";
    case thisExp:
        return "this";

    case andTac:
    case orTac:
    case greaterTac:
    case lesserTac:
    case equalTac:
    case addTac:
    case subTac:
    case mulTac:
    case divTac:
        buf = malloc(14);
        sprintf(buf,"__t%d", (*block)->tempCount++);
        AddTAC(*block, AST->tacOp, buf, RecGenerateGraph(block, AST->children[0], id), RecGenerateGraph(block, AST->children[1], id));
        return buf;

    case negTac:
        buf = malloc(14);
        sprintf(buf,"__t%d", (*block)->tempCount++);
        AddTAC(*block, AST->tacOp, buf, RecGenerateGraph(block, AST->children[0], id), "");
        return buf;

    case assignTac:
        AddTAC(*block, AST->tacOp, AST->value, RecGenerateGraph(block, AST->children[0], id), "");
        return "";

    default:
        break;
    }
    for (uint32_t i = 0; i < AST->size; i++)
        {
            RecGenerateGraph(block, AST->children[i], id);
        }
    return "";
}

void RecAddGraph(CFG_s graphs[static 1], Node_s AST[static 1], uint32_t* id, const char* className)
{
    if (AST->record == methodRecord)
    {
        if (graphs->size >= graphs->capacity)
        {
            graphs->capacity *= 2;
            graphs->graphList = realloc(graphs->graphList, sizeof(CFGBlock_s) * graphs->capacity);
            graphs->graphNames = realloc(graphs->graphNames, sizeof(char*) * graphs->capacity);
        }

        char*   blockName = NULL;
        const char* format = "%s_%s";
        int32_t nameSize = snprintf(NULL, 0, format, className, AST->value);
        if (nameSize < 0)
        {
            fprintf(stderr, "Unable to make block name (sprintf).");
            exit(1);
        }
        blockName = malloc(nameSize);
        sprintf(blockName, format, className, AST->value);

        graphs->graphNames[graphs->size] = blockName;
        graphs->graphList[graphs->size] = NewBlock(id);
        CFGBlock_s* blockPtr = &graphs->graphList[graphs->size];
        graphs->size++;

        for (uint32_t i = 0; i < AST->size; i++)
        {
            RecGenerateGraph(&blockPtr, AST->children[i], id);
        }
    }
    else if (AST->record == classRecord)
    {
        for (uint32_t i = 0; i < AST->size; i++)
        {
            RecAddGraph(graphs, AST->children[i], id, AST->value);
        }
    }
    else
    {
        for (uint32_t i = 0; i < AST->size; i++)
        {
            RecAddGraph(graphs, AST->children[i], id, className);
        }
    }
}

CFG_s GenerateControlFlowGraphs(Node_s* AST)
{
    uint32_t id = 0;
    CFG_s graphs;
    graphs.size = 0;
    graphs.capacity = 5;
    graphs.graphList = malloc(sizeof(CFGBlock_s) * graphs.capacity);
    graphs.graphNames = malloc(sizeof(char*) * graphs.capacity);
    for (uint32_t i = 0; i < AST->size; i++)
    {
        RecAddGraph(&graphs, AST->children[i], &id, "");
    }
    return graphs;
}

void WriteTAC(char* buf, uint32_t* bufsize, uint32_t* bufcap, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    
    uint32_t lineSize = vsnprintf(NULL, 0, format, args); 
    
    while (*bufcap <= ((*bufsize) + lineSize + 1))
    {
        *bufcap *= 2;
        buf = realloc((void*) buf, *bufcap);
    }

    va_start(args, format);
    vsprintf(buf + (*bufsize), format, args);
    *bufsize += lineSize;
}

static void generateVizContent(CFGBlock_s block[static 1], FILE file[static 1], const char* overrideName)
{
    const char* labelFormat = "n%d [shape=box label=\"%s\n\n%s\"];\n";
    const char* nnFormat    = "n%d -> n%d\n";

    uint32_t    lableCapacity  = 100;
    uint32_t    lableSize      = 0;
    char*       labelBuf       = malloc(lableCapacity);
    memset((void*) labelBuf, 0, lableCapacity);
    char blockName[17];
    
    sprintf(blockName, "Block_%d", block->identifier);
    
    for (uint32_t i = 0; i < block->tacSize; i++)
    {
        TAC_s* tac = &block->tacArray[i];
        switch (tac->op)
        {
            case andTac:
                WriteTAC(labelBuf, &lableSize, &lableCapacity, "%s := %s && %s\n", tac->dst, tac->src1, tac->src2);
                break;
            case orTac:
                WriteTAC(labelBuf, &lableSize, &lableCapacity, "%s := %s || %s\n", tac->dst, tac->src1, tac->src2);
                break;
            case greaterTac:
                WriteTAC(labelBuf, &lableSize, &lableCapacity, "%s := %s > %s\n", tac->dst, tac->src1, tac->src2);
                break;
            case lesserTac:
                WriteTAC(labelBuf, &lableSize, &lableCapacity, "%s := %s < %s\n", tac->dst, tac->src1, tac->src2);
                break;
            case equalTac:
                WriteTAC(labelBuf, &lableSize, &lableCapacity, "%s := %s == %s\n", tac->dst, tac->src1, tac->src2);
                break;
            case addTac:
                WriteTAC(labelBuf, &lableSize, &lableCapacity, "%s := %s + %s\n", tac->dst, tac->src1, tac->src2);
                break;
            case subTac:
                WriteTAC(labelBuf, &lableSize, &lableCapacity, "%s := %s - %s\n", tac->dst, tac->src1, tac->src2);
                break;
            case mulTac:
                WriteTAC(labelBuf, &lableSize, &lableCapacity, "%s := %s * %s\n", tac->dst, tac->src1, tac->src2);
                break;
            case divTac:
                WriteTAC(labelBuf, &lableSize, &lableCapacity, "%s := %s / %s\n", tac->dst, tac->src1, tac->src2);
                break;

            case negTac:
                WriteTAC(labelBuf, &lableSize, &lableCapacity, "%s := !%s\n", tac->dst, tac->src1);
                break;

            case assignTac:
                WriteTAC(labelBuf, &lableSize, &lableCapacity, "%s := %s\n", tac->dst, tac->src1);
                break;
            default:
                break;
        }
    }
    
    char*   writebuf = NULL;
    int32_t writeSize = snprintf(NULL, 0, labelFormat, block->identifier, (strcmp(overrideName, "") == 0) ? blockName : overrideName, labelBuf);
    
    if (writeSize < 0)
    {
        fprintf(stderr, "[-] Unable to generate tree (sprintf).");
        exit(1);
    }
    size_t bufCapacity = sizeof(char[writeSize]) * 2;
    writebuf           = malloc(bufCapacity);

    sprintf(writebuf, labelFormat, block->identifier, (strcmp(overrideName, "") == 0) ? blockName : overrideName, labelBuf);
    fwrite(writebuf, sizeof(*writebuf), writeSize, file);
    if (labelBuf)
        free(labelBuf);

    for (uint32_t i = 0; i < 2; ++i)
    {
        CFGBlock_s* blockPtr;
        if (i == 0)
            blockPtr = block->trueExit;
        else
            blockPtr = block->falseExit;
        if (blockPtr == NULL) continue;
        
        generateVizContent(blockPtr, file, "");
        
        writeSize = snprintf(
            NULL, 0, nnFormat, block->identifier, blockPtr->identifier);
        if (writeSize < 0)
        {
            fprintf(stderr, "[-] Unable to generate CFG viz, recursion\n");
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
        sprintf(writebuf, nnFormat, block->identifier, blockPtr->identifier);
        fwrite(writebuf, sizeof(*writebuf), writeSize, file);
    }
    if (writebuf)
        free(writebuf);
}

void CFGGenerateVisualization(CFG_s CFG[static 1])
{
    FILE* file;
    char* fname = "cfg.dot";
    file        = fopen(fname, "w");
    
    char    writebuf[] = "digraph {\n";
    fwrite(writebuf, sizeof(*writebuf), sizeof(writebuf) - 1, file);
    for (uint32_t i = 0; i < CFG->size; i++)
    {
        generateVizContent(&(CFG->graphList[i]), file, CFG->graphNames[i]);
    }
    fwrite("}", 1, 1, file);
    fclose(file);

    printf("\nBuilt a control flow graph vizualisation at %s. Use 'make cfg' to generate the pdf version.\n",
           fname);
}
