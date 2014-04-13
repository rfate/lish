#ifndef _LISH_LVAL_H
#define _LISH_LVAL_H

#include "mpc.h"

struct lval_t;
struct lenv_t;
typedef struct lval_t lval_t;
typedef struct lenv_t lenv_t;
typedef lval_t*(*lbuiltin)(lenv_t*, lval_t*);

enum {
  LVAL_ERR,
  LVAL_NUM,
  LVAL_SYM,
  LVAL_FUN,
  LVAL_SEXPR,
  LVAL_QEXPR,
};

struct lval_t {
  int type;

  double   num;
  char*    err;
  char*    sym;
  lbuiltin fun;

  int             count;
  struct lval_t** cell;
};

lval_t* lval_num(double);
lval_t* lval_err(char*, ...);
lval_t* lval_sym(char*);
lval_t* lval_fun(lbuiltin);
lval_t* lval_sexpr(void);
lval_t* lval_qexpr(void);

void    lval_del(lval_t*);
lval_t* lval_copy(lval_t*);
lval_t* lval_add(lval_t*, lval_t*);
lval_t* lval_join(lval_t*, lval_t*);

lval_t* lval_read_num(mpc_ast_t*);
lval_t* lval_read(mpc_ast_t*);

void lval_expr_print(lval_t*, char, char);
void lval_print(lval_t*);
void lval_println(lval_t*);

lval_t* lval_pop(lval_t*, int);
lval_t* lval_take(lval_t*, int);

lval_t* lval_eval      (lenv_t*, lval_t*);
lval_t* lval_eval_sexpr(lenv_t*, lval_t*);

#endif
