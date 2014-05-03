#ifndef _LISH_TYPE_QEXPR_H
#define _LISH_TYPE_QEXPR_H

#include "../lval.h"

lval_t* lval_qexpr(void);

// builtins
lval_t* lval_qexpr_len(lval_t*);
lval_t* lval_qexpr_nth(lval_t*);

#endif

