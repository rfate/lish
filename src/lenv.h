#ifndef _LISH_LENV_H
#define _LISH_LENV_H

#include "builtin.h"
#include "lval.h"

struct lenv_t {
  lenv_t*  parent;
  int      count;
  char**   syms;
  lval_t** vals;
};

lenv_t* lenv_new(void);
void    lenv_del(lenv_t*);
lenv_t* lenv_copy(lenv_t*);

lval_t* lenv_get(lenv_t*, lval_t*);
void    lenv_def(lenv_t*, lval_t*, lval_t*);
void    lenv_set(lenv_t*, lval_t*, lval_t*);

void    lenv_add_builtin(lenv_t*, char*, lbuiltin);
void    lenv_add_builtins(lenv_t* e);

#endif
