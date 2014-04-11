#ifndef _LISH_LISH_H
#define _LISH_LISH_H

#define LISH_VERSION "v0.0.0"

typedef struct {
  int  type;
  double num;
  int  err;
} lval_t;

enum {
  LVAL_NUM = 0,
  LVAL_ERR,
};

enum {
  LERR_DIV_ZERO = 0,
  LERR_BAD_OP,
  LERR_BAD_NUM,
};


lval_t eval_op(char*, lval_t, lval_t);

#endif
