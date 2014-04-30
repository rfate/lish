#ifndef _LISH_TYPE_TABLE_H
#define _LISH_TYPE_TABLE_H

#include "../lval.h"

lval_t* lval_table(void);

void lval_table_print(lval_t*);

// builtins

lval_t* lval_table_nth(lval_t*);
lval_t* lval_table_el(lval_t*);

#endif

