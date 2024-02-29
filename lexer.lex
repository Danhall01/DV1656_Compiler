%{
#include "settings.h"
#include "parser.tab.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int lex_valid = 1;
int lex_colno = 0;
extern int yylineno;
extern int yylex(void);
%}
%option yylineno noyywrap nounput batch noinput

%%
"int" {if (lex_valid && PRINT_LEXER_OUTPUT) printf("INT");lex_colno += strlen(yytext); return INT;}
"boolean" {if (lex_valid && PRINT_LEXER_OUTPUT) printf("BOOL");lex_colno += strlen(yytext); return BOOL;}
"String" {if (lex_valid && PRINT_LEXER_OUTPUT) printf("STRING");lex_colno += strlen(yytext); return STRING;}
"void" {if (lex_valid && PRINT_LEXER_OUTPUT) printf("VOID");lex_colno += strlen(yytext); return VOID;}

"main" {if (lex_valid && PRINT_LEXER_OUTPUT) printf("MAIN");lex_colno += strlen(yytext); return MAIN;}
"new" {if (lex_valid && PRINT_LEXER_OUTPUT) printf("ALLOC");lex_colno += strlen(yytext); return NEW;}
"this" {if (lex_valid && PRINT_LEXER_OUTPUT) printf("THIS");lex_colno += strlen(yytext); return THIS;}
"System.out.println" {if (lex_valid && PRINT_LEXER_OUTPUT) printf("PRINTLN");lex_colno += strlen(yytext); return PRINTLN;}
"length" {if (lex_valid && PRINT_LEXER_OUTPUT) printf("LENGTH");lex_colno += strlen(yytext); return LENGTH;}

"while" {if (lex_valid && PRINT_LEXER_OUTPUT) printf("WHILE");lex_colno += strlen(yytext); return WHILE;}
"if" {if (lex_valid && PRINT_LEXER_OUTPUT) printf("IF");lex_colno += strlen(yytext); return IF;}
"else" {if (lex_valid && PRINT_LEXER_OUTPUT) printf("ELSE");lex_colno += strlen(yytext); return ELSE;}

"class" {if (lex_valid && PRINT_LEXER_OUTPUT) printf("CLASS");lex_colno += strlen(yytext); return CLASS;}
"public" {if (lex_valid && PRINT_LEXER_OUTPUT) printf("PUBLIC");lex_colno += strlen(yytext); return PUBLIC;}
"static" {if (lex_valid && PRINT_LEXER_OUTPUT) printf("STATIC");lex_colno += strlen(yytext); return STATIC;}
"return" {if (lex_valid && PRINT_LEXER_OUTPUT) printf("RET");lex_colno += strlen(yytext); return RETURN;}

"=" {if (lex_valid && PRINT_LEXER_OUTPUT) printf("OP_ASG");lex_colno += strlen(yytext); return '=';}
"-" {if (lex_valid && PRINT_LEXER_OUTPUT) printf("OP_SUB");lex_colno += strlen(yytext); return '-';}
"+" {if (lex_valid && PRINT_LEXER_OUTPUT) printf("OP_ADD");lex_colno += strlen(yytext); return '+';}
"*" {if (lex_valid && PRINT_LEXER_OUTPUT) printf("OP_MUL");lex_colno += strlen(yytext); return '*';}
"/" {if (lex_valid && PRINT_LEXER_OUTPUT) printf("OP_DIV");lex_colno += strlen(yytext); return '/';}

"!" {if (lex_valid && PRINT_LEXER_OUTPUT) printf("OP_NOT");lex_colno += strlen(yytext); return '!';}
"<" {if (lex_valid && PRINT_LEXER_OUTPUT) printf("OP_L");lex_colno += strlen(yytext); return '<';}
">" {if (lex_valid && PRINT_LEXER_OUTPUT) printf("OP_G");lex_colno += strlen(yytext); return '>';}

"==" {if (lex_valid && PRINT_LEXER_OUTPUT) printf("OP_EQ");lex_colno += strlen(yytext); return EQUAL;}
"<=" {if (lex_valid && PRINT_LEXER_OUTPUT) printf("OP_LE");lex_colno += strlen(yytext);}
">=" {if (lex_valid && PRINT_LEXER_OUTPUT) printf("OP_GE");lex_colno += strlen(yytext);}
"&&" {if (lex_valid && PRINT_LEXER_OUTPUT) printf("OP_AND");lex_colno += strlen(yytext); return AND;}
"||" {if (lex_valid && PRINT_LEXER_OUTPUT) printf("OP_OR");lex_colno += strlen(yytext); return OR;}

0|[1-9][0-9]* {if (lex_valid && PRINT_LEXER_OUTPUT) printf("INTEGER");
    lex_colno += strlen(yytext);
    char* str = malloc(strlen(yytext)+1);
    strcpy(str, yytext);
    yylval.strval=str;
    return INTEGER_LITERAL;}

"true" {if (lex_valid && PRINT_LEXER_OUTPUT) printf("TRUE");lex_colno += strlen(yytext); return TRUE;}
"false" {if (lex_valid && PRINT_LEXER_OUTPUT) printf("FALSE");lex_colno += strlen(yytext); return FALSE;}

"(" {if (lex_valid && PRINT_LEXER_OUTPUT) printf("(");lex_colno += strlen(yytext); return '(';}
")" {if (lex_valid && PRINT_LEXER_OUTPUT) printf(")");lex_colno += strlen(yytext); return ')';}
"{" {if (lex_valid && PRINT_LEXER_OUTPUT) printf("{");lex_colno += strlen(yytext); return '{';}
"}" {if (lex_valid && PRINT_LEXER_OUTPUT) printf("}");lex_colno += strlen(yytext); return '}';}
"[" {if (lex_valid && PRINT_LEXER_OUTPUT) printf("[");lex_colno += strlen(yytext); return '[';}
"]" {if (lex_valid && PRINT_LEXER_OUTPUT) printf("]");lex_colno += strlen(yytext); return ']';}


"\." {if (lex_valid && PRINT_LEXER_OUTPUT) printf(".");lex_colno += strlen(yytext); return '.';}
"," {if (lex_valid && PRINT_LEXER_OUTPUT) printf(",");lex_colno += strlen(yytext); return ',';}

"\t" {if (lex_valid && PRINT_LEXER_OUTPUT) printf("\t");lex_colno += strlen(yytext);}
" " {if (lex_valid && PRINT_LEXER_OUTPUT) printf(" ");lex_colno += strlen(yytext);}
";" {if (lex_valid && PRINT_LEXER_OUTPUT) printf(";");lex_colno += strlen(yytext); return ';';}
"\r" {lex_colno += strlen(yytext);}

[a-zA-Z_][a-zA-Z0-9_]* {if (lex_valid && PRINT_LEXER_OUTPUT) printf("IDENTIFIER");
   lex_colno += strlen(yytext);
   char* str = malloc(strlen(yytext)+1);
   strcpy(str, yytext);
   yylval.strval=str;
   return IDENTIFIER;
}


\n {if (lex_valid && PRINT_LEXER_OUTPUT) printf("\n"); lex_colno = 0;}
<<EOF>> {printf("\n%s\n", lex_valid ? "[+] Lexing finished successfully.":"[-] Lexer failed to parse file. (Se logs for details)"); lex_colno += strlen(yytext);return EOF;}


"//"[^\n]* {}
. {fprintf(stderr, "\n[-] \t@error at line %d: - lexical ('%s' is not recognized by the grammar.)", yylineno, yytext); lex_valid = 0; lex_colno += strlen(yytext);}
%%
