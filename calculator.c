#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include "calculator.h"

static double callbuiltin(struct fncall *f)
{
    enum bifs functype = f->functype;
    double v = eval(f->l);

    switch (functype) {
    case B_sqrt: return sqrt(v);
    case B_exp: return exp(v);
    case B_log: return log(v);
    case B_print: 
        printf(" = %4.4g\n", v);
        return v;
    default: 
        yyerror("Unknown build-in function %d", functype);
        return 0.0;
    }
}

void dodef(struct symbol *name, struct symlist *syms, struct ast *func)
{
    if (name->syms) symlistfree(name->syms);
    if (name->func) treefree(name->func);
    name->syms = syms;
    name->func = func;
}

static double calluser(struct ufncall *f)
{
    struct symbol *fn = f->s;       /* function name */
    struct symlist *sl;             /* dummy arguments */
    struct ast *args = f->l;        /* actual arguments */
    double *oldval, *newval;        /* saved arg values */
    double v;
    int nargs;
    if (!fn->func) {
        yyerror("call to undefined function", fn->name);
        return 0;
    }

    /* count the arguments */
    nargs = 0;
    sl = fn->syms;
    while (NULL != sl) {
        nargs++;
        /* prepare arrays to save arguments */
        oldval = malloc(nargs * sizeof(double));
        newval = malloc(nargs * sizeof(double));
        if (NULL == oldval || NULL == newval) {
            yyerror("out of space in %s", fn->name);
            return 0.0;
        }
        sl = sl->next;
    }
    /*evaluate arguments */
    for (int i = 0; i < nargs; i++) {
        if (NULL == args) {
            yyerror("too few arguments in call to %s", fn->name);
            free(oldval);
            free(newval);
        }
        /* if this is a list node */
        if (args->nodetype == 'L') {
            newval[i] = eval(args);
            args = args->r;
        } else {    /* if it is the end of the list */
            newval[i] = eval(args);
            args = NULL;
        }
    }
    /* save old values of dummies, assign new ones why? */
    sl = fn->syms;
    for (int i = 0; i < nargs; i++) {
        struct symbol *s = sl->sym;
        oldval[i] = s->value;
        s->value = newval[i];
        sl = sl->next;
    }
    free(newval);
    /* evaluate the function */
    v = eval(fn->func);
    /* put the real values of the dummies back */
    sl = fn->syms;
    for (int i = 0; i < nargs; i++) {
        struct symbol *s = sl->sym;
        s->value = oldval[i];
        sl = sl->next;
    }
    free(oldval);
    return v;
}

/* symbol table */
/* hash a symbol */
static unsigned symhash(char *sym) 
{
    unsigned int hash = 0;
    unsigned c;
    while ((c = *sym++)) {
        hash = hash*9 ^ c;
    }
    printf("DEBUG: hash: %u\n", hash);
    return hash;
}

struct symbol *lookup(char *sym)
{
    unsigned i = symhash(sym) % NHASH;
    printf("DEBUG: i: %u\n", i);
    struct symbol *sp = &symtab[i];
    int scount = NHASH;

    while (--scount >= 0) {
        /* if found then return */
        if (sp->name && !strcmp(sp->name, sym)) {
            return sp;
        }
        /* handle new entry */
        if (!sp->name) {
            sp->name = strdup(sym);
            sp->value = 0;
            sp->func = NULL;
            sp->syms = NULL;
            return sp;
        }
        /* try next entry */
        if (++sp >= symtab+NHASH) {
            sp = symtab;
        }
        /* tried them all, table is full */
        yyerror("symbol table overflow\n");
        abort();
    }
    return NULL;
}

struct symlist *newsymlist(struct symbol *sym, struct symlist *next)
{
    struct symlist *sl = malloc(sizeof(struct symlist));
    if (NULL == sl) {
        yyerror("Out of space");
        exit(0);
    }
    sl->sym = sym;
    sl->next = next;
    return sl;
}

void symlistfree(struct symlist *sl)
{
    struct symlist *nsl;
    while (NULL != sl) {
        nsl = sl->next;
        free(sl);
        sl = nsl;
    }
}

struct ast *newast(int nodetype, struct ast *l, struct ast *r)
{
    struct ast *a = malloc(sizeof(struct ast));
    if (NULL == a) {
        yyerror("Out of space");
        exit(0);
    }
    a->nodetype = nodetype;
    a->l = l;
    a->r = r;
    return a;
}

struct ast *newcmp(int cmptype, struct ast *l, struct ast *r)
{
    struct ast *a = malloc(sizeof(struct ast));
    if (NULL == a) {
        yyerror("Out of space");
        exit(0);
    }
    a->nodetype = '0' + cmptype;
    a->l = l;
    a->r = r;
    return a;
}

struct ast *newfunc(int functype, struct ast* l)
{
    struct fncall *a = malloc(sizeof(struct fncall));
    if (NULL == a) {
        yyerror("Out of space");
        exit(0);
    }
    a->nodetype = 'F';
    a->functype = functype;
    a->l = l;
    return (struct ast *)a;
}

struct ast *newcall(struct symbol *s, struct ast* l)
{
    struct ufncall *a = malloc(sizeof(struct ufncall));
    if (NULL == a) {
        yyerror("Out of space");
        exit(0);
    }
    a->nodetype = 'C';
    a->s = s;
    a->l = l;
    return (struct ast *)a;
}

struct ast *newref(struct symbol *s)
{
    struct symref *a = malloc(sizeof(struct symref));
    if (NULL == a) {
        yyerror("Out of space");
        exit(0);
    }
    a->nodetype = 'N';
    a->s = s;
    return (struct ast *)a;
}

struct ast *newasgn(struct symbol *s, struct ast *v)
{
    struct symasgn *a = malloc(sizeof(struct symasgn));
    if (NULL == a) {
        yyerror("Out of space");
        exit(0);
    }
    a->nodetype = '=';
    a->s = s;
    a->v = v;
    return (struct ast *)a;
}

struct ast *newnum(double d)
{
    struct numval *a = malloc(sizeof(struct numval));
    if (NULL == a) {
        yyerror("Out of space");
        exit(0);
    }
    a->nodetype = 'K';
    a->number  = d;
    return (struct ast *)a;
}

struct ast *newflow(int nodetype, struct ast *cond, struct ast *tl, struct ast *el)
{
    struct flow *a = malloc(sizeof(struct flow));
    if (NULL == a) {
        yyerror("Out of space");
        exit(0);
    }
    a->nodetype = nodetype;
    a->cond = cond;
    a->tl = tl;
    a->el = el;
    return (struct ast *)a;
}

double eval(struct ast *a)
{
    if (NULL == a) {
        yyerror("Internal error, null eval");
        return 0.0;
    }
    double v;
    switch (a->nodetype) {
    /* constant */
    case 'K': v = ((struct numval*)a)->number;          break;
    /* name reference */
    case 'N': v = ((struct symref*)a)->s->value;        break;
    /* assignment */
    case '=': v = ((struct symasgn*)a)->s->value = \
        eval(((struct symasgn *)a)->v);                 break;
    /* expressions */
    case '+': v = eval(a->l) + eval(a->r);              break;
    case '-': v = eval(a->l) - eval(a->r);              break;
    case '*': v = eval(a->l) * eval(a->r);              break;
    case '/': v = eval(a->l) / eval(a->r);              break;
    case '|': v = fabs(eval(a->l));                     break;
    case '^': v = pow(eval(a->l), eval(a->r));          break;
    case 'M': v = -eval(a->l);                          break;
    /* comparisons */
    case '1': v = eval(a->l) > eval(a->r) ? 1 : 0;      break;
    case '2': v = eval(a->l) < eval(a->r) ? 1 : 0;      break;
    case '3': v = eval(a->l) != eval(a->r) ? 1 : 0;     break;
    case '4': v = eval(a->l) == eval(a->r) ? 1 : 0;     break;
    case '5': v = eval(a->l) >= eval(a->r) ? 1 : 0;     break;
    case '6': v = eval(a->l) <= eval(a->r) ? 1 : 0;     break;
    /* control flow */
    /* null eexpressions allowed in the grammar, so check for them */
    /* if/then/else */
    case 'I':
        if (eval(((struct flow*)a)->cond) != 0) {
            if (((struct flow*)a)->tl) {
                v = eval(((struct flow*)a)->tl);
            } else {
                v = 0.0;    /* default value  */
            }
        } else {
            if (((struct flow*)a)->el) {
                v = eval(((struct flow*)a)->el);
            } else {
                v = 0.0;    /* default value */
            }
        }
    break;
    /* while/do */
    case 'W':
        v = 0.0;
        if (((struct flow*)a)->tl) {
            while (((struct flow*)a)->cond != 0) {
                v = eval(((struct flow*)a)->tl);
            }
        }
    break;
    case 'L': eval(a->l); v = eval(a->r);               break;
    case 'F': v = callbuiltin((struct fncall*)a);       break;
    case 'C': v = calluser((struct ufncall*)a);         break;
    default: printf("internal error: bad node %c\n", a->nodetype);
    }

    return v;
}

void treefree(struct ast *a)
{
    switch (a->nodetype) {
    /* one subtree -> right side op */
    case '+': case '-': case '*': case '/':
    case '^': case 'L': case '1': case '2':
    case '3': case '4': case '5': case '6':
        treefree(a->r);
    /* one subtree -> left side op */
    case '|': case 'M': case 'C': case 'F':
        treefree(a->l);
    /* no subtree */
    case 'K': case 'N':
        break;
    case '=':
        free(((struct symasgn *)a)->v);
        break;
    /* up to three subtrees */
    case 'I': case 'W':
        free(((struct flow *)a)->cond);
        if (((struct flow *)a)->el) free(((struct flow *)a)->el);
        if (((struct flow *)a)->tl) free(((struct flow *)a)->tl);
        break;
    default: printf("internal error: bad node %c\n", a->nodetype);
    }
    /* always free the node itself */
    free(a);
}

void yyerror(const char* const s, ...)
{
    va_list argp;
    va_start(argp, s);
    fprintf(stderr, "error on line: %d\n", yylineno);
    vfprintf(stderr, s, argp);
    fprintf(stderr, "\n");
}

int main()
{
    printf("> ");
    return yyparse();
}
