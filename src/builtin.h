#ifndef _LISH_BUILTIN_H
#define _LISH_BUILTIN_H

struct lval_t;
struct lenv_t;
typedef struct lval_t lval_t;
typedef struct lenv_t lenv_t;

char* ltype_name(int);

#define LASSERT(args, cond, fmt, ...)           \
  if (!(cond)) {                                \
    lval_t* err = lval_err(fmt, ##__VA_ARGS__); \
    lval_del(args);                             \
    return err;                                 \
  }

#define LASSERT_ARG_COUNT(name, args, c)                                       \
  LASSERT(args, args->data.expr.count == c,                                    \
    "Builtin \"%s\" given incorrect number of arguments. Expected %d, got %d", \
    name, c, args->data.expr.count)

#define LASSERT_NONEMPTY_LIST(name, args, argn)                         \
  LASSERT(args, args->data.expr.cell[argn]->data.expr.count != 0,       \
    "Builtin \"%s\" given empty list for argument %d.", name, (argn+1))

#define LASSERT_ARG_TYPE(name, args, index, _type)                \
  LASSERT(args, args->data.expr.cell[index]->type == _type,       \
      "Builtin \"%s\" expected arg %d to be of type %s, got %s.", \
          name, (index+1), ltype_name(_type),                     \
          ltype_name(args->data.expr.cell[index]->type))          \

#define LASSERT_ARG_IS_NUM(name, args, index)                             \
  LASSERT(args, (args->data.expr.cell[index]->type == LVAL_INT            \
              || args->data.expr.cell[index]->type == LVAL_FLOAT),        \
          "Builtin \"%s\" expected arg %d to be Number, got %s.",         \
          name, (index+1), ltype_name(args->data.expr.cell[index]->type))

#define LASSERT_ARG_ITERABLE(name, args, index)                                  \
  LASSERT(args, (args->data.expr.cell[index]->type == LVAL_QEXPR                 \
              || args->data.expr.cell[index]->type == LVAL_STR                   \
              || args->data.expr.cell[index]->type == LVAL_TABLE),               \
    "Builtin \"%s\" expected arg %d of type %s to have length. Accepted types: " \
    "Q-Expression, String, Table",                                               \
     name, (index+1), ltype_name(args->data.expr.cell[index]->type));

lval_t* builtin_type(lenv_t*, lval_t*);

// symbols
lval_t* builtin_deref(lenv_t*, lval_t*);

// list handling
lval_t* builtin_eval(lenv_t*, lval_t*);
lval_t* builtin_head(lenv_t*, lval_t*);
lval_t* builtin_tail(lenv_t*, lval_t*);
lval_t* builtin_list(lenv_t*, lval_t*);
lval_t* builtin_join(lenv_t*, lval_t*);
lval_t* builtin_len(lenv_t*, lval_t*);
lval_t* builtin_nth(lenv_t*, lval_t*);
lval_t* builtin_map(lenv_t*, lval_t*);

// tables
lval_t* builtin_el(lenv_t*, lval_t*);

// strings
lval_t* builtin_substr(lenv_t*, lval_t*);
lval_t* builtin_tosym(lenv_t*, lval_t*);
lval_t* builtin_concat(lenv_t*, lval_t*);

// vars
lval_t* builtin_def(lenv_t*, lval_t*);
lval_t* builtin_set(lenv_t*, lval_t*);
lval_t* builtin_lambda(lenv_t*, lval_t*);

// operators
lval_t* builtin_add(lenv_t*, lval_t*);
lval_t* builtin_sub(lenv_t*, lval_t*);
lval_t* builtin_mul(lenv_t*, lval_t*);
lval_t* builtin_div(lenv_t*, lval_t*);
lval_t* builtin_mod(lenv_t*, lval_t*);
lval_t* builtin_not(lenv_t*, lval_t*);
lval_t* builtin_exist(lenv_t*, lval_t*);

// conditionals
lval_t* builtin_if(lenv_t*, lval_t*);
lval_t* builtin_gt(lenv_t*, lval_t*);
lval_t* builtin_lt(lenv_t*, lval_t*);
lval_t* builtin_ge(lenv_t*, lval_t*);
lval_t* builtin_le(lenv_t*, lval_t*);
lval_t* builtin_eq(lenv_t*, lval_t*);
lval_t* builtin_ne(lenv_t*, lval_t*);

// io
lval_t* builtin_load(lenv_t*, lval_t*);
lval_t* builtin_puts(lenv_t*, lval_t*);

#endif
