#include "interlib.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int main(int32_t argc, char* argv[static argc])
{
    if (argc < 2)
        (fprintf(stderr, "[-]\tNo target file given. (Stopping)\n"), exit(1));

    FILE* bytecode = fopen(argv[1], "r");
    if (bytecode == NULL)
        (fprintf(stderr, "[-]\tInvalid filename. (Stopping)\n"), exit(1));

    if (Parse(bytecode) != 0)
        (fprintf(stderr,
                 "[-]\tInterpreter failed to parse file \"%s\" (see logs above for details).\n",
                 argv[1]),
         exit(1));

    printf("[+]\tInterpreter successfully parsed file.\n");
    return 0;
}
