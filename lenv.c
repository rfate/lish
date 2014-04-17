#include "lenv.h"

lenv_t* lenv_new(void) {
  lenv_t* e = malloc(sizeof(lenv_t));
  e->parent = NULL;
  e->count  = 0;
  e->syms   = NULL;
  e->vals   = NULL;

  return e;
}

void lenv_del(lenv_t* e) {
  for (int i = 0; i < e->count; ++i) {
    free(e->syms[i]);
    lval_del(e->vals[i]);
  }

  free(e->syms);
  free(e->vals);
  free(e);
}

lenv_t* lenv_copy(lenv_t* e) {
  lenv_t* n = malloc(sizeof(lenv_t));
  n->parent = e->parent;
  n->count  = e->count;

  n->syms = malloc(sizeof(char*)   * n->count);
  n->vals = malloc(sizeof(lval_t*) * n->count);

  for (int i = 0; i < e->count; ++i)
  {
    n->syms[i] = malloc(strlen(e->syms[i]) + 1);
    strcpy(n->syms[i], e->syms[i]);
    n->vals[i] = lval_copy(e->vals[i]);
  }

  return n;
}

lval_t* lenv_get(lenv_t* e, lval_t* k) {
  for (int i = 0; i < e->count; ++i)
  {
    if (strcmp(e->syms[i], k->sym) == 0)
      return lval_copy(e->vals[i]);
  }

  if (e->parent) {
    return lenv_get(e->parent, k);
  }

  return lval_err("Unbound symbol");
}

// GLOBAL define
void lenv_def(lenv_t* e, lval_t* k, lval_t* v) {
  while (e->parent)
    e = e->parent;

  lenv_set(e, k, v);
}

void lenv_set(lenv_t* e, lval_t* k, lval_t* v) {
  for (int i = 0; i < e->count; ++i) {
    if (strcmp(e->syms[i], k->sym) == 0) {
      lval_del(e->vals[i]);
      e->vals[i] = lval_copy(v);
      return;
    }
  }

  e->count++;
  e->vals = realloc(e->vals, sizeof(lval_t*) * e->count);
  e->syms = realloc(e->syms, sizeof(char*)   * e->count);

  e->vals[e->count - 1] = lval_copy(v);
  e->syms[e->count - 1] = malloc(strlen(k->sym) + 1);
  strcpy(e->syms[e->count - 1], k->sym);
}

void lenv_add_builtin(lenv_t* e, char* name, lbuiltin func) {
  lval_t* k = lval_sym(name);
  lval_t* v = lval_fun(func);

  lenv_set(e, k, v);

  lval_del(k);
  lval_del(v);
}

void lenv_add_builtins(lenv_t* e) {
  // list
  lenv_add_builtin(e, "list", builtin_list);
  lenv_add_builtin(e, "head", builtin_head);
  lenv_add_builtin(e, "tail", builtin_tail);
  lenv_add_builtin(e, "eval", builtin_eval);
  lenv_add_builtin(e, "join", builtin_join);
  lenv_add_builtin(e, "len",  builtin_len);

  // vars
  lenv_add_builtin(e, "def",    builtin_set);
  lenv_add_builtin(e, "=",      builtin_def);
  lenv_add_builtin(e, "lambda", builtin_lambda);
  lenv_add_builtin(e, "λ",      builtin_lambda);

  // operators
  lenv_add_builtin(e, "+", builtin_add);
  lenv_add_builtin(e, "-", builtin_sub);
  lenv_add_builtin(e, "*", builtin_mul);
  lenv_add_builtin(e, "/", builtin_div);
  lenv_add_builtin(e, "%", builtin_mod);

  // conditionals
  lenv_add_builtin(e, "if", builtin_if);
  lenv_add_builtin(e, ">",  builtin_gt);
  lenv_add_builtin(e, "<",  builtin_lt);
  lenv_add_builtin(e, ">=", builtin_ge);
  lenv_add_builtin(e, "<=", builtin_le);

  // io
  lenv_add_builtin(e, "puts", builtin_puts);
  lenv_add_builtin(e, "load", builtin_load);
}
