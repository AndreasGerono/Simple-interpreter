/* Complex Calculator */

%{
#include <stdio.h>
#include <stdlib.h>
#include "calculator.h"

int yylex(void);
%}

/* Declare types of symbols (tokenas and nonterminals) to be used in parser */
/* by default all int */
/* this is yylval in scanner */
%union {
    struct ast *a;
    struct symbol *s;       /* which symbol */
    struct symlist *sl;
    int fn;                 /* which function or operation */
    double d;
}

/* Declare tokens
 * When we declare types symbols need type, int by default
 * You don’t have to declare a type for a token or declare a nonterminal at all if you don’t
 * use the symbol’s value. -> EOL
 */

%token <d> NUMBER
%token <s> NAME
%token <fn> FUNC
%token EOL
%token IF THEN ELSE WHILE DO LET

/* precedence and associativit rules for symbols */
%nonassoc <fn> CMP
%right '='
%left '+' '-'
%left '*' '/'
%left '^'
%nonassoc '|' UMINUS

/* Assigns the value of <a> to exp... */
%type <a> exp stmt list explist
%type <sl> symlist

/* defines the top-level rule */
%start calclist

/* Rules now create the ast instead of evaluating on the fly.
 * Rules can use literals tokens, no need to declare them unless you need to declare type
 */
 
 /* All rules are now in one symbol -> precedence and associativit is set explicitly by the %left operator,
  * not inplicitly by the symbols groups, lower the symbol with %ass higher the precedence
  */

%define parse.error verbose

%%

calclist:
    /* empty */
  | calclist stmt EOL {
      printf("= %4.4g\n>", eval($2)); 
      treefree($2);
  }
  | calclist LET NAME '(' symlist ')' '=' list EOL {
      dodef($3, $5, $8);
      printf("Defined %s\n> ", $3->name);
  }
  | calclist error EOL { printf("> "); }
    /* blank line or comment and error recovery 
     * pseudotoken error indicates an error recovery point
     */
;

stmt:
    IF exp THEN list            { $$ = newflow('I', $2, $4, NULL); }
  | IF exp THEN list ELSE list  { $$ = newflow('I', $2, $4, $6); }
  | WHILE exp DO list           { $$ = newflow('I', $2, $4, NULL); }
  | exp
;

list:
    /* empty */             { $$ = NULL; }
  | stmt ';' list           { $$ = $3 == NULL ? $1 : newast('L', $1, $3); }
;

exp:
    exp CMP exp                { $$ = newcmp($2, $1, $3); }
  | exp '+' exp                { $$ = newast('+', $1, $3); }
  | exp '-' exp                { $$ = newast('-', $1, $3); }
  | exp '*' exp                { $$ = newast('*', $1, $3); }
  | exp '/' exp                { $$ = newast('/', $1, $3); }
  | exp '^' exp                { $$ = newast('^', $1, $3); }
  | '|' exp                    { $$ = newast('|', $2, NULL); }
  | '(' exp ')'                { $$ = $2; }
  | '-' exp %prec UMINUS       { $$ = newast('M', $2, NULL); }  /* use UMINUS precedence for this rule */
  | NUMBER                     { $$ = newnum($1); }
  | NAME                       { $$ = newref($1); }
  | NAME '=' exp               { $$ = newasgn($1, $3); }
  | FUNC '(' explist ')'       { $$ = newfunc($1, $3); }
  | NAME '(' explist ')'       { $$ = newcall($1, $3); }
;

 /* function args for call */
explist:
    exp                 /* default */
  | exp ',' explist     { $$ = newast('L', $1, $3); }
;

 /* function symbols for def */
symlist:
    NAME                { $$ = newsymlist($1, NULL); }
  | NAME ',' symlist    { $$ = newsymlist($1, $3); }
;
%%
