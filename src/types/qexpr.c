#include "qexpr.h"

lval_t* lval_qexpr(void) {
  lval_t* v = malloc(sizeof(lval_t));
  v->type            = LVAL_QEXPR;
  v->data.expr.count = 0;
  v->data.expr.cell  = NULL;

  return v;
}

lval_t* lval_qexpr_len(lval_t* a) {
  lval_t* x = lval_int(a->data.expr.cell[0]->data.expr.count);

  lval_del(a);
  return x;
}

lval_t* lval_qexpr_nth(lval_t* a) {
  int length = a->data.expr.cell[0]->data.expr.count;
  long i     = a->data.expr.cell[1]->data.num;

  lval_t* x;

  if (i >= length || i < 0) {
    x = lval_qexpr();
  } else {
    lval_t* v = lval_take(a, 0);
    x = lval_pop(v, i);

    lval_del(v);
  }

  lval_del(a);
  return x;
}

