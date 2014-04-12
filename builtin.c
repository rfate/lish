#include "builtin.h"

lval_t* builtin(lval_t* a, char* func) {
  if (strcmp("eval",  func) == 0) return builtin_eval(a);
  if (strcmp("list",  func) == 0) return builtin_list(a);
  if (strcmp("join",  func) == 0) return builtin_join(a);
  if (strcmp("head",  func) == 0) return builtin_head(a);
  if (strcmp("tail",  func) == 0) return builtin_tail(a);
  if (strcmp("len",   func) == 0) return builtin_len(a);
  if (strstr("+-*/%", func))      return builtin_op(a, func);

  lval_del(a);

  return lval_err("Unknown function.");
}

lval_t* builtin_op(lval_t* a, char* op) {
  for (int i = 0; i < a->count; ++i)
  {
    if (a->cell[i]->type != LVAL_NUM) {
      lval_del(a);
      return lval_err("Cannot operate on non-number.");
    }
  }
  lval_t* x = lval_pop(a, 0);

  // If sub with no args, perform negation.
  if (strcmp(op, "-") == 0 && a->count == 0)
    x->num = -x->num;

  while (a->count > 0) {
    lval_t* y = lval_pop(a, 0);

    if (strcmp(op, "+") == 0)
      x->num += y->num;
    if (strcmp(op, "-") == 0)
      x->num -= y->num;
    if (strcmp(op, "*") == 0)
      x->num *= y->num;

    if (strcmp(op, "%") == 0) {
      if (y->num == 0) {
        lval_del(x);
        lval_del(y);
        return lval_err("Division by zero.");
      } else {
        x->num = fmod(x->num, y->num);
      }
    }

    if (strcmp(op, "/") == 0) {
      if (y->num == 0) {
        lval_del(x);
        lval_del(y);
        return lval_err("Division by zero.");
      } else {
        x->num /= y->num;
      }
    }

    lval_del(y);
  }

  lval_del(a);
  return x;
}

lval_t* builtin_head(lval_t* a) {
  LASSERT_MAX_ARGS("head", a, 1);
  LASSERT_ARG_TYPE("head", a, 0, LVAL_QEXPR);
  LASSERT_NONEMPTY_LIST("head", a, 0);

  lval_t* v = lval_take(a, 0);
  while (v->count > 1)
    lval_del(lval_pop(v, 1));

  return v;
}

lval_t* builtin_tail(lval_t* a) {
  LASSERT_MAX_ARGS("tail", a, 1);
  LASSERT_ARG_TYPE("tail", a, 0, LVAL_QEXPR);
  LASSERT_NONEMPTY_LIST("tail", a, 0);

  lval_t* v = lval_take(a, 0);
  lval_del(lval_pop(v, 0));

  return v;
}

lval_t* builtin_list(lval_t* a) {
  a->type = LVAL_QEXPR;

  return a;
}

lval_t* builtin_eval(lval_t* a) {
  LASSERT_MAX_ARGS("eval", a, 1);
  LASSERT_ARG_TYPE("eval", a, 0, LVAL_QEXPR);

  lval_t* x = lval_take(a, 0);
  x->type = LVAL_SEXPR;
  return lval_eval(x);
}

lval_t* builtin_join(lval_t* a) {
  for (int i = 0; i < a->count; ++i) {
    LASSERT_ARG_TYPE("join", a, i, LVAL_QEXPR);
  }

  lval_t* x = lval_pop(a, 0);

  while (a->count)
    x = lval_join(x, lval_pop(a, 0));

  lval_del(a);
  return x;
}

lval_t* builtin_len(lval_t* a) {
  LASSERT_ARG_TYPE("len", a, 0, LVAL_QEXPR);

  lval_t* x = lval_num(a->cell[0]->count);
  lval_del(a);

  return x;
}
