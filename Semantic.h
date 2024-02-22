#ifndef _SEMANTIC_H_
#define _SEMANTIC_H_

#include "settings.h"
#include "Node.h"
#include "SymbolTable.h"
#include <stdint.h>

// Perform a semantic analysis and prints every error to stderr
int32_t SemanticAnalysis(Node_s ASTRoot[static 1], SymbolTable_s symbolTable[static 1]);


#endif /* _SEMANTIC_H_ */
