#include "Semantic.h"
#include "SymbolTable.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// Tree Traversals:
//
// 1. Declaration check (Variables / Methods)
//
// 2. Type Check (Statements / Operations / Method Return Type)
//
// 3. Usage check (Array / Method Params)
//

#define TREE_CONTINUE 0
#define TREE_STOP 1

static uint32_t  s_errCount = 0;
static Record_u* s_scope    = NULL;

/*
static void PrintNode(Node_s* node)
{
    if (node == NULL)
        return;
    printf("Node:\n"
           "\tType: %s\n\tValue: %s\n\tRecord: %d\n",
           node->type,
           node->value,
           node->record);
}
*/

static Record_u* PollMethodExists(SymbolTable_s* st, const char* method)
{
    Record_u* retScope = NULL;
    Record_u* oldScope = STCurrentScope(st);

    STExitScope(st);
    retScope = STLookUp(st, method, NULL);

    STSetScope(st, oldScope);
    return retScope;
}
static Record_u* PollClassFunc(SymbolTable_s* st, const char* class, const char* method)
{
    Record_u* retScope = NULL;
    Record_u* oldScope = STCurrentScope(st);

    STResetScope(st);
    STEnterScope(st, class);
    retScope = STLookUp(st, method, NULL);

    STSetScope(st, oldScope);
    return retScope;
}


static void SetScopePre(SymbolTable_s* st, Node_s* node)
{
    size_t len = strlen(node->type);

    if (strncmp(node->type, "MAIN CLASS", len) == 0
        || strncmp(node->type, "CLASS DECLARATION", len) == 0)
    {
        STResetScope(st);
        s_scope = STCurrentScope(st);
        STSetScope(st, s_scope);
    }
    if (strncmp(node->type, "METHOD DECLARATION", len) == 0)
    {
        STEnterScope(st, node->value);
        STExitScope(st);

        s_scope = STCurrentScope(st);
    }
}
static void SetScopePost(SymbolTable_s* st, Node_s* node)
{
    size_t len = strlen(node->type);
    if (strncmp(node->type, "MAIN CLASS", len) == 0
        || strncmp(node->type, "CLASS DECLARATION", len) == 0)
    {
        s_scope = STEnterScope(st, node->value);
    }
    if (strncmp(node->type, "METHOD DECLARATION", len) == 0)
    {
        s_scope = STEnterScope(st, node->value);
    }
}


static int32_t IsVariable(const char value[static 1])
{
    // value can be [0...9]|"true"|"false"|"void" - else it is variable
    size_t len = strlen(value);
    if (value[0] == '\0')
        return 1;

    if (value[0] >= '0' && value[0] <= '9')
        return 2;
    if (strncmp(value, "INTEGER", len) == 0)
        return 2;

    if (strncmp(value, "VOID", len) == 0)
        return 3;

    if (strncmp(value, "TRUE", len) == 0)
        return 4;
    if (strncmp(value, "FALSE", len) == 0)
        return 4;
    return 0;
}

static int32_t DeclarationCheck(Node_s node[static 1], SymbolTable_s st[static 1])
{
    // See if saving the node is required
    static Node_s* parent = NULL;
    if (strcmp(node->type, "CLASS INSTANTIATION") == 0)
    {
        Record_u* oldScope = STCurrentScope(st);
        STResetScope(st);
        if (STLookUp(st, node->value, NULL) == NULL)
        {
            fprintf(stderr, "[w] \t@error at line %d: (Semantic) Warning, referenced class \"%s\" not found.\n", node->lineno, node->value);
            ++s_errCount;
        }
        parent = node;
        STSetScope(st, oldScope);
        return TREE_CONTINUE;
    }

    if (IsVariable(node->value) != 0)
        return TREE_CONTINUE;

    // Declaration
    if (node->record > 0)
    {
        int32_t refc = 0;
        SetScopePre(st, node);
        if (STLookUp(st, node->value, &refc) == NULL)
        {
            fprintf(
                stderr,
                "[w] \t@error at line %d: (Semantic) Warning undefined, identifier \"%s\" does not exist in scope. Possible generation of invalid Symbol Table. This might cause UB in compilation\n",
                node->lineno,
                node->value);
            ++s_errCount;
        }
        if (refc > 1)
        {
            fprintf(
                stderr,
                "[w] \t@error at line %d: (Semantic) Warning redefinition, identifier \"%s\" already declared.\n",
                node->lineno,
                node->value);
            ++s_errCount;
        }
        SetScopePost(st, node);
        return TREE_CONTINUE;
    }


    // Check if function exists for caller
    if (strcmp(node->type, "FUNCTION CALL") == 0)
    {
        // If method is from current scope
        if (parent == NULL)
        {
            if (PollMethodExists(st, node->value) == NULL)
            {
                fprintf(stderr, "[w] \t@error at line %d: (Semantic) Warning, undefined reference to local method \"%s\".",
                       node->lineno,
                       node->value);
            }
            return TREE_CONTINUE;
        }

        // If method is from outside class
        if (PollClassFunc(st, parent->value, node->value) == NULL)
        {
            fprintf(stderr, "[w] \t@error at line %d: (Semantic) Warning, undefined reference to method \"%s\" in class \"%s\"\n",
                   node->lineno,
                   node->value,
                   parent->value);
            ++s_errCount;
        }

        // Reset the parent assuming that only a single call follows the declaration
        parent = NULL;
        return TREE_CONTINUE;
    }

    // Check if variable is declared before it is used
    if (STLookUp(st, node->value, NULL) == NULL)
    {
        fprintf(stderr, "[w] \t@error at line %d: (Semantic) Warning, Variable \"%s\" is not defined before use.\n",
               node->lineno,
               node->value);
        ++s_errCount;
    }
    return TREE_CONTINUE;
}
/*
static int32_t TypeCheck(Node_s node[static 1], SymbolTable_s st[static 1])
{
    //
    return TREE_CONTINUE;
}
static int32_t UsageCheck(Node_s node[static 1], SymbolTable_s st[static 1])
{
    //
    return TREE_CONTINUE;
}
*/

static void ForEachNode(Node_s        ASTNode[static 1],
                        SymbolTable_s symbolTable[static 1],
                        int32_t (*action)(Node_s        node[static 1],
                                          SymbolTable_s symbolTable[static 1]))
{
    if (action(ASTNode, symbolTable) == 0)
        for (uint32_t i = 0; i < ASTNode->size; ++i)
            ForEachNode(ASTNode->children[i], symbolTable, action);
}
int32_t SemanticAnalysis(Node_s ASTRoot[static 1], SymbolTable_s symbolTable[static 1])
{
    STResetScope(symbolTable);
    s_scope = STCurrentScope(symbolTable);

    ForEachNode(ASTRoot, symbolTable, DeclarationCheck);
    // ForEachNode(ASTRoot, symbolTable, TypeCheck);
    // ForEachNode(ASTRoot, symbolTable, UsageCheck);


    printf("[+] Semantic Analysis finished with %d errors.\n", s_errCount);
    return s_errCount;
}
