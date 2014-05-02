#ifndef _LISH_TYPE_TABLE_H
#define _LISH_TYPE_TABLE_H

#include "../lval.h"

// Standard functions
lval_t* lval_table(void);
void    lval_table_del(lval_t*);
lval_t* lval_table_copy(lval_t*);
void    lval_table_print(lval_t*);

// Table specifics.
void    lval_table_set(lval_t*, lval_t*, lval_t*);
lval_t* lval_table_get(lval_t*, lval_t*);

// Builtins.
lval_t* lval_table_nth(lval_t*);
lval_t* lval_table_el(lval_t*);
lval_t* lval_table_len(lval_t*);

#endif

