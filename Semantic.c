#include "Semantic.h"
#include "SymbolTable.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
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
#define SEMANTIC_VERBAL_DEBUG

static uint32_t s_errCount = 0;

#ifdef SEMANTIC_VERBAL_DEBUG
static void PrintNode(Node_s* node)
{
    if (node == NULL)
        return;
    printf("Node:\n"
           "\tType: %s\n\tValue: %s\n\tRecord: %d\n\tLine %d\n\tCol %d\n",
           node->type,
           node->value,
           node->record,
           node->lineno,
           node->colno);
}
#endif

static Record_u* EntryExists(SymbolTable_s st[static 1], const char* target, int32_t* refc)
{
    Record_u* retScope = NULL;
    Record_u* oldScope = STCurrentScope(st);
    int32_t   refsum   = 0;

    // Check up until root
    while (1)
    {
        int32_t   refcount;
        Record_u* tscope = STLookUp(st, target, &refcount);
        refsum += refcount;
        if (tscope != NULL)
            retScope = tscope;

        if (st->currentScope == &(st->rootScope))
            break;
        STExitScope(st);
    }

    *refc = refsum;
    STSetScope(st, oldScope);
    return retScope;
}
static int32_t ValidateDeclare(Node_s node[static 1], SymbolTable_s st[static 1])
{
    if (node->record <= 0)
        return TREE_CONTINUE;

    // Check if type is the same (var to var / method to method) then check if it is non-first
    // instance to report.
    // line	col	res
    // f	f	F
    // f	t	T
    // t	f	T
    // t	t	T
    //
    int32_t   refc  = 0;
    Record_u* found = EntryExists(st, node->value, &refc);
    if (refc > 1 && found->Entry.record == node->record && found->Entry.lineno != node->lineno
        || found->Entry.colno != node->colno)
    {
        fprintf(
            stderr,
            "[w]\t@error at line %d: (Semantic) Re-definition, identifier \"%s\" already declared at line %d col %d\n",
            node->lineno,
            node->value,
            found->Entry.lineno,
            found->Entry.colno);
        ++s_errCount;
    }
    return TREE_CONTINUE;
}
static int32_t ValidateDeclUsage(Node_s node[static 1], SymbolTable_s st[static 1])
{
#ifdef SEMANTIC_VERBAL_DEBUG
    PrintNode(node);
#endif


    return TREE_CONTINUE;
}


// =======================
// Main invocation
static void SetScopePre(SymbolTable_s* st, Node_s* node)
{
    if (strcmp(node->type, "MAIN CLASS") == 0 || strcmp(node->type, "CLASS DECLARATION") == 0)
        STResetScope(st);

    if (strcmp(node->type, "METHOD DECLARATION") == 0)
    {
        STEnterScopeByLine(st, node->value, node->lineno);
        STExitScope(st);
    }
}
static void SetScopePost(SymbolTable_s* st, Node_s* node)
{
    if (strcmp(node->type, "MAIN CLASS") == 0 || strcmp(node->type, "CLASS DECLARATION") == 0)
        STEnterScopeByLine(st, node->value, node->lineno);

    if (strcmp(node->type, "METHOD DECLARATION") == 0)
        STEnterScopeByLine(st, node->value, node->lineno);
}
static int32_t scopeWrapper(int32_t (*action)(Node_s node[static 1], SymbolTable_s st[static 1]),
                            Node_s        node[static 1],
                            SymbolTable_s st[static 1])
{
    int32_t retval;
    SetScopePre(st, node);
    retval = action(node, st);
    SetScopePost(st, node);
    return retval;
}
static void ForEachNode(Node_s        ASTNode[static 1],
                        SymbolTable_s symbolTable[static 1],
                        int32_t (*action)(Node_s        node[static 1],
                                          SymbolTable_s symbolTable[static 1]))
{
    if (scopeWrapper(action, ASTNode, symbolTable) == 0)
        for (uint32_t i = 0; i < ASTNode->size; ++i)
            ForEachNode(ASTNode->children[i], symbolTable, action);
}
int32_t SemanticAnalysis(Node_s ASTRoot[static 1], SymbolTable_s symbolTable[static 1])
{
    STResetScope(symbolTable);

    ForEachNode(ASTRoot, symbolTable, ValidateDeclare);
    ForEachNode(ASTRoot, symbolTable, ValidateDeclUsage);
    // ForEachNode(ASTRoot, symbolTable, UsageCheck);

    printf("[+] Semantic Analysis finished with %d errors.\n", s_errCount);
    return s_errCount;
}
