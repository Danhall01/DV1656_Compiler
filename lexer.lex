
%{
int count = 0;
%}
%option noyywrap

%%
[A-Z] {printf("%s capital letter\n", yytext);
	count++;}
.	 {printf("%s not a capital letter\n", yytext);}
\n {return 0;}
%%
