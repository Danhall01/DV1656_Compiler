#include <stdio.h>

#include "parser.tab.h"
#include "settings.h"

extern int   count;
extern FILE* yyin;


int main(int argc, char** argv)
{
    // Reads from file if a file name is passed as an argument. Otherwise, reads from stdin.
    if (argc > 1)
    {
        if (!(yyin = fopen(argv[1], "r")))
        {
            perror(argv[1]);
            return 1;
        }
    }
    if (yyparse() == 0)
    {

    }
    else
    {
        
    }
    printf("\nNumber of Capital letters "
           "in the given input - %d\n",
           count);

    return 0;
}
