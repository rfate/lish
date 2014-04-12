#ifndef _LISH_BUILTIN_H
#define _LISH_BUILTIN_H

struct lval_t;
struct lenv_t;
typedef struct lval_t lval_t;
typedef struct lenv_t lenv_t;

#define LASSERT(args, cond, err) \
  if (!(cond)) {                 \
    lval_del(args);              \
    return lval_err(err);        \
  }

#define LASSERT_MAX_ARGS(name, args, max) \
  LASSERT(args, args->count == max,       \
    "Builtin '" name "' given too many arguments.")

#define LASSERT_NONEMPTY_LIST(name, args, argn) \
  LASSERT(args, args->cell[argn]->count != 0,   \
    "Builtin '" name "' given empty list.")

#define LASSERT_ARG_TYPE(name, args, argn, _type) \
  LASSERT(args, args->cell[argn]->type == _type,  \
    "Builtin '" name "' given incorrect type.");

lval_t* builtin_op  (lenv_t*, lval_t*, char*);

lval_t* builtin_eval(lenv_t*, lval_t*);
lval_t* builtin_head(lenv_t*, lval_t*);
lval_t* builtin_tail(lenv_t*, lval_t*);
lval_t* builtin_list(lenv_t*, lval_t*);
lval_t* builtin_join(lenv_t*, lval_t*);
lval_t* builtin_len(lenv_t*, lval_t*);

lval_t* builtin_def(lenv_t*, lval_t*);

lval_t* builtin_add(lenv_t*, lval_t*);
lval_t* builtin_sub(lenv_t*, lval_t*);
lval_t* builtin_mul(lenv_t*, lval_t*);
lval_t* builtin_div(lenv_t*, lval_t*);
lval_t* builtin_mod(lenv_t*, lval_t*);

#endif
