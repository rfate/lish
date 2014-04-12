#ifndef _LISH_BUILTIN_H
#define _LISH_BUILTIN_H

#include "lval.h"

lval_t* builtin_op  (lval_t*, char*);
lval_t* builtin_head(lval_t*, char*);
lval_t* builtin_tail(lval_t*, char*);
lval_t* builtin_eval(lval_t*, char*);
lval_t* builtin_list(lval_t*, char*);
lval_t* builtin_join(lval_t*, char*);

#endif
