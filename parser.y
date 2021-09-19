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
%token IF ELSE WHILE FUN

/* precedence and associativit rules for symbols */
%right '='
%left <fn> OR
%left <fn> AND
%left '|'
%left '^'
%left '&'
%left <fn> CMP
%left '+' '-'
%left '*' '/'
%nonassoc '!'
%nonassoc UMINUS

/* Assigns the value of <a> to exp... */
%type <a> exp stmts list explist all_symbols if_stmt
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
  | calclist all_symbols
  {
      printf("= %4.4g\n> ", eval($2)); 
      treefree($2);
  }
  | calclist FUN NAME '(' symlist ')' '{' list '}' 
  {
      dodef($3, $5, $8);
      printf("Defined %s\n> ", $3->name);
  }
  | calclist error { printf("> "); }
    /* blank line or comment and error recovery 
     * pseudotoken error indicates an error recovery point
     */
;

all_symbols:
    stmts
  | exp ';'
  | ';'     { $$ = newast(';', NULL, NULL); }
;

list:
    /* empty */                 { $$ = NULL; }
  | all_symbols list            { $$ = $2 == NULL ? $1 : newast('L', $1, $2); }
;

stmts:
    if_stmt
  | WHILE '(' exp ')' '{' list '}'                        { $$ = newflow('W', $3, $6, NULL); }
;

if_stmt:
    IF '(' exp ')' '{'list '}'                            { $$ = newflow('I', $3, $6, NULL); }
  | IF '(' exp ')' '{'list '}' ELSE if_stmt               { $$ = newflow('I', $3, $6, $9); }
  | IF '(' exp ')' '{'list '}' ELSE '{' list '}'          { $$ = newflow('I', $3, $6, $10); }
;

exp:
    exp CMP exp                { $$ = newcmp($2, $1, $3); }
  | exp AND exp                { $$ = newcmp($2, $1, $3); }
  | exp OR exp                 { $$ = newcmp($2, $1, $3); }
  | exp '+' exp                { $$ = newast('+', $1, $3); }
  | exp '-' exp                { $$ = newast('-', $1, $3); }
  | exp '*' exp                { $$ = newast('*', $1, $3); }
  | exp '/' exp                { $$ = newast('/', $1, $3); }
  | exp '^' exp                { $$ = newast('^', $1, $3); }
  | exp '|' exp                { $$ = newast('|', $1, $3); }
  | exp '&' exp                { $$ = newast('&', $1, $3); }
  | '(' exp ')'                { $$ = $2; }
  | '-' exp %prec UMINUS       { $$ = newast('M', $2, NULL); }  /* use UMINUS precedence for this rule */
  | '!' exp %prec UMINUS       { $$ = newast('!', $2, NULL); }  /* use UMINUS precedence for this rule */
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
