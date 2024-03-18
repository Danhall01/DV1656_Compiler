#include "CFG.h"
#include "SymbolTable.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "Node.h"

CFGBlock_s NewBlock(uint32_t* id)
{
    CFGBlock_s block;
    block.identifier = (*id)++;
    
    block.tacCapacity = 5;
    block.tacSize = 0;
    block.tacArray = malloc(sizeof(TAC_s) * block.tacCapacity);
    block.tempCount = 0;
    
    block.blockCondition.op = assignTac;
    block.blockCondition.dst = "";
    block.blockCondition.src1 = "true";
    block.blockCondition.src2 = "";
    block.trueExit = NULL;
    block.falseExit = NULL;

    block.visited = 0;
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
    const char* temp = NULL;
    CFGBlock_s* savedBlock = NULL;
    CFGBlock_s* savedBlock2 = NULL;
    uint32_t count = 0;
    char* Nbuf = NULL;
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
    case indexTac:
        buf = malloc(14);
        sprintf(buf,"__t%d", (*block)->tempCount++);
        AddTAC(*block, AST->tacOp, buf, RecGenerateGraph(block, AST->children[0], id), RecGenerateGraph(block, AST->children[1], id));
        return buf;

    case negTac:
    case lengthTac:
    case newArrTac:
        buf = malloc(14);
        sprintf(buf,"__t%d", (*block)->tempCount++);
        AddTAC(*block, AST->tacOp, buf, RecGenerateGraph(block, AST->children[0], id), "");
        return buf;
    
    case newClassTac:
        buf = malloc(14);
        sprintf(buf,"__t%d", (*block)->tempCount++);
        AddTAC(*block, AST->tacOp, buf, AST->value, "");
        return buf;

    case assignTac:
        temp = RecGenerateGraph(block, AST->children[0], id);
        if(strncmp(temp, "__t", 3) == 0)
        {
            (*block)->tempCount--;
            (*block)->tacArray[(*block)->tacSize - 1].dst = AST->value;
        }
        else
        {
            AddTAC(*block, AST->tacOp, AST->value, temp, "");
        }
        return "";
    
    case assignArrTac:
        AddTAC(*block, AST->tacOp, AST->value, RecGenerateGraph(block, AST->children[0], id), RecGenerateGraph(block, AST->children[1], id));
        return "";

    case openIfTac:
        savedBlock = *block;
        temp = RecGenerateGraph(block, AST->children[0], id);
        if(strncmp(temp, "__t", 3) == 0)
        {
            savedBlock->tempCount--;
            savedBlock->tacArray[savedBlock->tacSize - 1].dst = "";

            savedBlock->blockCondition = savedBlock->tacArray[savedBlock->tacSize - 1];
            savedBlock->tacSize--;
        }
        else
        {
            savedBlock->blockCondition.src1 = temp;
        }
        
        savedBlock->trueExit = malloc(sizeof(CFGBlock_s));
        *(savedBlock->trueExit) = NewBlock(id);
        *block = savedBlock->trueExit;
        RecGenerateGraph(block, AST->children[1], id);

        (*block)->trueExit = malloc(sizeof(CFGBlock_s));
        *((*block)->trueExit) = NewBlock(id);

        savedBlock->falseExit = (*block)->trueExit;

        *block = (*block)->trueExit;
        return "";

    case closedIfTac:
        savedBlock = *block;
        temp = RecGenerateGraph(block, AST->children[0], id);
        if(strncmp(temp, "__t", 3) == 0)
        {
            savedBlock->tempCount--;
            savedBlock->tacArray[savedBlock->tacSize - 1].dst = "";

            savedBlock->blockCondition = savedBlock->tacArray[savedBlock->tacSize - 1];
            savedBlock->tacSize--;
        }
        else
        {
            savedBlock->blockCondition.src1 = temp;
        }
        
        savedBlock->trueExit = malloc(sizeof(CFGBlock_s));
        *(savedBlock->trueExit) = NewBlock(id);
        *block = savedBlock->trueExit;
        RecGenerateGraph(block, AST->children[1], id);

        (*block)->trueExit = malloc(sizeof(CFGBlock_s));
        *((*block)->trueExit) = NewBlock(id);
        savedBlock2 = (*block)->trueExit;

        savedBlock->falseExit = malloc(sizeof(CFGBlock_s));
        *(savedBlock->falseExit) = NewBlock(id);
        *block = savedBlock->falseExit;
        RecGenerateGraph(block, AST->children[2], id);

        (*block)->trueExit = savedBlock2;
        *block = savedBlock2;
        return "";

    case whileTac:
        (*block)->trueExit = malloc(sizeof(CFGBlock_s));
        *((*block)->trueExit) = NewBlock(id);
        *block = (*block)->trueExit;
        savedBlock = *block;
        temp = RecGenerateGraph(block, AST->children[0], id);
        if(strncmp(temp, "__t", 3) == 0)
        {
            savedBlock->tempCount--;
            savedBlock->tacArray[savedBlock->tacSize - 1].dst = "";

            savedBlock->blockCondition = savedBlock->tacArray[savedBlock->tacSize - 1];
            savedBlock->tacSize--;
        }
        else
        {
            savedBlock->blockCondition.src1 = temp;
        }
        
        savedBlock->trueExit = malloc(sizeof(CFGBlock_s));
        *(savedBlock->trueExit) = NewBlock(id);
        *block = savedBlock->trueExit;
        RecGenerateGraph(block, AST->children[1], id);

        (*block)->trueExit = savedBlock;

        savedBlock->falseExit = malloc(sizeof(CFGBlock_s));
        *(savedBlock->falseExit) = NewBlock(id);

        *block = savedBlock->falseExit;
        return "";

    case printTac:
    case returnTac:
        AddTAC(*block, AST->tacOp, "", RecGenerateGraph(block, AST->children[0], id), "");
        return "";

    case functionTac:
        temp = RecGenerateGraph(block, AST->children[0], id);
        if (AST->size > 1)
        {
            uint32_t cap = 5;
            const char** paramBuf = malloc(sizeof(const char*) * cap);
            for (count = 0; count < AST->children[1]->size; count++)
            {
                if (count >= cap) 
                {
                    cap *= 2;
                    paramBuf = realloc(paramBuf, cap);
                }
                
                paramBuf[count] = RecGenerateGraph(block, AST->children[1]->children[count], id);
            }

            AddTAC(*block, callerTac, "", temp, "");

            for (uint32_t i = 0; i < count; i++)
            {
                AddTAC(*block, paramTac, "", paramBuf[i], "");
            }
            free(paramBuf);
        }
        else
        {
            AddTAC(*block, callerTac, "", temp, "");
        }

        buf = malloc(14);
        sprintf(buf,"__t%d", (*block)->tempCount++);

        Nbuf = malloc(11);
        sprintf(Nbuf,"%d", count + 1);

        AddTAC(*block, AST->tacOp, buf, AST->value, Nbuf);
        return buf;

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
        const char* format = "%s-%s";
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

void WriteTAC(char** buf, uint32_t* bufsize, uint32_t* bufcap, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    
    uint32_t lineSize = vsnprintf(NULL, 0, format, args); 
    
    while (*bufcap <= ((*bufsize) + lineSize + 1))
    {
        *bufcap *= 2;
        *buf = realloc((void*) (*buf), *bufcap);
    }

    va_start(args, format);
    vsprintf((*buf) + (*bufsize), format, args);
    *bufsize += lineSize;
}

void TranslateTAC(char** labelBuf, uint32_t* lableSize, uint32_t* lableCapacity, TAC_s* tac)
{
    switch (tac->op)
    {
        case andTac:
            WriteTAC(labelBuf, lableSize, lableCapacity, "%s := %s && %s\n", tac->dst, tac->src1, tac->src2);
            break;
        case orTac:
            WriteTAC(labelBuf, lableSize, lableCapacity, "%s := %s || %s\n", tac->dst, tac->src1, tac->src2);
            break;
        case greaterTac:
            WriteTAC(labelBuf, lableSize, lableCapacity, "%s := %s > %s\n", tac->dst, tac->src1, tac->src2);
            break;
        case lesserTac:
            WriteTAC(labelBuf, lableSize, lableCapacity, "%s := %s < %s\n", tac->dst, tac->src1, tac->src2);
            break;
        case equalTac:
            WriteTAC(labelBuf, lableSize, lableCapacity, "%s := %s == %s\n", tac->dst, tac->src1, tac->src2);
            break;
        case addTac:
            WriteTAC(labelBuf, lableSize, lableCapacity, "%s := %s + %s\n", tac->dst, tac->src1, tac->src2);
            break;
        case subTac:
            WriteTAC(labelBuf, lableSize, lableCapacity, "%s := %s - %s\n", tac->dst, tac->src1, tac->src2);
            break;
        case mulTac:
            WriteTAC(labelBuf, lableSize, lableCapacity, "%s := %s * %s\n", tac->dst, tac->src1, tac->src2);
            break;
        case divTac:
            WriteTAC(labelBuf, lableSize, lableCapacity, "%s := %s / %s\n", tac->dst, tac->src1, tac->src2);
            break;
        case indexTac:
            WriteTAC(labelBuf, lableSize, lableCapacity, "%s := %s[%s]\n", tac->dst, tac->src1, tac->src2);
            break;

        case negTac:
            WriteTAC(labelBuf, lableSize, lableCapacity, "%s := !%s\n", tac->dst, tac->src1);
            break;
        case lengthTac:
            WriteTAC(labelBuf, lableSize, lableCapacity, "%s := length %s\n", tac->dst, tac->src1);
            break;
        case newArrTac:
            WriteTAC(labelBuf, lableSize, lableCapacity, "%s := new int[%s]\n", tac->dst, tac->src1);
            break;
            
        case newClassTac:
            WriteTAC(labelBuf, lableSize, lableCapacity, "%s := new %s\n", tac->dst, tac->src1);
            break;

        case assignTac:
            WriteTAC(labelBuf, lableSize, lableCapacity, "%s := %s\n", tac->dst, tac->src1);
            break;

        case assignArrTac:
            WriteTAC(labelBuf, lableSize, lableCapacity, "%s[%s] := %s\n", tac->dst, tac->src1, tac->src2);
            break;

        case printTac:
            WriteTAC(labelBuf, lableSize, lableCapacity, "print %s\n", tac->src1);
            break;

        case returnTac:
            WriteTAC(labelBuf, lableSize, lableCapacity, "return %s\n", tac->src1);
            break;

        case functionTac:
            WriteTAC(labelBuf, lableSize, lableCapacity, "%s := call %s, %s\n", tac->dst, tac->src1, tac->src2);
            break;
        case callerTac:
            WriteTAC(labelBuf, lableSize, lableCapacity, "caller %s\n", tac->src1);
            break;
        case paramTac:
            WriteTAC(labelBuf, lableSize, lableCapacity, "param %s\n", tac->src1);
            break;

        default:
            break;
    }
}

static void generateVizContent(CFGBlock_s block[static 1], FILE file[static 1], const char* overrideName)
{
    block->visited = 1;
    const char* labelFormat = "n%d [shape=box label=\"%s\n\n%s\nif %s\"];\n";
    const char* nnFormatTrue    = "n%d -> n%d [label=\" True\"];\n";
    const char* nnFormatFalse    = "n%d -> n%d [label=\" False\"];\n";

    uint32_t    lableCapacity  = 100;
    uint32_t    lableSize      = 0;
    char*       labelBuf       = malloc(lableCapacity);
    memset((void*) labelBuf, 0, lableCapacity);

    uint32_t    condCapacity  = 20;
    uint32_t    condSize      = 0;
    char*       condBuf       = malloc(condCapacity);
    memset((void*) condBuf, 0, condCapacity);

    char blockName[16];
    
    sprintf(blockName, "Block%d", block->identifier);
    
    for (uint32_t i = 0; i < block->tacSize; i++)
    {
        TranslateTAC(&labelBuf, &lableSize, &lableCapacity, &block->tacArray[i]);
    }

    TranslateTAC(&condBuf, &condSize, &condCapacity, &block->blockCondition);
    char* condStart = condBuf + 4;
    
    char*   writebuf = NULL;
    int32_t writeSize = snprintf(NULL, 0, labelFormat, block->identifier, (strcmp(overrideName, "") == 0) ? blockName : overrideName, labelBuf, condStart);
    
    if (writeSize < 0)
    {
        fprintf(stderr, "[-] Unable to generate tree (sprintf).");
        exit(1);
    }
    size_t bufCapacity = sizeof(char[writeSize]) * 2;
    writebuf           = malloc(bufCapacity);

    sprintf(writebuf, labelFormat, block->identifier, (strcmp(overrideName, "") == 0) ? blockName : overrideName, labelBuf, condStart);
    fwrite(writebuf, sizeof(*writebuf), writeSize, file);

    for (uint32_t i = 0; i < 2; ++i)
    {
        CFGBlock_s* blockPtr;
        const char* nnFormat;
        if (i == 0)
        {
            blockPtr = block->trueExit;
            nnFormat = nnFormatTrue;
        }
        else
        {
            blockPtr = block->falseExit;
            nnFormat = nnFormatFalse;
        }
        if (blockPtr == NULL) continue;
        
        if (blockPtr->visited == 0)
        {
            generateVizContent(blockPtr, file, "");
        }

        
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
    if (labelBuf)
        free(labelBuf);
    if (condBuf)
        free(condBuf);
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

void RecResetVisited(CFGBlock_s block[static 1])
{
    block->visited = 0;
    if (block->trueExit != NULL && (block->trueExit->visited == 1))
        RecResetVisited(block->trueExit);
    if (block->falseExit != NULL && (block->falseExit->visited == 1))
        RecResetVisited(block->falseExit);
}
void CFGResetVisited(CFG_s CFG[static 1])
{
    for (uint32_t i = 0; i < CFG->size; i ++)
        RecResetVisited(&CFG->graphList[i]);
}
