#ifndef _LISH_BUILTIN_H
#define _LISH_BUILTIN_H

#include "lval.h"

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
    "Builtin '" name "' given incorrect type.")

lval_t* builtin(lval_t* a, char*);

lval_t* builtin_op  (lval_t*, char*);
lval_t* builtin_head(lval_t*);
lval_t* builtin_tail(lval_t*);
lval_t* builtin_eval(lval_t*);
lval_t* builtin_list(lval_t*);
lval_t* builtin_join(lval_t*);
lval_t* builtin_len (lval_t*);

#endif
