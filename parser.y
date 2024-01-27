%defines
%define parse.error verbose

%code requires{

} 

%code{

}

%token PUBLIC CLASS STATIC VOID MAIN STRING RETURN INT BOOL IF ELSE WHILE PRINTLN AND OR EQUAL LENGTH TRUE FALSE THIS NEW INTEGER_LITERAL IDENTIFIER FEND

%right then ELSE
%right '='
%left '-' '+'
%left '*' '/'

%start Goal

%%
Identifier:
      IDENTIFIER
;

RecParamExp:
        /* empty */
      | RecParamExp ',' Expression
;
Operators:
        /* empty */
      | Expression AND
      | Expression OR 
      | Expression '<'
      | Expression '>'
      | Expression EQUAL
      | Expression '+'
      | Expression '-'
      | Expression '*'
      | Expression '/'
      | Operators '!'
;
Expression:
        Expression '[' Expression ']'
      | Expression '.' LENGTH
      | Expression '.' Identifier '(' ')'
      | Expression '.' Identifier '(' Expression RecParamExp ')'
      | Operators INTEGER_LITERAL
      | Operators TRUE
      | Operators FALSE
      | Operators Identifier
      | Operators THIS
      | Operators NEW INT '[' Expression ']'
      | Operators NEW Identifier '(' ')'
      | Operators '(' Expression ')'
;

RecStatement:
          /* empty */
        | RecStatement Statement
;
Statement:
          '{' RecStatement '}'
        | IF '(' Expression ')' Statement              %prec then
        | IF '(' Expression ')' Statement ELSE Statement
        | WHILE '(' Expression ')' Statement
        | PRINTLN '(' Expression ')' ';'
        | Identifier '=' Expression ';'
        | Identifier '[' Expression ']' '=' Expression ';'
;

Type:
          INT '[' ']'
        | BOOL
        | INT
        | Identifier
;

VarDeclaration:
          Type Identifier ';'
;

RecParameter:
          /* empty */
        | RecParameter ',' Type Identifier
;
Parameters:
          /* empty */
        | Type Identifier RecParameter
;
RecMethodBody:
          /* empty */
        | RecMethodBody VarDeclaration
        | RecMethodBody Statement
;
MethodDeclaration:
          PUBLIC Type Identifier '(' Parameters ')'
          '{'
            RecMethodBody
            RETURN Expression ';'
          '}'
;

RecVarDecl:
          /* empty */
        | RecVarDecl VarDeclaration
;
RecMethodDecl:
          /* empty */
        | RecMethodDecl MethodDeclaration
;
ClassDeclaration:
          CLASS Identifier
          '{' 
              RecVarDecl
              RecMethodDecl
          '}'
;

RecMain:
          Statement
        | RecMain Statement
;
MainClass:
          PUBLIC CLASS Identifier 
          '{' 
            PUBLIC STATIC VOID MAIN '(' STRING '[' ']' Identifier ')' 
            '{'  
              RecMain 
            '}' 
          '}'
;

RecClassDecl:
          /* empty */
        | RecClassDecl ClassDeclaration
;
Goal:     
          MainClass RecClassDecl FEND
;

/* End of grammar */
%%

#include <stdio.h>

yyerror (s)  /* Called by yyparse on error */
     char *s;
{
  printf ("%s\n", s);
}
