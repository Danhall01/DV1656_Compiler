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
%left '.' '['
%left '('

%start Goal

%type <nodeval> ParametersExp RecParamExp Expression RecStatement Statement VarDeclaration RecParameter Parameters RecMethodBody MethodDeclaration RecVarDecl RecMethodDecl ClassDeclaration RecMain MainClass RecClassDecl Goal VariableDeclarations MethodDeclarations Classes
%type <strval> Identifier Type

%%
RecClassDecl:
          ClassDeclaration              { $$ = initNodeTree("CLASSES", "", yylineno); 
                                          addSubTree($$, $1); }
        | RecClassDecl ClassDeclaration { $$ = $1; 
                                          addSubTree($$, $2); }
;
Classes:
          /* empty */                   { $$ = NULL; }
        | RecClassDecl                  { $$ = $1; }
;
Goal:     
          MainClass Classes { $$ = root = initNodeTree("ROOT", "", yylineno); 
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
          '}'                                                           { $$ = initNodeTree("MAIN CLASS", $3, yylineno); 
                                                                          addSubTree($$, initNodeTree("INPUT", $13, yylineno));
                                                                          addSubTree($$, $16); }
;

RecVarDecl:
          VarDeclaration            { $$ = initNodeTree("VARIABLES", "", yylineno); 
                                      addSubTree($$, $1); }
        | RecVarDecl VarDeclaration { $$ = $1; 
                                      addSubTree($$, $2); }
;
VariableDeclarations:
          /* empty */ { $$ = NULL; }
        | RecVarDecl  { $$ = $1; }
;
RecMethodDecl:
          MethodDeclaration               { $$ = initNodeTree("METHODS", "", yylineno); 
                                            addSubTree($$, $1); }
        | RecMethodDecl MethodDeclaration { $$ = $1; 
                                            addSubTree($$, $2); }
;
MethodDeclarations:
          /* empty */   { $$ = NULL; }
        | RecMethodDecl { $$ = $1; }
;
ClassDeclaration:
          CLASS Identifier
          '{' 
              VariableDeclarations
              MethodDeclarations
          '}'                 { $$ = initNodeTree("CLASS DECLARATION", $2, yylineno); 
                                addSubTree($$, $4);
                                addSubTree($$, $5); }
;

RecParameter:
          Type Identifier                   { $$ = initNodeTree("PARAMETERS", "", yylineno); 
                                              Node_s* node = initNodeTree($2, $1, yylineno);
                                              addSubTree($$, node); }
        | RecParameter ',' Type Identifier  { $$ = $1;
                                              Node_s* node = initNodeTree($4, $3, yylineno); 
                                              addSubTree($$, node); }
;
Parameters:
          /* empty */     { $$ = NULL; }
        |  RecParameter   { $$ = $1; }
;
RecMethodBody:
          /* empty */                   { $$ = initNodeTree("METHOD BODY", "", yylineno); }
        | RecMethodBody VarDeclaration  { $$ = $1; 
                                          addSubTree($$, $2); }
        | RecMethodBody Statement       { $$ = $1; 
                                          addSubTree($$, $2); }
;
MethodDeclaration:
          PUBLIC Type Identifier '(' Parameters ')'
          '{'
            RecMethodBody
            RETURN Expression ';'
          '}'                                       { $$ = initNodeTree("METHOD DECLARATION", $3, yylineno); 
                                                      addSubTree($$, initNodeTree("RETURN TYPE", $2, yylineno));
                                                      addSubTree($$, $5);
                                                      addSubTree($$, $8);
                                                      Node_s* node = initNodeTree("RETURN EXPRESSION", "", yylineno);
                                                      addSubTree(node, $10);
                                                      addSubTree($$, node); }
;

VarDeclaration:
          Type Identifier ';' { $$ = initNodeTree("VARIABLE DECLARATION", $2, yylineno);
                                addSubTree($$, initNodeTree("TYPE", $1, yylineno)); }
;

Type:
          INT '[' ']' { $$ = "INTEGER ARRAY"; }
        | BOOL        { $$ = "BOOLEAN"; }
        | INT         { $$ = "INTEGER"; }
        | Identifier  { $$ = $1; }
;

RecStatement:
          /* empty */             { $$ = NULL; }
        | RecStatement Statement  { $$ = $2; 
                                    addSubTree($$, $1); }
;
Statement:
          '{' RecStatement '}'                                    { $$ = $2; }
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
        | Identifier '=' Expression ';'                           { $$ = initNodeTree("ASSIGN", $1, yylineno); 
                                                                    addSubTree($$, $3); }
        | Identifier '[' Expression ']' '=' Expression ';'        { $$ = initNodeTree("ASSIGN INDEX", $1, yylineno); 
                                                                    addSubTree($$, $3);
                                                                    addSubTree($$, $6); }
;

RecParamExp:
          Expression                    { $$ = initNodeTree("INPUT PARAMETERS", "", yylineno); 
                                          addSubTree($$, $1); }
        | RecParamExp ',' Expression   { $$ = $1;
                                          addSubTree($$, $3); }
;
ParametersExp:
          /* empty */     { $$ = NULL; }
        |  RecParamExp  { $$ = $1; }
;
Expression:
        Expression AND Expression                                 { $$ = initNodeTree("AND", "", yylineno);
                                                                    addSubTree($$, $1);
                                                                    addSubTree($$, $3); }
      | Expression OR Expression                                  { $$ = initNodeTree("OR", "", yylineno);
                                                                    addSubTree($$, $1);
                                                                    addSubTree($$, $3); }
      | Expression '<' Expression                                 { $$ = initNodeTree("LESS THAN", "", yylineno);
                                                                    addSubTree($$, $1);
                                                                    addSubTree($$, $3); }
      | Expression '>' Expression                                 { $$ = initNodeTree("GREATER THAN", "", yylineno);
                                                                    addSubTree($$, $1);
                                                                    addSubTree($$, $3); }
      | Expression EQUAL Expression                               { $$ = initNodeTree("EQUAL", "", yylineno);
                                                                    addSubTree($$, $1);
                                                                    addSubTree($$, $3); }
      | Expression '+' Expression                                 { $$ = initNodeTree("ADDITION", "", yylineno);
                                                                    addSubTree($$, $1);
                                                                    addSubTree($$, $3); }
      | Expression '-' Expression                                 { $$ = initNodeTree("SUBTRACTION", "", yylineno);
                                                                    addSubTree($$, $1);
                                                                    addSubTree($$, $3); }
      | Expression '*' Expression                                 { $$ = initNodeTree("MULTIPLICATION", "", yylineno);
                                                                    addSubTree($$, $1);
                                                                    addSubTree($$, $3); }
      | Expression '/' Expression                                 { $$ = initNodeTree("DIVISION", "", yylineno);
                                                                    addSubTree($$, $1);
                                                                    addSubTree($$, $3); }
      | Expression '[' Expression ']'                             { $$ = initNodeTree("INDEXATION", "", yylineno); 
                                                                    addSubTree($$, $1);
                                                                    addSubTree($$, $3); }
      | Expression '.' LENGTH                                     { $$ = initNodeTree("LENGTH", "", yylineno); 
                                                                    addSubTree($$, $1); }
      | Expression '.' Identifier '(' ParametersExp ')'           { $$ = $1;
                                                                    Node_s* node = initNodeTree("FUNCTION CALL", $3, yylineno);
                                                                    addSubTree(node, $5);
                                                                    addSubTree($$, node); }
      | INTEGER_LITERAL                                           { $$ = initNodeTree("", yylval.strval, yylineno); }
      | TRUE                                                      { $$ = initNodeTree("TRUE", "", yylineno); }
      | FALSE                                                     { $$ = initNodeTree("FALSE", "", yylineno); }
      | Identifier                                                { $$ = initNodeTree("", $1, yylineno); }
      | THIS                                                      { $$ = initNodeTree("THIS", "", yylineno); }
      | NEW INT '[' Expression ']'                                { $$ = initNodeTree("ARRAY INSTANTIATION", "", yylineno); 
                                                                    addSubTree($$, $4); }
      | NEW Identifier '(' ')'                                    { $$ = initNodeTree("CLASS INSTANTIATION", $2, yylineno); }
      | '!' Expression                                            { $$ = initNodeTree("NEGATION", "", yylineno); 
                                                                    addSubTree($$, $2); }
      | '(' Expression ')'                                        { $$ = $2; }
;

Identifier:
      IDENTIFIER { $$ = yylval.strval; }
;
/* End of grammar */
%%
