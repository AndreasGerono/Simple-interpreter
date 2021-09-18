/* Tokens */
/* No need for 3 section */

%option noyywrap nodefault yylineno

%{
#include "parser.tab.h"     /* token definitions */
#include "calculator.h"     /* ast definition    */
%}

/* complex patterns defined heare to reuse later */
/* float exponent */

EXP     ([Ee][-+]?[0-9]+)

/* Actions return a token*/
/* Same action for tokens -> separate by | */
/* return first char as a token */

%%
 /* single character symbols */
"=" |
"+" |
"-" |
"*" |
"^" |
"/" |
"|" |
"," |
";" |
"{" |
"}" |
"(" |
")"                     { return yytext[0]; }

 /* comparison ops, all are CMP token */
">"                     { yylval.fn = 1; return CMP; }
"<"                     { yylval.fn = 2; return CMP; }
"!="                    { yylval.fn = 3; return CMP; }
"=="                    { yylval.fn = 4; return CMP; }
">="                    { yylval.fn = 5; return CMP; }
"<="                    { yylval.fn = 6; return CMP; }

 /* keywords */
"if"                    { return IF; }
"else"                  { return ELSE; }
"while"                 { return WHILE; }
"fun"                   { return FUN; }

 /* buld in functions */
"sqrt"                  { yylval.fn = B_sqrt; return FUNC; }
"exp"                   { yylval.fn = B_exp; return FUNC; }
"log"                   { yylval.fn = B_log; return FUNC; }
"print"                 { yylval.fn = B_print; return FUNC; }

 /* names */
[a-zA-Z][a-zA-Z0-9]*    { yylval.s = lookup(yytext); return NAME; }     /* looks up the name in the sym table */

"."?[0-9]+{EXP}?    |
[0-9]+"."[0-9]*{EXP}    { yylval.d = atof(yytext); return NUMBER; }

"//".*                  /* ignore comment */
[ \t]                   /* ignore whitespace */
\\\n                    { printf("c> "); }  /* ignore line continuation as \ */
\n                      { }
.                       { yyerror("Mystery character %c\n", *yytext); }
%%
