#ifndef _LISH_LVAL_H
#define _LISH_LVAL_H

#include <stdint.h>
#include "mpc.h"

struct lval_t;
struct lenv_t;
typedef struct lval_t lval_t;
typedef struct lenv_t lenv_t;
typedef lval_t*(*lbuiltin)(lenv_t*, lval_t*);

enum {
  LVAL_NIL,
  LVAL_ERR,
  LVAL_INT,
  LVAL_FLOAT,
  LVAL_BOOL,
  LVAL_STR,
  LVAL_SYM,
  LVAL_FUN,
  LVAL_SEXPR,
  LVAL_QEXPR,
  LVAL_TABLE,
};

char* ltype_name(int);

#define TRUE  1
#define FALSE 0

#pragma pack(push, 1)
struct lval_t {
  uint8_t type;

  union {
    // int, float, bool
    double   num;
    // string
    char*    str;
    // error
    char*    err;

    // symbol, literal or otherwise
    struct {
      char*   name;
      uint8_t lit;
    } sym;

    // lambda and builtin
    struct {
      lbuiltin builtin;
      lval_t*  formals;
      lval_t*  body;
      lenv_t*  env;
    } func;

    // sexpr, qexpr
    struct {
      int             count;
      struct lval_t** cell;
    } expr;

    // table
    struct {
      int             count;
      struct lval_t** keys;
      struct lval_t** vals;
    } table;
  } data;
};
#pragma pack(pop)

lval_t* lval_nil(void);
lval_t* lval_bool(int);
lval_t* lval_err(char*, ...);
lval_t* lval_sym(char*, int);
lval_t* lval_fun(lbuiltin);
lval_t* lval_sexpr(void);
lval_t* lval_lambda(lval_t*, lval_t*);

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


// types
#include "types/number.h"
#include "types/string.h"
#include "types/table.h"
#include "types/qexpr.h"
#endif
