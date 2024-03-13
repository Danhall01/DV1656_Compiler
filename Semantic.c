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

#define ERROR_FORMAT "[w]\t@error at line %d: "

#define TREE_CONTINUE 0
#define TREE_STOP 1
#define SEMANTIC_VERBAL_DEBUG

static uint32_t s_errCount = 0;


static uint32_t* s_errLines         = NULL;
static size_t    s_errLinesSize     = 0;
static size_t    s_errLinesCapacity = 0;
static void      InitErrLines()
{
    s_errLinesCapacity = 64;
    s_errLines         = malloc(sizeof(uint32_t[s_errLinesCapacity]));
    if (s_errLines == NULL)
        abort();
}
static void AppendErrLines(uint32_t value)
{
    if (s_errLinesSize >= s_errLinesCapacity - 1)
    {
        s_errLinesCapacity *= 1.4;
        s_errLines = realloc(s_errLines, sizeof(uint32_t[s_errLinesCapacity]));
        if (s_errLines == NULL)
            abort();
    }
    s_errLines[s_errLinesSize++] = value;
}
// Returns 0 if value is found in list
static int32_t InErrLines(uint32_t value)
{
    for (uint32_t i = 0; i < s_errLinesSize; ++i)
        if (s_errLines[i] == value)
            return 0;
    return 1;
}
static void DeInitErrLines() { free(s_errLines); }


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
static int32_t ScopeWrapper(int32_t (*action)(Node_s node[static 1], SymbolTable_s st[static 1]),
                            Node_s        node[static 1],
                            SymbolTable_s st[static 1])
{
    int32_t retval;
    SetScopePre(st, node);
    retval = action(node, st);
    SetScopePost(st, node);
    return retval;
}
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
// Checks if class exists (does not check for duplicates). Returns the first reference or NULL if
// not found.
static Record_u* ExistsClass(SymbolTable_s st[static 1], const char class[static 1])
{
    Record_u* retScope = NULL;
    Record_u* oldScope = STCurrentScope(st);

    STResetScope(st);
    retScope = STLookUp(st, class, NULL);

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
// Returns 0 on valid type, non-zero on invalid type
static int32_t IsValidType(SymbolTable_s st[static 1], const char type[static 1])
{
    // Check for basic types
    if (strcmp(type, "INTEGER") == 0 || strcmp(type, "BOOLEAN") == 0
        || strcmp(type, "INTEGER ARRAY") == 0 || strcmp(type, "STRING ARRAY") == 0)
        return 0;

    // Check for class type
    if (ExistsClass(st, type) != NULL)
        return 0;

    // Invalid type
    return 1;
}
static const char* GetClassFromThis(SymbolTable_s st[static 1])
{
    Record_u* oldScope = STCurrentScope(st);
    STExitScope(st);

    const char* ret = STCurrentScope(st)->Entry.name;
    if (st->currentScope == &(st->rootScope))
        ret = oldScope->Entry.name;

    STSetScope(st, oldScope);
    return ret;
}
static const char* GetVarType(SymbolTable_s st[static 1], Node_s node[static 1])
{
    const char* target = node->value;

    // Non-variable Type
    if (target[0] >= '0' && target[0] <= '9')
        return "INTEGER";
    if (strcmp(node->type, "FALSE") == 0 || strcmp(node->type, "TRUE") == 0)
        return "BOOLEAN";

    if (strcmp(node->type, "THIS") == 0)
        return GetClassFromThis(st);

    // Variable Type
    Record_u* retVal = ExistsEntry(st, target, NULL);
    if (retVal == NULL)
        return NULL;
    return retVal->Entry.type;
}

static const char* _ValidateArgs(Node_s cmdNode[static 1], const char* lhs, const char* rhs)
{
    const char* cmd = cmdNode->type;
    if (cmd == NULL)
        return NULL;
    // printf("cmd: %s, lhs: %s, rhs: %s\n", cmd, lhs, rhs);

    if (strcmp(cmd, "INDEXATION") == 0)
    {
        if (lhs && rhs && strstr(lhs, "ARRAY") != NULL && strcmp(rhs, "INTEGER") == 0)
        {
            if (strncmp(lhs, "INTEGER", sizeof("INTEGER") - 1) == 0)
                return "INTEGER";
            else
                return "STRING";
        }
        return NULL;
    }

    else if (strcmp(cmd, "ADDITION") == 0 || strcmp(cmd, "SUBTRACTION") == 0
             || strcmp(cmd, "MULTIPLICATION") == 0)
    {
        if (lhs && rhs && strcmp(lhs, "INTEGER") == 0 && strcmp(lhs, rhs) == 0)
            return lhs;
        return NULL;
    }

    else if (strcmp(cmd, "AND") == 0 || strcmp(cmd, "OR") == 0)
    {
        if (lhs && rhs && strcmp(lhs, "BOOLEAN") == 0 && strcmp(lhs, rhs) == 0)
            return lhs;
        return NULL;
    }
    else if (strcmp(cmd, "NEGATION") == 0)
    {
        if (lhs && strcmp(lhs, "BOOLEAN") == 0)
            return lhs;
        return NULL;
    }

    else if (strcmp(cmd, "LESS THAN") == 0 || strcmp(cmd, "GREATER THAN") == 0)
    {
        if (lhs && rhs && strcmp(lhs, "INTEGER") == 0 && strcmp(lhs, rhs) == 0)
            return "BOOLEAN";
        return NULL;
    }
    else if (strcmp(cmd, "EQUAL") == 0)
    {
        if (lhs && rhs && strcmp(lhs, rhs) == 0)
            return lhs;
        return NULL;
    }

    else if (strcmp(cmd, "LENGTH") == 0)
    {
        if (lhs && strstr(lhs, "ARRAY") != NULL)
            return "INTEGER";
        return NULL;
    }
    else if (strcmp(cmd, "ARRAY INSTANTIATION") == 0)
    {
        if (lhs && strcmp(lhs, "INTEGER") == 0)
            return "INTEGER ARRAY";
        return NULL;
    }

    printf("[-] Unknown cmd found %s\n", cmd);
    return NULL;
}
static int32_t     ParseParameters(SymbolTable_s st[static 1],
                                   Node_s        fcall[static 1],
                                   Record_u*     forigin);
static const char* _Eval(SymbolTable_s st[static 1], Node_s* node)
{
    if (node == NULL)
        return NULL;

    // debug_PrintNode(node);

    if (strcmp(node->type, "FUNCTION CALL") == 0)
    {
        const char* lhs_class = _Eval(st, node->children[0]);
        if (lhs_class == NULL)
            return NULL;
        Record_u* ret = ExistsMethod(st, node->value, lhs_class);

        if (ret == NULL || ParseParameters(st, node->children[1], ret) != 0)
            return NULL;

        return ret->Entry.type;
    }

    // Is split node
    if (node->size > 0)
    {
        const char* lhs = _Eval(st, node->children[0]);
        const char* rhs = node->size > 1 ? _Eval(st, node->children[1]) : NULL;
        const char* ret = _ValidateArgs(node, lhs, rhs);
        return ret;
    }

    // Is variable
    return GetVarType(st, node);
}
static int32_t ParseParameters(SymbolTable_s st[static 1], Node_s* fcall, Record_u* forigin)
{
    if (forigin == NULL || forigin->Entry.subScope == NULL)
        return 1;

    uint32_t paramc = forigin->Entry.subScope[0].Meta.paramc;
    if (fcall == NULL)
        return paramc > 0;
    if (fcall->size != paramc)
        return 1;

    for (uint32_t i = 0; i < paramc; ++i)
    {
        const char* paramType  = _Eval(st, fcall->children[i]);
        const char* originType = forigin->Entry.subScope[i + 1].Entry.type;
        if (paramType == NULL || originType == NULL || strcmp(paramType, originType) != 0)
        {
            // printf("Line %d, Got %s, expected %s\n", fcall->lineno, paramType, originType);
            return -1;
        }
    }
    return 0;
}
static int32_t EvaluateExpression(SymbolTable_s st[static 1], Node_s node[static 1])
{
    const char* lhs = GetVarType(st, node);
    const char* rhs = _Eval(st, node->children[0]);
    if (lhs && rhs)
        return strcmp(lhs, rhs);
    return 1;
}
static int32_t EvaluateExpressionIndexed(SymbolTable_s st[static 1], Node_s node[static 1])
{
    const char* index = _Eval(st, node->children[0]);
    if (strcmp(index, "INTEGER") != 0)
        return 1;

    const char* value = _Eval(st, node->children[1]);
    if (value == NULL)
        return 1;

    const char* this    = GetVarType(st, node);
    const char* thisArr = strstr(this, "ARRAY");
    if (thisArr == NULL)
        return 1;

    // If they are both INTEGER or STRING
    uint32_t chars = strlen(this) - strlen(thisArr) - 1;
    if (strncmp(this, value, chars) != 0)
        return 1;
    return 0;
}
static int32_t EvaluateExpressionBranched(SymbolTable_s st[static 1], Node_s node[static 1])
{
    // Only do leftmost child (It contains the logic, rest is handled by other catchers)
    const char* lhs = _Eval(st, node->children[0]);
    if (lhs == NULL)
        return 1;
    return 0;
}
static int32_t EvaluateReturnType(SymbolTable_s st[static 1], Node_s method[static 1])
{
    const char* lhs = method->children[0]->value;
    const char* rhs = NULL;

    if (strcmp(method->children[method->size - 1]->type, "RETURN EXPRESSION") == 0
        && method->children[method->size - 1]->size > 0)
    {
        if (strcmp(lhs, "VOID") == 0)
            return 1;
        // printf("================\n");
        // printf("LHS: %s\n", lhs);

        // Ensure scope is set to method, in case return uses local vars
        Record_u* oldScope = STCurrentScope(st);
        STEnterScope(st, method->value);
        rhs = _Eval(st, method->children[method->size - 1]->children[0]);
        STSetScope(st, oldScope);
        // printf("RHS: %s\n", rhs);

        if (lhs && rhs)
            return strcmp(lhs, rhs);
    }
    if (strcmp(lhs, "VOID") == 0)
        return 0;

    return 1;
}
// =================================================================


static int32_t ValidateDoubleDeclare(Node_s node[static 1], SymbolTable_s st[static 1])
{
    if (node->record <= 0)
        return TREE_CONTINUE;

    int32_t refc = 0;
    // Variable can exist as multiple instances, if one is parameter and one is class scope, it is
    // valid
    Record_u* found = STLookUp(st, node->value, &refc);

    // Check for non-first instance
    if (refc > 1 && found->Entry.record == node->record
        && (found->Entry.lineno != node->lineno || found->Entry.colno != node->colno))
    {
        fprintf(stderr,
                ERROR_FORMAT
                "(Semantic) Re-definition, identifier \"%s\" already declared at line %d col %d\n",
                node->lineno,
                node->value,
                found->Entry.lineno,
                found->Entry.colno);
        ++s_errCount;
    }
    return TREE_CONTINUE;
}

static int32_t ParseVarUsage(Node_s        node[static 1],
                             SymbolTable_s st[static 1],
                             size_t        declSize,
                             const char*   declList[static 1])
{
    if (node->record != 0 || IsVariable(node) != 0)
        return TREE_CONTINUE;

    // TODO: Maybe iterate over each function call to see if they are all valid
    if (strcmp(node->type, "FUNCTION CALL") == 0)
        return TREE_STOP;

    // Check if previously declared in scope
    for (uint32_t i = 0; i < declSize; ++i)
        if (strcmp(node->value, declList[i]) == 0)
            return TREE_CONTINUE;

    // Check if declared in scopes above (global or class-variable)
    Record_u* oldScope = STCurrentScope(st);
    STExitScope(st);
    if (ExistsEntry(st, node->value, NULL) == NULL)
    {
        fprintf(stderr,
                ERROR_FORMAT "(Semantic) Undeclared identifier \"%s\", line %d col %d\n",
                node->lineno,
                node->value,
                node->lineno,
                node->colno);
        ++s_errCount;
        AppendErrLines(node->lineno);
    }
    STSetScope(st, oldScope);
    return TREE_CONTINUE;
}
static int32_t ValidateVarUsage(Node_s node[static 1], SymbolTable_s st[static 1])
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
    return ParseVarUsage(node, st, declSize, declList);
}

static int32_t ValidateTypes(Node_s node[static 1], SymbolTable_s st[static 1])
{
    // Invalid declaration
    if (node->record == 4 && IsValidType(st, node->type) != 0)
    {
        fprintf(stderr,
                ERROR_FORMAT "(Semantic) Invalid type \"%s\", line %d, col %d\n",
                node->lineno,
                node->type,
                node->lineno,
                node->colno);
        ++s_errCount;
        return TREE_CONTINUE;
    }

    // Invalid Assignment / Arithmitics
    if (strcmp(node->type, "ASSIGN") == 0 && InErrLines(node->lineno) != 0
        && EvaluateExpression(st, node) != 0)
    {
        fprintf(stderr,
                ERROR_FORMAT "(Semantic), Type mismatch, line %d, col %d\n",
                node->lineno,
                node->lineno,
                node->colno);
        ++s_errCount;
        return TREE_STOP;
    }
    if (strcmp(node->type, "ASSIGN INDEX") == 0 && InErrLines(node->lineno) != 0
        && EvaluateExpressionIndexed(st, node) != 0)
    {
        fprintf(stderr,
                ERROR_FORMAT "(Semantic), Invalid array usage, line %d, col %d\n",
                node->lineno,
                node->lineno,
                node->colno);
        ++s_errCount;
        return TREE_STOP;
    }
    // Only check left child
    if (strcmp(node->type, "IF STATEMENT CLOSED") == 0 && InErrLines(node->lineno) != 0
        && EvaluateExpressionBranched(st, node) != 0)
    {
        fprintf(stderr,
                ERROR_FORMAT "(Semantic), Invalid branch condition, line %d, col %d\n",
                node->children[0]->lineno,
                node->children[0]->lineno,
                node->children[0]->colno);
        ++s_errCount;
        return TREE_STOP;
        return TREE_CONTINUE;
    }
    // WHILE COMMAND

    // Invalid Return Type
    if (strcmp(node->type, "METHOD DECLARATION") == 0 && EvaluateReturnType(st, node) != 0)
    {
        int32_t line = node->lineno;
        int32_t col  = node->colno;

        Record_u* oldScope = STCurrentScope(st);
        STEnterScope(st, node->value);
        Record_u* localVar = STLookUp(st, node->children[node->size - 1]->children[0]->value, NULL);

        if (localVar != NULL)
        {
            line = node->children[node->size - 1]->lineno - 1;
            col  = node->children[node->size - 1]->colno;
        }
        fprintf(stderr,
                ERROR_FORMAT "(Semantic), Return type mismatch, line %d, col %d\n",
                line,
                line,
                col);
        ++s_errCount;
        STSetScope(st, oldScope);
        return TREE_CONTINUE;
    }
    return TREE_CONTINUE;
}


// =======================
// Main invocation
static void ForEachNode(Node_s        ASTNode[static 1],
                        SymbolTable_s symbolTable[static 1],
                        int32_t (*action)(Node_s        node[static 1],
                                          SymbolTable_s symbolTable[static 1]))
{
    if (ScopeWrapper(action, ASTNode, symbolTable) == TREE_CONTINUE)
        for (uint32_t i = 0; i < ASTNode->size; ++i)
            ForEachNode(ASTNode->children[i], symbolTable, action);
}
int32_t SemanticAnalysis(Node_s ASTRoot[static 1], SymbolTable_s symbolTable[static 1])
{
    STResetScope(symbolTable);
    InitErrLines();

    ForEachNode(ASTRoot, symbolTable, ValidateDoubleDeclare);
    ForEachNode(ASTRoot, symbolTable, ValidateVarUsage);
    ForEachNode(ASTRoot, symbolTable, ValidateTypes);

    DeInitErrLines();
    if (s_errCount > 0)
        fprintf(stderr, "[-] Semantic Analysis finished with %d errors.\n", s_errCount);
    else
        printf("[+] Semantic Analysis finished successfully.\n");
    return s_errCount;
}
