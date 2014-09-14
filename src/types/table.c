#include "table.h"
#include "number.h"
#include "../lenv.h"

lval_t*
lval_table(void)
{
  lval_t *v = malloc(sizeof(lval_t));
  v->type             = LVAL_TABLE;
  v->table.count = 0;
  v->table.keys  = NULL;
  v->table.vals  = NULL;

  return v;
}

void
lval_table_del(lval_t *v)
{
  for (int i = 0; i < v->table.count; ++i) {
    lval_del(v->table.keys[i]);
    lval_del(v->table.vals[i]);
  }

  free(v->table.keys);
  free(v->table.vals);
  free(v);
}

lval_t*
lval_table_copy(lval_t *v)
{
  lval_t* n = malloc(sizeof(lval_t));
  n->type             = LVAL_TABLE;
  n->table.count = v->table.count;

  n->table.keys = malloc(sizeof(lval_t*) * n->table.count);
  n->table.vals = malloc(sizeof(lval_t*) * n->table.count);

  for (int i = 0; i < n->table.count; ++i) {
    n->table.keys[i] = lval_copy(v->table.keys[i]);
    n->table.vals[i] = lval_copy(v->table.vals[i]);
  }

  return n;
}

void
lval_table_set(lval_t *t, lval_t *k, lval_t *v)
{
  for (int i = 0; i < t->table.count; ++i) {
    if (lval_eq(t->table.keys[i], k)) {
      lval_del(t->table.vals[i]);
      t->table.vals[i] = lval_copy(v);
      return;
    }
  }

  t->table.count++;
  t->table.keys = realloc(t->table.keys, sizeof(lval_t*) * t->table.count);
  t->table.vals = realloc(t->table.vals, sizeof(lval_t*) * t->table.count);

  t->table.keys[t->table.count - 1] = lval_copy(k);
  t->table.vals[t->table.count - 1] = lval_copy(v);
}

lval_t*
lval_table_get(lval_t *t, lval_t *k)
{
  lval_t *x = NULL;

  for (int i = 0; i < t->table.count; ++i) {
    if (lval_eq(k, t->table.keys[i])) {
      x = lval_copy(t->table.vals[i]);
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

  for (int i = 0; i < v->table.count; ++i) {
    if (i > 0)
      putchar(',');

    putchar(' ');
    lval_print(v->table.keys[i]);
    printf(" = ");
    lval_print(v->table.vals[i]);
  }

  printf(" ]");
}

lval_t*
lval_table_nth(lval_t *a)
{
  lval_t *x;
  int length = a->expr.cell[0]->table.count;
  long int i = a->expr.cell[1]->num;

  // If negative index, check backwards.
  if (i < 0) {
    i = length + i;
  }
 
  // Out of bounds.
  if (i >= length || i < 0) {
    x = lval_nil();
  } else {
    x = lval_copy(a->expr.cell[0]->table.vals[i]);
  }

  lval_del(a);
  return x;
}

lval_t*
lval_table_el(lval_t *a)
{
  lval_t *x = lval_table_get(a->expr.cell[0], a->expr.cell[1]);

  lval_del(a);
  return x;
}

lval_t*
lval_table_len(lval_t *a)
{
  lval_t *x = lval_int(a->expr.cell[0]->table.count);

  lval_del(a);
  return x;
}

