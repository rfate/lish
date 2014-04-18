#ifndef _LISH_BUILTIN_H
#define _LISH_BUILTIN_H

struct lval_t;
struct lenv_t;
typedef struct lval_t lval_t;
typedef struct lenv_t lenv_t;

char* ltype_name(int);

#define LASSERT(args, cond, fmt, ...)    \
  if (!(cond)) {                         \
    lval_del(args);                      \
    return lval_err(fmt, ##__VA_ARGS__); \
  }

#define LASSERT_ARG_COUNT(name, args, c) \
  LASSERT(args, args->count == c,        \
    "Builtin '" name "' given incorrect number of arguments. Expected %d, got %d", c, args->count)

#define LASSERT_NONEMPTY_LIST(name, args, argn) \
  LASSERT(args, args->cell[argn]->count != 0,   \
    "Builtin '" name "' given empty list.")

#define LASSERT_ARG_TYPE(name, args, index, _type)                  \
  LASSERT(args, args->cell[index]->type == _type,                   \
    "Builtin '" name "' expected arg %d to be of type %s, got %s.", \
    index, ltype_name(_type), ltype_name(args->cell[index]->type));

lval_t* builtin_op  (lenv_t*, lval_t*, char*);

// list handling
lval_t* builtin_eval(lenv_t*, lval_t*);
lval_t* builtin_head(lenv_t*, lval_t*);
lval_t* builtin_tail(lenv_t*, lval_t*);
lval_t* builtin_list(lenv_t*, lval_t*);
lval_t* builtin_join(lenv_t*, lval_t*);
lval_t* builtin_len(lenv_t*, lval_t*);

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