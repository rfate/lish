#ifndef _LISH_NUMBER_H
#define _LISH_NUMBER_H

#include "../lval.h"

lval_t* lval_int(long int);
lval_t* lval_float(double);
lval_t* lval_bigint(char*);

#endif

