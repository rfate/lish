#ifndef _LISH_TYPE_STR_H
#define _LISH_TYPE_STR_H

#include "../lval.h"

lval_t* lval_str(char*);

void lval_str_print(lval_t*);

// builtins
lval_t* lval_str_nth(lval_t*);

#endif

