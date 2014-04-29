#include "str.h"

lval_t* lval_str(char* str) {
  lval_t* v = malloc(sizeof(lval_t));
  v->type     = LVAL_STR;
  v->data.str = malloc(strlen(str) + 1);
  strcpy(v->data.str, str);
  
  return v;
}

void lval_str_print(lval_t* v) {
  char* escaped = malloc(strlen(v->data.str) + 1);
  strcpy(escaped, v->data.str);

  escaped = mpcf_escape(escaped);

  printf("\"%s\"", escaped);

  free(escaped);
}

lval_t* lval_str_nth(lval_t* a) {
  long i     = a->data.expr.cell[1]->data.num;
  int length = strlen(a->data.expr.cell[0]->data.str);

  lval_t* x;

  if (i < 0) {
    i = length + i;
  }

  // Out of bounds.
  if (i >= length || i < 0) {
    x = lval_qexpr();
  } else {
    char* str = malloc(2);
    strncpy(str, a->data.expr.cell[0]->data.str + i, 1);
    str[1] = '\0';
    x = lval_str(str);
    free(str);
  }

  lval_del(a);
  return x;
}

