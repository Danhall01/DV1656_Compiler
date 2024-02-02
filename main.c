#include <stdio.h>

#include "parser.tab.h"
#include "settings.h"
#include "Node.h"

extern FILE*   yyin;
extern int     lex_valid;
extern Node_s* root;


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
    if (yyparse() != 0 || !lex_valid)
        return 1;
    else
        printf("[+] Lexing finished successfully.\n");
    generateTree(root);
    fclose(yyin);
    return 0;
}
