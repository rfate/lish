#ifndef _LISH_LVAL_H
#define _LISH_LVAL_H

#include "mpc.h"

struct lval_t;
struct lenv_t;
typedef struct lval_t lval_t;
typedef struct lenv_t lenv_t;
typedef lval_t*(*lbuiltin)(lenv_t*, lval_t*);

enum {
  LVAL_ERR   =   1,
  LVAL_INT   =   2,
  LVAL_FLOAT =   4,
  LVAL_BOOL  =   8,
  LVAL_STR   =  16,
  LVAL_SYM   =  32,
  LVAL_FUN   =  64,
  LVAL_SEXPR = 128,
  LVAL_QEXPR = 256,
  LVAL_TABLE = 512,
  // ORs
  LVAL_NUM   = (LVAL_INT | LVAL_FLOAT),
};

char* ltype_name(int);

struct lval_t {
  int type;

  union {  
    double   num;
    char*    str;
    char*    err;
    char*    sym;

    struct {
      lbuiltin builtin;
      lval_t*  formals;
      lval_t*  body;
    } func;
  } data;

  lenv_t*  env;

  int             count;
  struct lval_t** cell;
};

lval_t* lval_int(long int);
lval_t* lval_float(double);
lval_t* lval_bool(int);
lval_t* lval_str(char*);
lval_t* lval_err(char*, ...);
lval_t* lval_sym(char*);
lval_t* lval_fun(lbuiltin);
lval_t* lval_sexpr(void);
lval_t* lval_qexpr(void);
lval_t* lval_lambda(lval_t*, lval_t*);
lval_t* lval_table(void);

void    lval_del(lval_t*);
lval_t* lval_copy(lval_t*);
lval_t* lval_add(lval_t*, lval_t*);
lval_t* lval_join(lval_t*, lval_t*);

lval_t* lval_read_num(mpc_ast_t*);
lval_t* lval_read(mpc_ast_t*);

int     lval_eq(lval_t*, lval_t*);
lval_t* lval_truthy(lval_t*);

void    lval_expr_print(lval_t*, char, char);
void    lval_print(lval_t*);
void    lval_println(lval_t*);

lval_t* lval_pop(lval_t*, int);
lval_t* lval_take(lval_t*, int);

lval_t* lval_eval      (lenv_t*, lval_t*);
lval_t* lval_eval_sexpr(lenv_t*, lval_t*);

#endif
