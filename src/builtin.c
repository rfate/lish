#include "builtin.h"
#include "lval.h"
#include "lenv.h"
#include "grammar.h"

#include "types/table.h"
#include "types/number.h"
#include "types/string.h"
#include "types/qexpr.h"

lval_t* builtin_exit(lenv_t* e, lval_t* a) {
  uint8_t code = 0;

  if (a->data.expr.cell[0]->type == LVAL_INT) {
    // And what if it's too big? oh well, right?
    code = a->data.expr.cell[0]->data.num;
  }

  lval_del(a);
  exit(code);
}

lval_t* builtin_error(lenv_t* e, lval_t* a) {
  lval_t* x;

  if (a->data.expr.count == 0 || a->data.expr.cell[0]->type != LVAL_STR) {
    x = lval_err("");
  } else {
    x = lval_err(a->data.expr.cell[0]->data.str);
  }

  lval_del(a);
  return x;
}

lval_t* builtin_deref(lenv_t* e, lval_t* a) {
  LASSERT_ARG_COUNT("deref", a, 1);
  LASSERT_ARG_TYPE("deref", a, 0, LVAL_SYM);
  
  lval_t* x = a->data.expr.cell[0];
  x->data.sym.lit = FALSE;

  lval_t* y = lval_copy(lenv_get(e, x));
  lval_del(a);

  return y;
}

lval_t* builtin_el(lenv_t* e, lval_t* a) {
  LASSERT_ARG_COUNT("el", a, 2);

  if (a->data.expr.cell[0]->type == LVAL_TABLE) {
    return lval_table_el(a);
  }

  lval_t* x = lval_err("No method \"el\" found for type %s.",
    ltype_name(a->data.expr.cell[0]->type));

  lval_del(a);
  return x;
}

lval_t* builtin_type(lenv_t* e, lval_t* a) {
  LASSERT_ARG_COUNT("type", a, 1);

  lval_t* x = lval_str(ltype_name(a->data.expr.cell[0]->type));

  lval_del(a);

  return x;
}

lval_t* builtin_concat(lenv_t* e, lval_t* a) {
  LASSERT_ARG_COUNT("concat", a, 2);
  LASSERT_ARG_TYPE("concat", a, 0, LVAL_STR);
  LASSERT_ARG_TYPE("concat", a, 1, LVAL_STR);

  char* str = malloc(strlen(a->data.expr.cell[0]->data.str) + strlen(a->data.expr.cell[1]->data.str) + 1);
  strcpy(str, a->data.expr.cell[0]->data.str);
  strcat(str, a->data.expr.cell[1]->data.str);

  lval_del(a);
  return lval_str(str);
}

// This fucking blows.
lval_t* builtin_substr(lenv_t* e, lval_t* a) {
  LASSERT_ARG_TYPE("substr", a, 0, LVAL_STR);

  lval_t* x;

  // Don't know why this would be needed, but if provided
  // only a string, return the full string.
  if (a->data.expr.count == 1) {
    x = lval_copy(a->data.expr.cell[0]);
    lval_del(a);
    return x;
  }

  LASSERT_ARG_IS_NUM("substr", a, 1);

  int length = strlen(a->data.expr.cell[0]->data.str);
  int start  = a->data.expr.cell[1]->data.num;
  int end;

  if (a->data.expr.count > 2) {
    LASSERT_ARG_IS_NUM("substr", a, 2);
    end = a->data.expr.cell[2]->data.num;
  } else {
    end = length - 1;
  }

  if (start < 0 || end < 0) {
    lval_del(a);
    return lval_err("Builtin \"substr\" doesn't know wtf to do with a negative index.");
  }

  if (start > end) {
    lval_del(a);
    return lval_err("Builtin \"substr\" requires end of range to be higher than start.");
  }

  if (start > length || end > length) {
    lval_del(a);
    return lval_err("Builtin \"substr\" tried to reach out of string bounds.");
  }

  char* newString = malloc(end - start + 1);
  strncpy(newString, &a->data.expr.cell[0]->data.str[start], end - start);
  newString[end - start] = '\0';
  
  x = lval_str(newString);

  free(newString);
  lval_del(a);

  return x;
}

lval_t* builtin_tosym(lenv_t* e, lval_t* a) {
  LASSERT_ARG_COUNT("tosym", a, 1);
  LASSERT_ARG_TYPE("tosym", a, 0, LVAL_STR);

  return lval_str_tosym(a);
}

lval_t*
builtin_bigint_op(lenv_t *e, lval_t *a, char *op)
{
  // Args may be int or float, so we must convert to bigint.
  lval_t *args = lval_qexpr();

  for (int i = 0; i < a->data.expr.count; ++i) {
    if (a->data.expr.cell[i]->type == LVAL_BIGINT) {
      args = lval_add(args, lval_copy(a->data.expr.cell[i]));
      continue;
    }

    if (a->data.expr.cell[i]->type == LVAL_INT) {
      lval_t *v = lval_bigint("0");
      mpz_set_si(v->data.bignum, (long int) a->data.expr.cell[i]->data.num);
      args = lval_add(args, v);
      continue;
    }

    if (a->data.expr.cell[i]->type == LVAL_FLOAT) {
      lval_t *v = lval_bigint("0");
      mpz_set_d(v->data.bignum, a->data.expr.cell[i]->data.num);
      args = lval_add(args, v);
      continue;
    }

    lval_t *err = lval_err("Cannot operate on non-numberic type %s",
      ltype_name(a->data.expr.cell[i]->type));

    lval_del(a);
    lval_del(args);

    return err;
  }

  lval_del(a);

  lval_t *x = lval_pop(args, 0);

  // Unary negate and posit.
  if (strcmp(op, "-") == 0 && args->data.expr.count == 0)
    mpz_neg(x->data.bignum, x->data.bignum);
  if (strcmp(op, "+") == 0 && args->data.expr.count == 0)
    mpz_abs(x->data.bignum, x->data.bignum);

  while (args->data.expr.count > 0) {
    lval_t *y = lval_pop(args, 0);

    if (strcmp(op, "+") == 0)
      mpz_add(x->data.bignum, x->data.bignum, y->data.bignum);
    if (strcmp(op, "-") == 0)
      mpz_sub(x->data.bignum, x->data.bignum, y->data.bignum);

    if (strcmp(op, "*") == 0)
      mpz_mul(x->data.bignum, x->data.bignum, y->data.bignum);

    if (strcmp(op, "/") == 0) {
      if (mpz_cmp_d(y->data.bignum, 0) == 0) {
        lval_del(x);
        lval_del(y);
        lval_del(args);
        return lval_err("Division by zero.");
      }

      mpz_fdiv_q(x->data.bignum, x->data.bignum, y->data.bignum);
    }

    if (strcmp(op, "%") == 0) {
      if (mpz_cmp_d(y->data.bignum, 0) == 0) {
        lval_del(x);
        lval_del(y);
        lval_del(args);
        return lval_err("Division by zero");
      }

      mpz_mod(x->data.bignum, x->data.bignum, y->data.bignum);
    }

    lval_del(y);
  }

  lval_del(args);

  return x;
}

lval_t*
builtin_op(lenv_t *e, lval_t *a, char *op)
{
  for (int i = 0; i < a->data.expr.count; ++i) {
    // If _any_ number is a bigint, we must use the special builtin.
    if (a->data.expr.cell[i]->type == LVAL_BIGINT) {
      return builtin_bigint_op(e, a, op);
    }

    // Otherwise, it must be an int or float.
    LASSERT_ARG_IS_NUM(op, a, i);
  }

  lval_t *x = lval_pop(a, 0);

  // If sub with no args, perform negation.
  if (strcmp(op, "-") == 0 && a->data.expr.count == 0)
    x->data.num = -x->data.num;

  if (strcmp(op, "+") == 0 && a->data.expr.count == 0)
    x->data.num = (x->data.num < 0) ? -x->data.num : x->data.num;

  while (a->data.expr.count > 0) {
    lval_t *y = lval_pop(a, 0);

    if (strcmp(op, "+") == 0)
      x->data.num += y->data.num;
    if (strcmp(op, "-") == 0)
      x->data.num -= y->data.num;
    if (strcmp(op, "*") == 0)
      x->data.num *= y->data.num;

    if (strcmp(op, "%") == 0) {
      if (y->data.num == 0) {
        lval_del(x);
        lval_del(y);
        return lval_err("Division by zero.");
      } else {
        x->data.num = fmod(x->data.num, y->data.num);
      }
    }

    if (strcmp(op, "/") == 0) {
      if (y->data.num == 0) {
        lval_del(x);
        lval_del(y);
        return lval_err("Division by zero.");
      } else {
        x->data.num /= y->data.num;
      }
    }

    lval_del(y);
  }

  lval_del(a);

  // if we have a remainder, convert int to float
  if (x->type == LVAL_INT && (fmod(x->data.num, 1) > 0))
    x->type = LVAL_FLOAT;

  return x;
}

lval_t* builtin_add(lenv_t* e, lval_t* v) { return builtin_op(e, v, "+"); }
lval_t* builtin_sub(lenv_t* e, lval_t* v) { return builtin_op(e, v, "-"); }
lval_t* builtin_mul(lenv_t* e, lval_t* v) { return builtin_op(e, v, "*"); }
lval_t* builtin_div(lenv_t* e, lval_t* v) { return builtin_op(e, v, "/"); }
lval_t* builtin_mod(lenv_t* e, lval_t* v) { return builtin_op(e, v, "%"); }

lval_t* builtin_not(lenv_t* e, lval_t* a) {
  LASSERT_ARG_COUNT("not", a, 1);

  lval_t* x = lval_truthy(a->data.expr.cell[0]);

  x->data.num = !x->data.num;

  lval_del(a);
  return x;
}

lval_t* builtin_exist(lenv_t* e, lval_t* a) {
  LASSERT_ARG_COUNT("exist", a, 1);

  lval_t* x = lval_truthy(a->data.expr.cell[0]);

  lval_del(a);
  return x;
}

lval_t* builtin_if(lenv_t* e, lval_t* a) {
  LASSERT_ARG_COUNT("if", a, 3);
  LASSERT_ARG_TYPE("if", a, 0, LVAL_BOOL);
  LASSERT_ARG_TYPE("if", a, 1, LVAL_QEXPR);
  LASSERT_ARG_TYPE("if", a, 2, LVAL_QEXPR);

  // Mark expressions as evaluable.
  lval_t* x;
  a->data.expr.cell[1]->type = LVAL_SEXPR;
  a->data.expr.cell[2]->type = LVAL_SEXPR;

  if (a->data.expr.cell[0]->data.num != FALSE) {
    x = lval_eval(e, lval_pop(a, 1));
  } else {
    x = lval_eval(e, lval_pop(a, 2));
  }

  lval_del(a);
  return x;
}

lval_t* builtin_ord(lenv_t* e, lval_t* a, char* op) {
  LASSERT_ARG_COUNT(op, a, 2);
  LASSERT_ARG_IS_NUM(op, a, 0);
  LASSERT_ARG_IS_NUM(op, a, 1);

  int r;
  if (strcmp(op, ">")  == 0) { r = (a->data.expr.cell[0]->data.num >  a->data.expr.cell[1]->data.num); }
  if (strcmp(op, "<")  == 0) { r = (a->data.expr.cell[0]->data.num <  a->data.expr.cell[1]->data.num); }
  if (strcmp(op, ">=") == 0) { r = (a->data.expr.cell[0]->data.num >= a->data.expr.cell[1]->data.num); }
  if (strcmp(op, "<=") == 0) { r = (a->data.expr.cell[0]->data.num <= a->data.expr.cell[1]->data.num); }

  lval_del(a);

  return lval_bool(r);
}

lval_t* builtin_gt(lenv_t* e, lval_t* a) { return builtin_ord(e, a, ">");  }
lval_t* builtin_lt(lenv_t* e, lval_t* a) { return builtin_ord(e, a, "<");  }
lval_t* builtin_ge(lenv_t* e, lval_t* a) { return builtin_ord(e, a, ">="); }
lval_t* builtin_le(lenv_t* e, lval_t* a) { return builtin_ord(e, a, "<="); }

lval_t* builtin_cmp(lenv_t* e, lval_t* a, char* op) {
  LASSERT_ARG_COUNT(op, a, 2);

  int r;
  if (strcmp(op, "==") == 0) { r =  lval_eq(a->data.expr.cell[0], a->data.expr.cell[1]); }
  if (strcmp(op, "!=") == 0) { r = !lval_eq(a->data.expr.cell[0], a->data.expr.cell[1]); }

  lval_del(a);
  return lval_bool(r);
}

lval_t* builtin_eq(lenv_t* e, lval_t* a) { return builtin_cmp(e, a, "=="); }
lval_t* builtin_ne(lenv_t* e, lval_t* a) { return builtin_cmp(e, a, "!="); }

lval_t* builtin_load(lenv_t* e, lval_t* a) {
  LASSERT_ARG_TYPE("load", a, 0, LVAL_STR);

  uint8_t usePath = FALSE;

  // If given a bool to specify loading from path...
  if (a->data.expr.count > 1) {
    LASSERT_ARG_TYPE("load", a, 1, LVAL_BOOL);
    usePath = a->data.expr.cell[1]->data.num;
  }

  char* dir;
  if (usePath) {
    dir = LISH_INSTALL_PATH;
  } else {
    dir = "";
  }

  char* path = malloc(strlen(dir) + strlen(a->data.expr.cell[0]->data.str) + 1);
  strcpy(path, dir);
  strcat(path, a->data.expr.cell[0]->data.str);

  mpc_result_t r;
  if (mpc_parse_contents(path, grammar_lish, &r)) {
    lval_t* expr = lval_read(r.output);
    mpc_ast_delete(r.output);

    while (expr->data.expr.count) {
      lval_t* x = lval_eval(e, lval_pop(expr, 0));

      if (x->type == LVAL_ERR)
        lval_println(x);

      lval_del(x);
    }

    lval_del(expr);
    lval_del(a);
    free(path);

    return lval_bool(TRUE);
  }

  char* err_msg = mpc_err_string(r.error);
  mpc_err_delete(r.error);
  
  lval_t* err = lval_err("Could not load file '%s'.", err_msg);
  free(err_msg);
  lval_del(a);
  free(path);

  return err;
}

lval_t* builtin_head(lenv_t* e, lval_t* a) {
  LASSERT_ARG_COUNT("head", a, 1);
  LASSERT_ARG_TYPE("head", a, 0, LVAL_QEXPR);
  LASSERT_NONEMPTY_LIST("head", a, 0);

/*  lval_t* v = lval_take(a, 0);
  while (v->data.expr.count > 1)
    lval_del(lval_pop(v, 1));*/

  lval_t* v = lval_copy(a->data.expr.cell[0]->data.expr.cell[0]);

  lval_del(a);

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

/*
 * (nth {42 64 32} 1)       => 64
 * (nth {42 64 32} 12)      => {}
 * (nth [x=4, y=9, z=12] 2) => 9
 */
lval_t* builtin_nth(lenv_t* e, lval_t* a) {
  LASSERT_ARG_COUNT("nth", a, 2);
  LASSERT_ARG_ITERABLE("nth", a, 0);
  LASSERT_ARG_IS_NUM("nth", a, 1);


  if (a->data.expr.cell[0]->type == LVAL_TABLE)
    return lval_table_nth(a);

  if (a->data.expr.cell[0]->type == LVAL_QEXPR)
    return lval_qexpr_nth(a);

  if (a->data.expr.cell[0]->type == LVAL_STR)
    return lval_str_nth(a);

  // This should never be possible.
  lval_t* x = lval_err("Builtin \"nth\" reached default case. given type: %s",
    ltype_name(a->data.expr.cell[0]->type));

  lval_del(a);
  return x;

}

lval_t* builtin_map(lenv_t* e, lval_t* a) {
  LASSERT_ARG_COUNT("map", a, 2);
  LASSERT_ARG_TYPE("map", a, 0, LVAL_QEXPR);
  LASSERT_ARG_TYPE("map", a, 1, LVAL_FUN);

  lval_t* v = lval_qexpr();
  
  for (int i = 0; i < a->data.expr.cell[0]->data.expr.count; ++i) {
    lval_t* x = lval_sexpr();
    x = lval_add(x, lval_copy(a->data.expr.cell[1]));
    x = lval_add(x, a->data.expr.cell[0]->data.expr.cell[i]);
    v = lval_add(v, lval_eval(e, x));
  }

  return v;
}

lval_t* builtin_eval(lenv_t* e, lval_t* a) {
  LASSERT_ARG_COUNT("eval", a, 1);
  LASSERT_ARG_TYPE("eval", a, 0, LVAL_QEXPR);

  lval_t* x = lval_take(a, 0);
  x->type = LVAL_SEXPR;
  return lval_eval(e, x);
}

lval_t* builtin_join(lenv_t* e, lval_t* a) {
  for (int i = 0; i < a->data.expr.count; ++i) {
    LASSERT_ARG_TYPE("join", a, i, LVAL_QEXPR);
  }

  lval_t* x = lval_pop(a, 0);

  while (a->data.expr.count)
    x = lval_join(x, lval_pop(a, 0));

  lval_del(a);
  return x;
}

lval_t* builtin_len(lenv_t* e, lval_t* a) {
  LASSERT_ARG_COUNT("len", a, 1);
  
  switch (a->data.expr.cell[0]->type) {
    case LVAL_QEXPR: return lval_qexpr_len(a);
    case LVAL_STR:   return lval_str_len(a);
    case LVAL_TABLE: return lval_table_len(a);
  }

  lval_t* x = lval_err("Type %s has no length.",
    ltype_name(a->data.expr.cell[0]->type));

  lval_del(a);
  return x;
}

lval_t* builtin_var(lenv_t* e, lval_t* a, char* func) {
  LASSERT_ARG_COUNT(func, a, 2);
  LASSERT_ARG_TYPE(func, a, 0, LVAL_SYM);

  if (strcmp(func, "=")   == 0)
    lenv_set(e, a->data.expr.cell[0], a->data.expr.cell[1]);
  if (strcmp(func, "def") == 0)
    lenv_def(e, a->data.expr.cell[0], a->data.expr.cell[1]);

  lval_del(a);

  return lval_sexpr();
}

lval_t* builtin_def(lenv_t* e, lval_t* v) { return builtin_var(e, v, "="); }
lval_t* builtin_set(lenv_t* e, lval_t* v) { return builtin_var(e, v, "def");   }

lval_t* builtin_puts(lenv_t* e, lval_t* v) {
  LASSERT_ARG_COUNT("puts", v, 1);

  // Strings need to be handled differently so they aren't escaped.
  if (v->data.expr.cell[0]->type == LVAL_STR) {
    printf("%s\n", v->data.expr.cell[0]->data.str);
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

  for (int i = 0 ; i < a->data.expr.cell[0]->data.expr.count; ++i) {
    LASSERT(a, (a->data.expr.cell[0]->data.expr.cell[i]->type == LVAL_SYM),
      "Cannot define non-symbol. Expected %s, got %s.");
  }

  lval_t* formals = lval_pop(a, 0);
  lval_t* body    = lval_pop(a, 0);
  lval_del(a);

  return lval_lambda(formals, body);
}

