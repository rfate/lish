#include "string.h"
#include "number.h"

lval_t*
lval_str(char *str)
{
  lval_t *v = malloc(sizeof(lval_t));
  v->type     = LVAL_STR;
  v->str = malloc(strlen(str) + 1);
  strcpy(v->str, str);
  
  return v;
}

void
lval_str_print(lval_t *v)
{
  char *escaped = malloc(strlen(v->str) + 1);
  strcpy(escaped, v->str);

  escaped = mpcf_escape(escaped);

  printf("\"%s\"", escaped);

  free(escaped);
}

lval_t*
lval_str_nth(lval_t *a)
{
  long i     = a->expr.cell[1]->num;
  int length = strlen(a->expr.cell[0]->str);

  lval_t *x;

  if (i < 0) {
    i = length + i;
  }

  // Out of bounds.
  if (i >= length || i < 0) {
    x = lval_nil();
  } else {
    char *str = malloc(2);
    strncpy(str, a->expr.cell[0]->str + i, 1);
    str[1] = '\0';
    x = lval_str(str);
    free(str);
  }

  lval_del(a);
  return x;
}

lval_t *lval_str_len(lval_t *a)
{
  lval_t *x = lval_int(strlen(a->expr.cell[0]->str));

  lval_del(a);
  return x;
}

lval_t*
lval_str_tosym(lval_t *a)
{
  lval_t *x;

  if (strlen(a->expr.cell[0]->str) == 0) {
    x = lval_err("Cannot define empty symbol.");
  } else {
    x = lval_sym(a->expr.cell[0]->str, 1);
  }

  lval_del(a);
  return x;
}

