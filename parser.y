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
%left '='
%left OR
%left AND
%left EQUAL
%left '>' '<'
%left '-' '+'
%left '*' '/'
%left '!'

%start Goal

%type <nodeval> Identifier RecParamExp Operator Expression RecStatement Statement Type VarDeclaration RecParameter Parameters RecMethodBody MethodDeclaration RecVarDecl RecMethodDecl ClassDeclaration RecMain MainClass RecClassDecl Goal

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
        | Identifier '=' Expression ';'                           { $$ = initNodeTree("ASSIGN", "", yylineno); 
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
Operator:
        AND    { $$ = initNodeTree("AND", "", yylineno); }
      | OR     { $$ = initNodeTree("OR", "", yylineno); }
      | '<'    { $$ = initNodeTree("LESS THAN", "", yylineno); }
      | '>'    { $$ = initNodeTree("GREATER THAN", "", yylineno); }
      | EQUAL  { $$ = initNodeTree("EQUALITY", "", yylineno); }
      | '+'    { $$ = initNodeTree("ADDITION", "", yylineno); }
      | '-'    { $$ = initNodeTree("SUBTRACTION", "", yylineno); }
      | '*'    { $$ = initNodeTree("MULTIPLICATION", "", yylineno); }
      | '/'    { $$ = initNodeTree("SUBDIVISION", "", yylineno); }
;
Expression:
        Expression Operator Expression                            { $$ = $2;
                                                                    addSubTree($$, $1);
                                                                    addSubTree($$, $3); }
      | Expression '[' Expression ']'                             { $$ = initNodeTree("INDEXATION", "", yylineno); 
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
      | INTEGER_LITERAL                                           { $$ = initNodeTree("INTEGER LITERAL", yylval.strval, yylineno); }
      | TRUE                                                      { $$ = initNodeTree("TRUE", "", yylineno); }
      | FALSE                                                     { $$ = initNodeTree("FALSE", "", yylineno); }
      | Identifier                                                { $$ = $1; }
      | THIS                                                      { $$ = initNodeTree("THIS", "", yylineno); }
      | NEW INT '[' Expression ']'                                { $$ = initNodeTree("ARRAY INSTANTIATION", "", yylineno); 
                                                                    addSubTree($$, $4); }
      | NEW Identifier '(' ')'                                    { $$ = initNodeTree("CLASS INSTANTIATION", "", yylineno); 
                                                                    addSubTree($$, $2); }
      //| '!' Expression                                            { $$ = initNodeTree("NEGATION", "", yylineno); 
      //                                                              addSubTree($$, $2); }
      | '(' Expression ')'                                        { $$ = initNodeTree("PARENTHESES", "", yylineno); 
                                                                    addSubTree($$, $2); }
;

Identifier:
      IDENTIFIER { $$ = initNodeTree("IDENTIFIER", yylval.strval, yylineno); }
;
/* End of grammar */
%%
