#include "builtin.h"
#include "lval.h"
#include "lenv.h"
#include "parser.h"

lval_t* builtin_op(lenv_t* e, lval_t* a, char* op) {
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

  if (strcmp(op, "+") == 0 && a->count == 0)
    x->num = (x->num < 0) ? -x->num : x->num;

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

lval_t* builtin_add(lenv_t* e, lval_t* v) { return builtin_op(e, v, "+"); }
lval_t* builtin_sub(lenv_t* e, lval_t* v) { return builtin_op(e, v, "-"); }
lval_t* builtin_mul(lenv_t* e, lval_t* v) { return builtin_op(e, v, "*"); }
lval_t* builtin_div(lenv_t* e, lval_t* v) { return builtin_op(e, v, "/"); }
lval_t* builtin_mod(lenv_t* e, lval_t* v) { return builtin_op(e, v, "%"); }

lval_t* builtin_load(lenv_t* e, lval_t* a) {
  LASSERT_ARG_COUNT("load", a, 1);
  LASSERT_ARG_TYPE("load", a, 0, LVAL_STR);

  mpc_result_t r;

  if (mpc_parse_contents(a->cell[0]->str, Lish, &r)) {
    lval_t* expr = lval_read(r.output);
    mpc_ast_delete(r.output);

    while (expr->count) {
      lval_t* x = lval_eval(e, lval_pop(expr, 0));

      if (x->type == LVAL_ERR)
        lval_println(x);

      lval_del(x);
    }

    lval_del(expr);
    lval_del(a);

    return lval_sexpr();
  }

  char* err_msg = mpc_err_string(r.error);
  mpc_err_delete(r.error);
  
  lval_t* err = lval_err("Could not load file '%s'.", err_msg);
  free(err_msg);
  lval_del(a);

  return err;
}

lval_t* builtin_head(lenv_t* e, lval_t* a) {
  LASSERT_ARG_COUNT("head", a, 1);
  LASSERT_ARG_TYPE("head", a, 0, LVAL_QEXPR);
  LASSERT_NONEMPTY_LIST("head", a, 0);

  lval_t* v = lval_take(a, 0);
  while (v->count > 1)
    lval_del(lval_pop(v, 1));

  return v;
}

lval_t* builtin_tail(lenv_t* e, lval_t* a) {
  LASSERT_ARG_COUNT("tail", a, 1);
  LASSERT_ARG_TYPE("tail", a, 0, LVAL_QEXPR);
  LASSERT_NONEMPTY_LIST("tail", a, 0);

  lval_t* v = lval_take(a, 0);
  lval_del(lval_pop(v, 0));

  return v;
}

lval_t* builtin_list(lenv_t* e, lval_t* a) {
  a->type = LVAL_QEXPR;

  return a;
}

lval_t* builtin_eval(lenv_t* e, lval_t* a) {
  LASSERT_ARG_COUNT("eval", a, 1);
  LASSERT_ARG_TYPE("eval", a, 0, LVAL_QEXPR);

  lval_t* x = lval_take(a, 0);
  x->type = LVAL_SEXPR;
  return lval_eval(e, x);
}

lval_t* builtin_join(lenv_t* e, lval_t* a) {
  for (int i = 0; i < a->count; ++i) {
    LASSERT_ARG_TYPE("join", a, i, LVAL_QEXPR);
  }

  lval_t* x = lval_pop(a, 0);

  while (a->count)
    x = lval_join(x, lval_pop(a, 0));

  lval_del(a);
  return x;
}

lval_t* builtin_len(lenv_t* e, lval_t* a) {
  LASSERT_ARG_TYPE("len", a, 0, LVAL_QEXPR);

  lval_t* x = lval_num(a->cell[0]->count);
  lval_del(a);

  return x;
}

lval_t* builtin_var(lenv_t* e, lval_t* a, char* func) {
  LASSERT_ARG_TYPE("var??", a, 0, LVAL_QEXPR);

  lval_t* syms = a->cell[0];

  for (int i = 0; i < syms->count; ++i) {
    LASSERT(a, (syms->cell[i]->type == LVAL_SYM), "Builtin '%s' cannot define non-symbol.", func);
  }

  LASSERT(a, (syms->count == a->count-1), "Builtin '%s' cannot assign incorrect number of values to symbols. Expected %d, got %d.",
    func, syms->count, a->count-1);

  for (int i = 0; i < syms->count; ++i) {
    if (strcmp(func, "def") == 0)
      lenv_set(e, syms->cell[i], a->cell[i+1]);
    if (strcmp(func, "=") == 0)
      lenv_def(e, syms->cell[i], a->cell[i+1]);
  }

  lval_del(a);

  return lval_sexpr();
}

lval_t* builtin_def(lenv_t* e, lval_t* v) { return builtin_var(e, v, "def"); }
lval_t* builtin_set(lenv_t* e, lval_t* v) { return builtin_var(e, v, "=");   }

lval_t* builtin_puts(lenv_t* e, lval_t* v) {
  LASSERT_ARG_COUNT("puts", v, 1);

  if (v->cell[0]->type == LVAL_STR) {
    printf("%s", v->cell[0]->str);
  } else {
    lval_println(v);
  }
  lval_del(v);

  return lval_sexpr();
}

lval_t* builtin_lambda(lenv_t* e, lval_t* a) {
  LASSERT_ARG_COUNT("lambda", a, 2);
  LASSERT_ARG_TYPE("lambda", a, 0, LVAL_QEXPR);
  LASSERT_ARG_TYPE("lambda", a, 1, LVAL_QEXPR);

  for (int i = 0 ; i < a->cell[0]->count; ++i) {
    LASSERT(a, (a->cell[0]->cell[i]->type == LVAL_SYM),
      "Cannot define non-symbol. Expected %s, got %s.");
  }

  lval_t* formals = lval_pop(a, 0);
  lval_t* body    = lval_pop(a, 0);
  lval_del(a);

  return lval_lambda(formals, body);
}

