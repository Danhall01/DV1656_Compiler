%{
%}
%option noyywrap

%%
"int " {printf("INT ");}
"boolean " {printf("BOOLEAN ");}
"new " {printf("ALLOC ");}

"while"" "*"(" {printf("WHILE ");}
"if"" "*"(" {printf("IF ");}
(" "|"")+"else"(" "|"")+ {printf(" ELSE ");}

"class " {printf("CLASS ");}
"public "|"static " {printf("IDENTIFIER ");}
"return " {printf("RET ");}

" = " {printf(" OPASG ");}
" == " {printf(" OPEQ ");}



^[^a-zA-Z0-9\_]+$[0-9]+ {printf(" INTEGER");}
(" "|"(")+[0-9]+\.[0-9]+"f"{0,1} {printf(" FLOAT");}

[a-zA-Z_][a-zA-Z0-9_\.]*"(" {printf("FUNCTION ");}





"{"|"}" {}
"("|")" {}










.	 {printf("%s", yytext);}
\n {printf("\n");}
%%
