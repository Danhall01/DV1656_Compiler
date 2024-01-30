%defines
%define parse.error verbose

%code requires{
#include "settings.h"
#include "Node.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
} 

%code{
Node_s* root;
extern int yylineno;
extern int yylex(void);
void yyerror (char const * s)  /* Called by yyparse on error */
{
  printf ("%s\n", s);
}
}

%union {
  int intval;
  const char* strval;
  Node_s* nodeval;
}

%token <intval> PUBLIC CLASS STATIC VOID MAIN STRING RETURN INT BOOL IF ELSE WHILE PRINTLN AND OR EQUAL LENGTH TRUE FALSE THIS NEW INTEGER_LITERAL IDENTIFIER

%right then ELSE
%right '='
%left '-' '+'
%left '*' '/'

%start Goal

%type <nodeval> Identifier RecParamExp Operators Expression RecStatement Statement Type VarDeclaration RecParameter Parameters RecMethodBody MethodDeclaration RecVarDecl RecMethodDecl ClassDeclaration RecMain MainClass RecClassDecl Goal

%%
RecClassDecl:
          /* empty */                   { $$ = NULL; }
        | RecClassDecl ClassDeclaration { $$ = $2; 
                                          addSubTree($$, $1); }
;
Goal:     
          MainClass RecClassDecl { $$ = root = initNodeTree("ROOT", "", yylineno); 
                                        addSubTree($$, $1);
                                        addSubTree($$, $2); }
;

RecMain:
          Statement         { $$ = $1; }
        | RecMain Statement { $$ = $2; 
                              addSubTree($$, $1); }
;
MainClass:
          PUBLIC CLASS Identifier 
          '{' 
            PUBLIC STATIC VOID MAIN '(' STRING '[' ']' Identifier ')' 
            '{'  
              RecMain 
            '}' 
          '}'                                                           { $$ = initNodeTree("MAIN CLASS", "", yylineno); 
                                                                          addSubTree($$, $3);
                                                                          addSubTree($$, $13);
                                                                          addSubTree($$, $16); }
;

RecVarDecl:
          /* empty */               { $$ = NULL; }
        | RecVarDecl VarDeclaration { $$ = $2; 
                                    addSubTree($$, $1); }
;
RecMethodDecl:
          /* empty */                     { $$ = NULL; }
        | RecMethodDecl MethodDeclaration { $$ = $2; 
                                            addSubTree($$, $1); }
;
ClassDeclaration:
          CLASS Identifier
          '{' 
              RecVarDecl
              RecMethodDecl
          '}'                 { $$ = initNodeTree("CLASS DECLARATION", "", yylineno); 
                                addSubTree($$, $2);
                                addSubTree($$, $4);
                                addSubTree($$, $5); }
;

RecParameter:
          /* empty */                       { $$ = NULL; }
        | RecParameter ',' Type Identifier  { $$ = initNodeTree("REC PARAMETER", "", yylineno); 
                                              addSubTree($$, $1);
                                              addSubTree($$, $3);
                                              addSubTree($$, $4); }
;
Parameters:
          /* empty */                   { $$ = NULL; }
        | Type Identifier RecParameter  { $$ = initNodeTree("METHOD PARAMETER", "", yylineno); 
                                          addSubTree($$, $1);
                                          addSubTree($$, $2);
                                          addSubTree($$, $3); }
;
RecMethodBody:
          /* empty */                   { $$ = NULL; }
        | RecMethodBody VarDeclaration  { $$ = initNodeTree("METHOD BODY", "", yylineno); 
                                          addSubTree($$, $1);
                                          addSubTree($$, $2); }
        | RecMethodBody Statement       { $$ = initNodeTree("METHOD BODY", "", yylineno); 
                                          addSubTree($$, $1);
                                          addSubTree($$, $2); }
;
MethodDeclaration:
          PUBLIC Type Identifier '(' Parameters ')'
          '{'
            RecMethodBody
            RETURN Expression ';'
          '}'                                       { $$ = initNodeTree("METHOD DECLARATION", "", yylineno); 
                                                      addSubTree($$, $2);
                                                      addSubTree($$, $3);
                                                      addSubTree($$, $5);
                                                      addSubTree($$, $8);
                                                      addSubTree($$, $10); }
;

VarDeclaration:
          Type Identifier ';' { $$ = initNodeTree("VARIABLE DECLARATION", "", yylineno); 
                                addSubTree($$, $1);
                                addSubTree($$, $2); }
;

Type:
          INT '[' ']' { $$ = initNodeTree("INTEGER ARRAY", "", yylineno); }
        | BOOL        { $$ = initNodeTree("BOOLEAN", "", yylineno); }
        | INT         { $$ = initNodeTree("INTEGER", "", yylineno); }
        | Identifier  { $$ = $1; }
;

RecStatement:
          /* empty */             { $$ = NULL; }
        | RecStatement Statement  { $$ = $2; 
                                    addSubTree($$, $1); }
;
Statement:
          '{' RecStatement '}'                                    { $$ = initNodeTree("CODE BLOCK", "", yylineno); 
                                                                    addSubTree($$, $2); }
        | IF '(' Expression ')' Statement              %prec then { $$ = initNodeTree("IF STATEMENT OPEN", "", yylineno); 
                                                                    addSubTree($$, $3);
                                                                    addSubTree($$, $5); }
        | IF '(' Expression ')' Statement ELSE Statement          { $$ = initNodeTree("IF STATEMENT CLOSED", "", yylineno); 
                                                                    addSubTree($$, $3);
                                                                    addSubTree($$, $5);
                                                                    addSubTree($$, $7); }
        | WHILE '(' Expression ')' Statement                      { $$ = initNodeTree("WHILE LOOP", "", yylineno); 
                                                                    addSubTree($$, $3);
                                                                    addSubTree($$, $5); }
        | PRINTLN '(' Expression ')' ';'                          { $$ = initNodeTree("PRINT LINE", "", yylineno); 
                                                                    addSubTree($$, $3); }
        | Identifier '=' Expression ';'                           { $$ = initNodeTree("SET AS", "", yylineno); 
                                                                    addSubTree($$, $1);
                                                                    addSubTree($$, $3); }
        | Identifier '[' Expression ']' '=' Expression ';'        { $$ = initNodeTree("SET AS AT INDEX", "", yylineno); 
                                                                    addSubTree($$, $1);
                                                                    addSubTree($$, $3);
                                                                    addSubTree($$, $6); }
;

RecParamExp:
        /* empty */                 { $$ = NULL; }
      | RecParamExp ',' Expression  { $$ = initNodeTree("EXP REC PARAM", "", yylineno); 
                                      addSubTree($$, $1);
                                      addSubTree($$, $3); }
;
Operators:
        /* empty */       { $$ = NULL; }
      | Expression AND    { $$ = initNodeTree("AND", "", yylineno); 
                            addSubTree($$, $1); }
      | Expression OR     { $$ = initNodeTree("OR", "", yylineno); 
                            addSubTree($$, $1); }
      | Expression '<'    { $$ = initNodeTree("LESS THAN", "", yylineno); 
                            addSubTree($$, $1); }
      | Expression '>'    { $$ = initNodeTree("GREATER THAN", "", yylineno); 
                            addSubTree($$, $1); }
      | Expression EQUAL  { $$ = initNodeTree("EQUALITY", "", yylineno); 
                            addSubTree($$, $1); }
      | Expression '+'     { $$ = initNodeTree("ADDITION", "", yylineno); 
                            addSubTree($$, $1); }
      | Expression '-'    { $$ = initNodeTree("SUBTRACTION", "", yylineno); 
                            addSubTree($$, $1); }
      | Expression '*'    { $$ = initNodeTree("MULTIPLICATION", "", yylineno); 
                            addSubTree($$, $1); }
      | Expression '/'    { $$ = initNodeTree("SUBDIVISION", "", yylineno); 
                            addSubTree($$, $1); }
      | Operators '!'     { $$ = initNodeTree("NEGATION", "", yylineno); 
                            addSubTree($$, $1); }
;
Expression:
        Expression '[' Expression ']'                             { $$ = initNodeTree("INDEXATION", "", yylineno); 
                                                                    addSubTree($$, $1);
                                                                    addSubTree($$, $3); }
      | Expression '.' LENGTH                                     { $$ = initNodeTree("LENGTH", "", yylineno); 
                                                                    addSubTree($$, $1); }
      | Expression '.' Identifier '(' ')'                         { $$ = initNodeTree("FUNCTION CALL", "", yylineno); 
                                                                    addSubTree($$, $1);
                                                                    addSubTree($$, $3); }
      | Expression '.' Identifier '(' Expression RecParamExp ')'  { $$ = initNodeTree("FUNCTION CALL", "", yylineno); 
                                                                    addSubTree($$, $1);
                                                                    addSubTree($$, $3);
                                                                    addSubTree($$, $5);
                                                                    addSubTree($$, $6); }
      | Operators INTEGER_LITERAL                                 { $$ = initNodeTree("INTEGER LITERAL", yylval.strval, yylineno);
                                                                    addSubTree($$, $1); }
      | Operators TRUE                                            { $$ = initNodeTree("TRUE", "", yylineno); 
                                                                    addSubTree($$, $1); }
      | Operators FALSE                                           { $$ = initNodeTree("FALSE", "", yylineno); 
                                                                    addSubTree($$, $1); }
      | Operators Identifier                                      { $$ = $2;
                                                                    addSubTree($$, $1); }
      | Operators THIS                                            { $$ = initNodeTree("THIS", "", yylineno); 
                                                                    addSubTree($$, $1); }
      | Operators NEW INT '[' Expression ']'                      { $$ = initNodeTree("ARRAY INSTANTIATION", "", yylineno); 
                                                                    addSubTree($$, $1);
                                                                    addSubTree($$, $5); }
      | Operators NEW Identifier '(' ')'                          { $$ = initNodeTree("CLASS INSTANTIATION", "", yylineno); 
                                                                    addSubTree($$, $1);
                                                                    addSubTree($$, $3); }
      | Operators '(' Expression ')'                              { $$ = initNodeTree("PARENTHESES", "", yylineno); 
                                                                    addSubTree($$, $1);
                                                                    addSubTree($$, $3); }
;

Identifier:
      IDENTIFIER { $$ = initNodeTree("IDENTIFIER", yylval.strval, yylineno); }
;
/* End of grammar */
%%
