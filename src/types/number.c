#include "number.h"

lval_t*
lval_int(long int x)
{
  lval_t *v = malloc(sizeof(lval_t));
  v->type     = LVAL_INT;
  v->data.num = x;

  return v;
}

lval_t*
lval_float(double x)
{
  lval_t *v = malloc(sizeof(lval_t));
  v->type     = LVAL_FLOAT;
  v->data.num = x;

  return v;
}

lval_t*
lval_bigint(char *x)
{
  lval_t *v = malloc(sizeof(lval_t));
  v->type = LVAL_BIGINT;
  mpz_init_set_str(v->data.bignum, x, 10);

  return v;
}

