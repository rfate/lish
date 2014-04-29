#include "table.h"
#include "../lenv.h"

lval_t* lval_table(void) {
  lval_t* v = malloc(sizeof(lval_t));
  v->type = LVAL_TABLE;
  v->env  = lenv_new();

  return v;
}

void lval_table_print(lval_t* v) {
  putchar('[');

  for (int i = 0; i < v->env->count; ++i) {
    if (i > 0) { printf(","); }

    printf(" %s = ", v->env->syms[i]);
    lval_print(v->env->vals[i]);
  }

  puts(" ]");
}

lval_t* lval_table_nth(lval_t* a) {
  lval_t* x;
  int length = a->data.expr.cell[0]->env->count;
  long i     = a->data.expr.cell[1]->data.num;

  // If negative index, check backwards.
  if (i < 0) {
    i = length + i;
  }
 
  // Out of bounds.
  if (i >= length || i < 0) {
    x = lval_qexpr();
  } else {
    x = lval_copy(a->data.expr.cell[0]->env->vals[i]);
  }

  lval_del(a);
  return x;
}

