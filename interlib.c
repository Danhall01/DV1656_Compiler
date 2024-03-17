#include "interlib.h"
#include "stack.h"
#include "iarray.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

#define VERBAL_CMD 0

// List of all variables
static stack_s* stack;
enum errorTypes
{
    ERROR_NONE = 0,
    ERROR_INSTRUCTION_UNKNOWN,
    ERROR_INSTRUCTION_INVALID,
    ERROR_DIV_BY_ZERO,
    ERROR_METHOD_FAILED,


    // keep last
    ERROR_COUNT
};
#define RET_DONE -1

static char* _SplitStr(char* string, const char delim)
{
    char* ret = strchr(string, delim);
    if (ret == NULL)
        return "";
    *ret = '\0';
    return ++ret;
}

static int32_t _EvalParse(FILE bytecode[static 1]);
static int32_t _ParseInstruction(const char* cmd,
                                 char*       rhs,
                                 FILE        bytecode[static 1],
                                 iarray_s    variables[static 1])
{
    if (cmd == NULL)
        return ERROR_INSTRUCTION_INVALID;
#if VERBAL_CMD == 1
    printf("[d] cmd: \"%s\", rhs: \"%s\"\n", cmd, rhs);
#endif

    int32_t v1, v2;

    if (strcmp(cmd, "iload") == 0)
    {
        int32_t n = atoi(rhs + 1);
        Push(&stack, variables->arr[n]);
        return 0;
    }
    else if (strcmp(cmd, "iconst") == 0)
    {
        int32_t v = atoi(rhs);
        Push(&stack, v);
        return 0;
    }


    else if (strcmp(cmd, "istore") == 0)
    {
        int32_t var = atoi(rhs + 1);
        v1          = Pop(&stack);
        Insert(&variables, var, v1);
        return 0;
    }
    else if (strcmp(cmd, "iadd") == 0)
    {
        v1 = Pop(&stack);
        v2 = Pop(&stack);
        Push(&stack, v2 + v1);
        return 0;
    }
    else if (strcmp(cmd, "isub") == 0)
    {
        v1 = Pop(&stack);
        v2 = Pop(&stack);
        Push(&stack, v2 - v1);
        return 0;
    }
    else if (strcmp(cmd, "imul") == 0)
    {
        v1 = Pop(&stack);
        v2 = Pop(&stack);
        Push(&stack, v2 * v1);
        return 0;
    }
    else if (strcmp(cmd, "idiv") == 0)
    {
        v1 = Pop(&stack);
        v2 = Pop(&stack);
        if (v1 == 0)
            return ERROR_DIV_BY_ZERO;
        Push(&stack, v2 / v1);
        return 0;
    }

    else if (strcmp(cmd, "ilt") == 0)
    {
        v1 = Pop(&stack);
        v2 = Pop(&stack);
        Push(&stack, v2 < v1);
        return 0;
    }
    else if (strcmp(cmd, "iand") == 0)
    {
        v1 = Pop(&stack);
        v2 = Pop(&stack);
        Push(&stack, (v2 * v1) != 0);
        return 0;
    }
    else if (strcmp(cmd, "ior") == 0)
    {
        v1 = Pop(&stack);
        v2 = Pop(&stack);
        Push(&stack, (v2 + v1) != 0);
        return 0;
    }
    else if (strcmp(cmd, "inot") == 0)
    {
        v1 = Pop(&stack);
        Push(&stack, v1 == 0);
        return 0;
    }


    else if (strcmp(cmd, "goto") == 0)
    {
        int32_t i = atoi(rhs);
        fseek(bytecode, i, SEEK_SET);
        return 0;
    }
    else if (strcmp(cmd, "iffalse") == 0)
    {
        int32_t i = atoi(_SplitStr(rhs, ' '));
        if (!Pop(&stack))
            fseek(bytecode, i, SEEK_SET);
        return 0;
    }
    else if (strcmp(cmd, "invokevirtual") == 0)
    {
        int32_t callback = ftell(bytecode);

        // Invoke new method
        int32_t method = atoi(rhs);
        fseek(bytecode, method, SEEK_SET);
        int32_t eval = _EvalParse(bytecode);
        if (eval != 0)
            return eval;

        // Restore state
        fseek(bytecode, callback, SEEK_SET);
        return 0;
    }
    else if (strcmp(cmd, "ireturn") == 0)
    {
        return RET_DONE;
    }
    else if (strcmp(cmd, "print") == 0)
    {
        v1 = Pop(&stack);
        printf("%d\n", v1);
        return 0;
    }
    else if (strcmp(cmd, "stop") == 0)
    {
        return RET_DONE;
    }

    return ERROR_INSTRUCTION_UNKNOWN;
}


#define BUF_SIZE 256
static int32_t _EvalParse(FILE bytecode[static 1])
{
    iarray_s* variables = CreateArray();

    size_t  buffSize = BUF_SIZE;
    char*   buffer   = malloc(sizeof(char[BUF_SIZE]));
    int32_t readbytes;
    while ((readbytes = getline(&buffer, &buffSize, bytecode)) >= 0)
    {
        buffer[--readbytes] = '\0';
        int32_t ret = _ParseInstruction(buffer, _SplitStr(buffer, ' '), bytecode, variables);
        if (ret > 0)
        {
            fprintf(stderr, "[-]\tFailed parsing instruction \"%s\". (Stopping)\n", buffer);
            free(buffer);
            DestroyArray(&variables);
            return ret;
        }
        if (ret == RET_DONE)
            break;
    }

    free(buffer);
    DestroyArray(&variables);
    return 0;
}

int32_t Parse(FILE bytecode[static 1])
{
    stack = CreateStack();

    int32_t ret = _EvalParse(bytecode);
    if (ret != 0)
        printf("[-] File parsing exited with error code %d\n", ret);


    DestroyStack(&stack);
    return ret;
}
