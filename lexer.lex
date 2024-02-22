%{
#include "settings.h"
#include "parser.tab.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int lex_valid = 1;
extern int yylineno;
extern int yylex(void);
%}
%option yylineno noyywrap nounput batch noinput

%%
"int" {if (lex_valid && PRINT_LEXER_OUTPUT) printf("INT"); return INT;}
"boolean" {if (lex_valid && PRINT_LEXER_OUTPUT) printf("BOOL"); return BOOL;}
"String" {if (lex_valid && PRINT_LEXER_OUTPUT) printf("STRING"); return STRING;}
"void" {if (lex_valid && PRINT_LEXER_OUTPUT) printf("VOID"); return VOID;}

"main" {if (lex_valid && PRINT_LEXER_OUTPUT) printf("MAIN"); return MAIN;}
"new" {if (lex_valid && PRINT_LEXER_OUTPUT) printf("ALLOC"); return NEW;}
"this" {if (lex_valid && PRINT_LEXER_OUTPUT) printf("THIS"); return THIS;}
"System.out.println" {if (lex_valid && PRINT_LEXER_OUTPUT) printf("PRINTLN"); return PRINTLN;}
"length" {if (lex_valid && PRINT_LEXER_OUTPUT) printf("LENGTH"); return LENGTH;}

"while" {if (lex_valid && PRINT_LEXER_OUTPUT) printf("WHILE"); return WHILE;}
"if" {if (lex_valid && PRINT_LEXER_OUTPUT) printf("IF"); return IF;}
"else" {if (lex_valid && PRINT_LEXER_OUTPUT) printf("ELSE"); return ELSE;}

"class" {if (lex_valid && PRINT_LEXER_OUTPUT) printf("CLASS"); return CLASS;}
"public" {if (lex_valid && PRINT_LEXER_OUTPUT) printf("PUBLIC"); return PUBLIC;}
"static" {if (lex_valid && PRINT_LEXER_OUTPUT) printf("STATIC"); return STATIC;}
"return" {if (lex_valid && PRINT_LEXER_OUTPUT) printf("RET"); return RETURN;}

"=" {if (lex_valid && PRINT_LEXER_OUTPUT) printf("OP_ASG"); return '=';}
"-" {if (lex_valid && PRINT_LEXER_OUTPUT) printf("OP_SUB"); return '-';}
"+" {if (lex_valid && PRINT_LEXER_OUTPUT) printf("OP_ADD"); return '+';}
"*" {if (lex_valid && PRINT_LEXER_OUTPUT) printf("OP_MUL"); return '*';}
"/" {if (lex_valid && PRINT_LEXER_OUTPUT) printf("OP_DIV"); return '/';}

"!" {if (lex_valid && PRINT_LEXER_OUTPUT) printf("OP_NOT"); return '!';}
"<" {if (lex_valid && PRINT_LEXER_OUTPUT) printf("OP_L"); return '<';}
">" {if (lex_valid && PRINT_LEXER_OUTPUT) printf("OP_G"); return '>';}

"==" {if (lex_valid && PRINT_LEXER_OUTPUT) printf("OP_EQ"); return EQUAL;}
"<=" {if (lex_valid && PRINT_LEXER_OUTPUT) printf("OP_LE");}
">=" {if (lex_valid && PRINT_LEXER_OUTPUT) printf("OP_GE");}
"&&" {if (lex_valid && PRINT_LEXER_OUTPUT) printf("OP_AND"); return AND;}
"||" {if (lex_valid && PRINT_LEXER_OUTPUT) printf("OP_OR"); return OR;}

0|[1-9][0-9]* {if (lex_valid && PRINT_LEXER_OUTPUT) printf("INTEGER");
    char* str = malloc(strlen(yytext)+1);
    strcpy(str, yytext);
    yylval.strval=str; 
    return INTEGER_LITERAL;}

"true" {if (lex_valid && PRINT_LEXER_OUTPUT) printf("TRUE"); return TRUE;}
"false" {if (lex_valid && PRINT_LEXER_OUTPUT) printf("FALSE"); return FALSE;}

"(" {if (lex_valid && PRINT_LEXER_OUTPUT) printf("("); return '(';}
")" {if (lex_valid && PRINT_LEXER_OUTPUT) printf(")"); return ')';}
"{" {if (lex_valid && PRINT_LEXER_OUTPUT) printf("{"); return '{';}
"}" {if (lex_valid && PRINT_LEXER_OUTPUT) printf("}"); return '}';}
"[" {if (lex_valid && PRINT_LEXER_OUTPUT) printf("["); return '[';}
"]" {if (lex_valid && PRINT_LEXER_OUTPUT) printf("]"); return ']';}


"\." {if (lex_valid && PRINT_LEXER_OUTPUT) printf("."); return '.';}
"," {if (lex_valid && PRINT_LEXER_OUTPUT) printf(","); return ',';}

"\t" {if (lex_valid && PRINT_LEXER_OUTPUT) printf("\t");}
" " {if (lex_valid && PRINT_LEXER_OUTPUT) printf(" ");}
";" {if (lex_valid && PRINT_LEXER_OUTPUT) printf(";"); return ';';}
"\r" {}

[a-zA-Z_][a-zA-Z0-9_]* {if (lex_valid && PRINT_LEXER_OUTPUT) printf("IDENTIFIER"); 
    char* str = malloc(strlen(yytext)+1);
    strcpy(str, yytext);
    yylval.strval=str;
    return IDENTIFIER;
}


\n {if (lex_valid && PRINT_LEXER_OUTPUT) printf("\n");}
<<EOF>> {printf("\n%s\n", lex_valid ? "[+] Lexing finished successfully.":"[-] Lexer failed to parse file. (Se logs for details)"); return EOF;}


"//"[^\n]* {}
. {fprintf(stderr, "\n[-] \t@error at line %d: - lexical ('%s' is not recognized by the grammar.)", yylineno, yytext); lex_valid = 0; }
%%
