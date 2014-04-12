#include <math.h>
#include "lval.h"
#include "builtin.h"

/// Constructors
lval_t* lval_num(double x) {
  lval_t* v = malloc(sizeof(lval_t));
  v->type = LVAL_NUM;
  v->num  = x;

  return v;
}

lval_t* lval_err(char* err) {
  lval_t* v = malloc(sizeof(lval_t));
  v->type = LVAL_ERR;
  v->err  = malloc(strlen(err) + 1);
  strcpy(v->err, err);

  return v;
}

lval_t* lval_sym(char* sym) {
  lval_t* v = malloc(sizeof(lval_t));
  v->type = LVAL_SYM;
  v->sym  = malloc(strlen(sym) + 1);
  strcpy(v->sym, sym);

  return v;
}

lval_t* lval_sexpr(void) {
  lval_t* v = malloc(sizeof(lval_t));
  v->type  = LVAL_SEXPR;
  v->count = 0;
  v->cell  = NULL;

  return v;
}

lval_t* lval_qexpr(void) {
  lval_t* v = malloc(sizeof(lval_t));
  v->type  = LVAL_QEXPR;
  v->count = 0;
  v->cell  = NULL;

  return v;
}
/// 

void lval_del(lval_t* v) {
  switch (v->type) {
    case LVAL_NUM: break;

    case LVAL_ERR: free(v->err); break;
    case LVAL_SYM: free(v->sym); break;

    case LVAL_SEXPR:
    case LVAL_QEXPR:
      for (int i = 0; i < v->count; ++i) {
        lval_del(v->cell[i]);
      }

      free(v->cell);
      break;
  }

  free(v);
}

lval_t* lval_add(lval_t* v, lval_t* x) {
  v->count++;
  v->cell = realloc(v->cell, sizeof(lval_t*) * v->count);
  v->cell[v->count-1] = x;

  return v;
}

lval_t* lval_read_num(mpc_ast_t* t) {
  errno = 0;
  double x = strtod(t->contents, NULL);

  return errno != ERANGE ? lval_num(x) : lval_err("invalid number");
}

lval_t* lval_read(mpc_ast_t* t) {
  if (strstr(t->tag, "number"))
    return lval_read_num(t);
  if (strstr(t->tag, "symbol"))
    return lval_sym(t->contents);
  
  lval_t* x = NULL;
  if (strcmp(t->tag, ">") == 0) { x = lval_sexpr(); }
  if (strstr(t->tag, "sexpr"))  { x = lval_sexpr(); }
  if (strstr(t->tag, "qexpr"))  { x = lval_qexpr(); }

  for (int i = 0; i < t->children_num; ++i) {
    if (strcmp(t->children[i]->contents, "(") == 0) { continue; }
    if (strcmp(t->children[i]->contents, ")") == 0) { continue; }
    if (strcmp(t->children[i]->contents, "{") == 0) { continue; }
    if (strcmp(t->children[i]->contents, "}") == 0) { continue; }
    if (strcmp(t->children[i]->tag,  "regex") == 0) { continue; }
    x = lval_add(x, lval_read(t->children[i]));
  }

  return x;
}

void lval_expr_print(lval_t* v, char open, char close) {
  putchar(open);

  for (int i = 0; i < v->count; ++i) {
    lval_print(v->cell[i]);

    // Print trailing space if not last element.
    if (i != v->count-1) {
      putchar(' ');
    }
  }

  putchar(close);
}

void lval_print(lval_t* v) {
  switch (v->type) {
    case LVAL_NUM:
      printf("%lf", v->num);
      break;
    case LVAL_ERR:
      printf("Error: %s", v->err);
      break;
    case LVAL_SYM:
      printf("%s", v->sym);
      break;
    case LVAL_SEXPR:
      lval_expr_print(v, '(', ')');
      break;
    case LVAL_QEXPR:
      lval_expr_print(v, '{', '}');
      break;
  }
}

void lval_println(lval_t* v) {
  lval_print(v);
  putchar('\n');
}

lval_t* lval_eval_sexpr(lval_t* v) {
  for (int i = 0; i < v->count; ++i)
  {
    v->cell[i] = lval_eval(v->cell[i]);
  }

  for (int i = 0; i < v->count; ++i) {
    if (v->cell[i]->type == LVAL_ERR)
      return lval_take(v, i);
  }

  if (v->count == 0)
    return v;

  if (v->count == 1)
    return lval_take(v, 0);


  // Ensure first lval is a symbol
  lval_t* f = lval_pop(v, 0);
  if (f->type != LVAL_SYM) {
    lval_del(f);
    lval_del(v);

    return lval_err("sexp does not begin with symbol");
  }

  lval_t* result = builtin_op(v, f->sym);
  lval_del(f);
  return result;
}

lval_t* lval_eval(lval_t* v) {
  if (v->type == LVAL_SEXPR)
    return lval_eval_sexpr(v);

  return v;
}

lval_t* lval_pop(lval_t* v, int i) {
  lval_t* x = v->cell[i];

  memmove(&v->cell[i], &v->cell[i+1], sizeof(lval_t*) * (v->count-i-1));

  v->count--;

  v->cell = realloc(v->cell, sizeof(lval_t*) * v->count);

  return x;
}

lval_t* lval_take(lval_t* v, int i) {
  lval_t* x = lval_pop(v, i);
  lval_del(v);

  return x;
}
