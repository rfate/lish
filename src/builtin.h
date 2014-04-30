#ifndef _LISH_BUILTIN_H
#define _LISH_BUILTIN_H

#include "assert.h"

struct lval_t;
struct lenv_t;
typedef struct lval_t lval_t;
typedef struct lenv_t lenv_t;

char* ltype_name(int);

lval_t* builtin_type(lenv_t*, lval_t*);

// symbols
lval_t* builtin_deref(lenv_t*, lval_t*);

// list handling
lval_t* builtin_eval(lenv_t*, lval_t*);
lval_t* builtin_head(lenv_t*, lval_t*);
lval_t* builtin_tail(lenv_t*, lval_t*);

lval_t* builtin_list(lenv_t*, lval_t*);
lval_t* builtin_join(lenv_t*, lval_t*);
lval_t* builtin_len(lenv_t*, lval_t*);
lval_t* builtin_nth(lenv_t*, lval_t*);
lval_t* builtin_map(lenv_t*, lval_t*);

// tables
lval_t* builtin_el(lenv_t*, lval_t*);

// strings
lval_t* builtin_substr(lenv_t*, lval_t*);
lval_t* builtin_tosym(lenv_t*, lval_t*);
lval_t* builtin_concat(lenv_t*, lval_t*);

// vars
lval_t* builtin_def(lenv_t*, lval_t*);
lval_t* builtin_set(lenv_t*, lval_t*);
lval_t* builtin_lambda(lenv_t*, lval_t*);

// operators
lval_t* builtin_add(lenv_t*, lval_t*);
lval_t* builtin_sub(lenv_t*, lval_t*);
lval_t* builtin_mul(lenv_t*, lval_t*);
lval_t* builtin_div(lenv_t*, lval_t*);
lval_t* builtin_mod(lenv_t*, lval_t*);
lval_t* builtin_not(lenv_t*, lval_t*);
lval_t* builtin_exist(lenv_t*, lval_t*);

// conditionals
lval_t* builtin_if(lenv_t*, lval_t*);
lval_t* builtin_gt(lenv_t*, lval_t*);
lval_t* builtin_lt(lenv_t*, lval_t*);
lval_t* builtin_ge(lenv_t*, lval_t*);
lval_t* builtin_le(lenv_t*, lval_t*);
lval_t* builtin_eq(lenv_t*, lval_t*);
lval_t* builtin_ne(lenv_t*, lval_t*);

// io
lval_t* builtin_load(lenv_t*, lval_t*);
lval_t* builtin_puts(lenv_t*, lval_t*);

#endif
