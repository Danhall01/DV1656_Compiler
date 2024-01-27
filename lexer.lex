%{
int lex_valid = 1;
int lineno = 1;
%}
%option noyywrap

%%
"int" {if (lex_valid) printf("INT");}
"boolean" {if (lex_valid) printf("BOOL");}
"void" {if (lex_valid) printf("VOID");}
"new" {if (lex_valid) printf("ALLOC");}

"while" {if (lex_valid) printf("WHILE");}
"if" {if (lex_valid) printf("IF");}
"else" {if (lex_valid) printf("ELSE");}

"class" {if (lex_valid) printf("CLASS");}
"public" {if (lex_valid) printf("PUBLIC");}
"static" {if (lex_valid) printf("STATIC");}
"return" {if (lex_valid) printf("RET");}

"=" {if (lex_valid) printf("OP_ASG");}
"-" {if (lex_valid) printf("OP_SUB");}
"+" {if (lex_valid) printf("OP_ADD");}
"*" {if (lex_valid) printf("OP_MUL");}
"/" {if (lex_valid) printf("OP_DIV");}

"==" {if (lex_valid) printf("OP_EQ");}
"!" {if (lex_valid) printf("OP_NOT");}
"<" {if (lex_valid) printf("OP_L");}
"<=" {if (lex_valid) printf("OP_LE");}
">" {if (lex_valid) printf("OP_G");}
">=" {if (lex_valid) printf("OP_GE");}
"&&" {if (lex_valid) printf("OP_AND");}
"||" {if (lex_valid) printf("OP_OR");}

[+0-9]+ {if (lex_valid) printf("INTEGER_T");}
"true"|"false" {if (lex_valid) printf("BOOLEAN_T");}

[a-zA-Z_][a-zA-Z0-9_\.]*"(" {if (lex_valid) printf("FUNC PAR_START ");}
"(" {if (lex_valid) printf(" PAR_START ");}
")" {if (lex_valid) printf(" PAR_END");}

[a-zA-Z_][a-zA-Z0-9_]*"[]" {if (lex_valid) printf("ARRAY");}
[a-zA-Z_][a-zA-Z0-9_]* {if (lex_valid) printf("VAR");}

"\." {if (lex_valid) printf(" CHAIN ");}

"\t" {if (lex_valid) printf("\t");}
"," {}
"{"|"}" {}
" " {if (lex_valid) printf(" ");}
";" {if (lex_valid) printf(";");}

"//"(.)+ {}
. {printf("\n[-] Error (line: %d) - lexical ('%s' is not recognized by the grammar.)", lineno, yytext); lex_valid = 0; }

\n {if (lex_valid) printf("\n"); ++lineno;}
%%
