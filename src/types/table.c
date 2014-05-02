#include "table.h"
#include "../lenv.h"

lval_t* lval_table(void) {
  lval_t* v = malloc(sizeof(lval_t));
  v->type           = LVAL_TABLE;
  v->data.func.env  = lenv_new();

  return v;
}

void lval_table_print(lval_t* v) {
  putchar('[');

  for (int i = 0; i < v->data.func.env->count; ++i) {
    if (i > 0) { printf(","); }

    printf(" %s = ", v->data.func.env->syms[i]);
    lval_print(v->data.func.env->vals[i]);
  }

  puts(" ]");
}

lval_t* lval_table_nth(lval_t* a) {
  lval_t* x;
  int length = a->data.expr.cell[0]->data.func.env->count;
  long i     = a->data.expr.cell[1]->data.num;

  // If negative index, check backwards.
  if (i < 0) {
    i = length + i;
  }
 
  // Out of bounds.
  if (i >= length || i < 0) {
    x = lval_qexpr();
  } else {
    x = lval_copy(a->data.expr.cell[0]->data.func.env->vals[i]);
  }

  lval_del(a);
  return x;
}

lval_t* lval_table_el(lval_t* a) {
  lval_t* x;

  if (a->data.expr.cell[1]->type != LVAL_STR
   && a->data.expr.cell[1]->type != LVAL_SYM) {
    x = lval_err("Builtin \"el\" cannot access index of non-index type %s",
      ltype_name(a->data.expr.cell[1]->type));
  } else {
    x = lenv_get(a->data.expr.cell[0]->data.func.env, a->data.expr.cell[1]);
  }

  lval_del(a);
  return x;
}


