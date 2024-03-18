#include <stdio.h>
#include <stdlib.h>

#include "Semantic.h"
#include "parser.tab.h"
#include "settings.h"
#include "Node.h"
#include "SymbolTable.h"
#include "CFG.h"
#include "BytecodeGeneration.h"

extern FILE* yyin;

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
    if (yyparse() != 0)
        return 1;
    else
        printf("[+] Parser finished without error\n");

    if (GENERATE_PARSER_TREE == 1)
        generateTree(root);

    SymbolTable_s ST = GenerateSymboltable(root);
    if (GENERATE_SYMBOL_TABLE_TREE == 1)
        STGenerateVisualization(&ST);

    if (SemanticAnalysis(root, &ST) != 0)
    {
        fclose(yyin);
        return 0;
    }
    printf("Semantic Done.\n");

    CFG_s CFG = GenerateControlFlowGraphs(root);
    if (GENERATE_CONTROL_FLOW_GRAPH_TREE == 1)
    {
        CFGGenerateVisualization(&CFG);
        CFGResetVisited(&CFG);
    }

    const char* output = argc > 2 ? argv[2] : "program.runnable";
    GenerateJavaBytecode(output, &CFG, &ST);

    fclose(yyin);
    return 0;
}
