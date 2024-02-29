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
// #define SEMANTIC_VERBAL_DEBUG

static uint32_t s_errCount = 0;

static void debug_PrintNode(Node_s* node)
{
#ifdef SEMANTIC_VERBAL_DEBUG
    if (node == NULL)
        return;
    printf("Node:\n"
           "\tType: %s\n\tValue: %s\n\tRecord: %d\n\tLine %d\n\tCol %d\n",
           node->type,
           node->value,
           node->record,
           node->lineno,
           node->colno);
#endif
}


// Helper Functions
// =================================================================
// Checks if entry exists in current scope or in up-scope, refc is the amount that exists. Returns
// the first instance in the highest scope, or NULL if not found.
static Record_u* ExistsEntry(SymbolTable_s st[static 1], const char target[static 1], int32_t* refc)
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
    if (refc != NULL)
        *refc = refsum;
    STSetScope(st, oldScope);
    return retScope;
}
// Checks if method exists in given class. Returns the first reference or NULL if not found.
static Record_u* ExistsMethod(SymbolTable_s st[static 1],
                              const char    method[static 1],
                              const char class[static 1])
{
    Record_u* retScope = NULL;
    Record_u* oldScope = STCurrentScope(st);

    STResetScope(st);
    STEnterScope(st, class);
    retScope = STLookUp(st, method, NULL);

    STSetScope(st, oldScope);
    return retScope;
}
// Checks if the value in the given node refers to a variable type
static int32_t IsVariable(Node_s node[static 1])
{
    // value can be [0...9]|"true"|"false"|"void" - else it is variable
    if (node->value[0] == '\0')
        return 1;

    if (node->value[0] >= '0' && node->value[0] <= '9')
        return 2;

    if (strcmp(node->type, "RETURN TYPE") == 0)
        return 6;


    if (strcmp(node->value, "INTEGER") == 0)
        return 2;

    if (strcmp(node->value, "VOID") == 0)
        return 3;

    if (strcmp(node->value, "BOOLEAN") == 0)
        return 4;
    if (strcmp(node->value, "TRUE") == 0)
        return 4;
    if (strcmp(node->value, "FALSE") == 0)
        return 4;

    if (strcmp(node->value, "INTEGER ARRAY") == 0)
        return 5;
    return 0;
}
// =================================================================


static int32_t ValidateDoubleDeclare(Node_s node[static 1], SymbolTable_s st[static 1])
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
    Record_u* found = ExistsEntry(st, node->value, &refc);
    if (refc > 1 && found->Entry.record == node->record
        && (found->Entry.lineno != node->lineno || found->Entry.colno != node->colno))
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

static int32_t ParseDeclUsage(Node_s        node[static 1],
                              SymbolTable_s st[static 1],
                              size_t        declSize,
                              const char*   declList[static 1])
{
    if (node->record != 0 || IsVariable(node) != 0)
        return TREE_CONTINUE;

    int16_t stop = 0;
    for (uint32_t i = 0; i < node->size; ++i)
    {
        if (strcmp(node->children[i]->type, "FUNCTION CALL") == 0)
        {
            stop = 1;

            Record_u* classref = ExistsEntry(st, node->value, NULL);
            if (classref == NULL)
            {
                fprintf(
                    stderr,
                    "[w]\t@error at line %d: (Semantic) Unknown class reference \"%s\", line %d col %d\n",
                    node->lineno,
                    node->value,
                    node->lineno,
                    node->colno);
                ++s_errCount;
                continue;
            }
            if (ExistsMethod(st, node->children[i]->value, classref->Entry.type) == NULL)
            {
                fprintf(
                    stderr,
                    "[w]\t@error at line %d: (Semantic) Undeclared method call \"%s\" in class \"%s\", line %d col %d\n",
                    node->lineno,
                    node->children[i]->value,
                    classref->Entry.type,
                    node->lineno,
                    node->colno);
                ++s_errCount;
            }
            continue;
        }
        break;
    }
    if (stop)
        return TREE_STOP;

    for (uint32_t i = 0; i < declSize; ++i)
        if (strcmp(node->value, declList[i]) == 0)
            return TREE_CONTINUE;

    Record_u* oldScope = STCurrentScope(st);
    STExitScope(st);
    if (ExistsEntry(st, node->value, NULL) == NULL)
    {
        fprintf(stderr,
                "[w]\t@error at line %d: (Semantic) Undeclared identifier \"%s\", line %d col %d\n",
                node->lineno,
                node->value,
                node->lineno,
                node->colno);
        ++s_errCount;
    }
    STSetScope(st, oldScope);

    return TREE_CONTINUE;
}
static int32_t ValidateDeclUsage(Node_s node[static 1], SymbolTable_s st[static 1])
{
    static size_t       declSize     = 0;
    static size_t       declCapacity = 0;
    static const char** declList     = NULL;
    static int32_t      paramcount   = 0;
    if (declCapacity == 0)
    {
        declCapacity = 10;
        declList     = malloc(declCapacity * sizeof(char*));
    }
    // Variables declared under this tag exists in Symbol Table up-scope
    if (strcmp(node->type, "VARIABLES") == 0)
        return TREE_STOP;

    // Reduce
    if (strcmp(node->type, "CLASS DECLARATION") == 0)
    {
        declSize   = 0;
        paramcount = 0;
    }
    if (strcmp(node->type, "METHOD DECLARATION") == 0)
    {
        declSize -= paramcount;
        paramcount = 0;
    }

    // Append
    if (strcmp(node->type, "PARAMETERS") == 0)
    {
        paramcount = node->size;
        if (declSize + paramcount >= declCapacity)
        {
            declCapacity = paramcount + declCapacity * 1.4;
            declList     = realloc(declList, declCapacity * sizeof(char*));
            if (declList == NULL)
                abort();
        }
        for (int32_t i = 0; i < paramcount; ++i)
            declList[declSize++] = node->children[i]->value;
        return TREE_STOP;
    }
    if (strcmp(node->type, "VARIABLE DECLARATION") == 0)
    {
        if (declSize + 1 >= declCapacity)
        {
            declCapacity *= 1.4;
            declList = realloc(declList, declCapacity * sizeof(char*));
            if (declList == NULL)
                abort();
        }
        declList[declSize++] = node->children[0]->value;
        return TREE_STOP;
    }
    return ParseDeclUsage(node, st, declSize, declList);
}

static int32_t ValidateTypes(Node_s node[static 1], SymbolTable_s st[static 1])
{
    //
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
    if (scopeWrapper(action, ASTNode, symbolTable) == TREE_CONTINUE)
        for (uint32_t i = 0; i < ASTNode->size; ++i)
            ForEachNode(ASTNode->children[i], symbolTable, action);
}
int32_t SemanticAnalysis(Node_s ASTRoot[static 1], SymbolTable_s symbolTable[static 1])
{
    STResetScope(symbolTable);

    ForEachNode(ASTRoot, symbolTable, ValidateDoubleDeclare);
    ForEachNode(ASTRoot, symbolTable, ValidateDeclUsage);
    ForEachNode(ASTRoot, symbolTable, ValidateTypes);

    printf("[+] Semantic Analysis finished with %d errors.\n", s_errCount);
    return s_errCount;
}
