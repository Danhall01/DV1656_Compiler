#include <stdio.h>

#include "parser.tab.h"
#include "settings.h"

extern int   count;
extern int   yylex(void);
extern FILE* yyin;
extern Node *root;
extern int yylineno;


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
    yylex();
    printf("\nNumber of Capital letters "
           "in the given input - %d\n",
           count);

    return 0;
}
