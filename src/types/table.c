#include "table.h"
#include "../lenv.h"

lval_t*
lval_table(void)
{
  lval_t *v = malloc(sizeof(lval_t));
  v->type             = LVAL_TABLE;
  v->data.table.count = 0;
  v->data.table.keys  = NULL;
  v->data.table.vals  = NULL;

  return v;
}

void
lval_table_del(lval_t *v)
{
  for (int i = 0; i < v->data.table.count; ++i) {
    lval_del(v->data.table.keys[i]);
    lval_del(v->data.table.vals[i]);
  }

  free(v->data.table.keys);
  free(v->data.table.vals);
  free(v);
}

lval_t*
lval_table_copy(lval_t *v)
{
  lval_t* n = malloc(sizeof(lval_t));
  n->type             = LVAL_TABLE;
  n->data.table.count = v->data.table.count;

  n->data.table.keys = malloc(sizeof(lval_t*) * n->data.table.count);
  n->data.table.vals = malloc(sizeof(lval_t*) * n->data.table.count);

  for (int i = 0; i < n->data.table.count; ++i) {
    n->data.table.keys[i] = lval_copy(v->data.table.keys[i]);
    n->data.table.vals[i] = lval_copy(v->data.table.vals[i]);
  }

  return n;
}

void
lval_table_set(lval_t *t, lval_t *k, lval_t *v)
{
  for (int i = 0; i < t->data.table.count; ++i) {
    if (lval_eq(t->data.table.keys[i], k)) {
      lval_del(t->data.table.vals[i]);
      t->data.table.vals[i] = lval_copy(v);
      return;
    }
  }

  t->data.table.count++;
  t->data.table.keys = realloc(t->data.table.keys, sizeof(lval_t*) * t->data.table.count);
  t->data.table.vals = realloc(t->data.table.vals, sizeof(lval_t*) * t->data.table.count);

  t->data.table.keys[t->data.table.count - 1] = lval_copy(k);
  t->data.table.vals[t->data.table.count - 1] = lval_copy(v);
}

lval_t*
lval_table_get(lval_t *t, lval_t *k)
{
  lval_t *x = NULL;

  for (int i = 0; i < t->data.table.count; ++i) {
    if (lval_eq(k, t->data.table.keys[i])) {
      x = lval_copy(t->data.table.vals[i]);
      break;
    }
  }

  if (x == NULL)
    x = lval_nil();

  return x;
}

void
lval_table_print(lval_t* v)
{
  putchar('[');

  for (int i = 0; i < v->data.table.count; ++i) {
    if (i > 0)
      putchar(',');

    putchar(' ');
    lval_print(v->data.table.keys[i]);
    printf(" = ");
    lval_print(v->data.table.vals[i]);
  }

  printf(" ]");
}

lval_t*
lval_table_nth(lval_t *a)
{
  lval_t *x;
  int length = a->data.expr.cell[0]->data.table.count;
  long int i = a->data.expr.cell[1]->data.num;

  // If negative index, check backwards.
  if (i < 0) {
    i = length + i;
  }
 
  // Out of bounds.
  if (i >= length || i < 0) {
    x = lval_nil();
  } else {
    x = lval_copy(a->data.expr.cell[0]->data.table.vals[i]);
  }

  lval_del(a);
  return x;
}

lval_t*
lval_table_el(lval_t *a)
{
  lval_t *x = lval_table_get(a->data.expr.cell[0], a->data.expr.cell[1]);

  lval_del(a);
  return x;
}

lval_t*
lval_table_len(lval_t *a)
{
  lval_t *x = lval_int(a->data.expr.cell[0]->data.table.count);

  lval_del(a);
  return x;
}

