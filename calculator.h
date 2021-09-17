/* Abstract syntax tree for complex calculator
 * All structures with nodetype are kinds of nodes for AST
 * Node types:
 * + - * / | ^
 * 0-7 comparison ops, bit coded 04 equal, 02 less, 01 greater M unary minus
 * L expression or statement list
 * I IF statement
 * W WHILE statement
 * N symbol ref
 * = assignment
 * S list of symbols
 * F built in function call
 * C user function call 
 */

#ifndef AST_H
#define AST_H

#define NHASH 9997          /* ? */

/* interface to the lexet */
extern int yyparse(void);   /* from lexer */
extern int yylineno;        /* from lexer */
void yyerror(const char* const s, ...);

struct symbol;

/* symbol table */
struct symbol {
    char *name;             /* variable name */
    double value;
    struct ast *func;       /* stmt for function */
    struct symlist *syms;   /* list of args */
};

struct symbol symtab[NHASH];

struct symlist {
    struct symbol *sym;
    struct symlist *next;
};

/* build in functions */
enum bifs {
    B_sqrt = 1,
    B_exp,
    B_log,
    B_print,
};

/* nodes in the abstract syntax tree */
struct ast {
    int nodetype;
    struct ast *l;
    struct ast *r;
};

/* build in function */
struct fncall {
    int nodetype;           /* type F */
    struct ast *l;
    enum bifs functype;
};

/* user defined function */
struct ufncall {
    int nodetype;           /* type C */
    struct ast *l;          /* list of arguments */
    struct symbol *s;       /* func symbols */
};

/* reference to a symbol */
struct symref {
    int nodetype;       /* type N */
    struct symbol *s;
};

/* all assignments */
struct symasgn {
    int nodetype;       /* type = */
    struct symbol *s;
    struct ast *v;      /* value */
};

/* constant numbers */
struct numval {
    int nodetype;       /* type K */
    double number;
};

/* flow controll expressions if/then/else while/do */
struct flow {
    int nodetype;       /* type I or W */
    struct ast *cond;   /* condition */
    struct ast *tl;     /* then branch or do list */
    struct ast *el;     /* optional else branch */
};

/* simple symtab of fixed size */

struct symbol *lookup(char*);
struct symlist *newsymlist(struct symbol *sym, struct symlist *next);
void symlistfree(struct symlist *sl);

/* build an AST */

struct ast *newast(int nodetype, struct ast* l, struct ast* r);
struct ast *newcmp(int cmptype, struct ast* l, struct ast* r);
struct ast *newfunc(int functype, struct ast* l);
struct ast *newcall(struct symbol *s, struct ast* l);
struct ast *newref(struct symbol *s);
struct ast *newasgn(struct symbol *s, struct ast *v);
struct ast *newnum(double d);
struct ast *newflow(int nodetype, struct ast *cond, struct ast *tl, struct ast *el);

/* define a function */
void dodef(struct symbol *name, struct symlist *syms, struct ast *func);

/* evaluate an AST */
double eval(struct ast* a);

/* delete and free AST */
void treefree(struct ast* a);

#endif
