#include "lenv.h"

lenv_t* lenv_new(void) {
  lenv_t* e = malloc(sizeof(lenv_t));
  e->count = 0;
  e->syms  = NULL;
  e->vals  = NULL;

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

lval_t* lenv_get(lenv_t* e, lval_t* k) {
  for (int i = 0; i < e->count; ++i)
  {
    if (strcmp(e->syms[i], k->sym) == 0)
      return lval_copy(e->vals[i]);
  }

  return lval_err("Unbound symbol");
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
  lenv_add_builtin(e, "list", builtin_list);
  lenv_add_builtin(e, "head", builtin_head);
  lenv_add_builtin(e, "tail", builtin_tail);
  lenv_add_builtin(e, "eval", builtin_eval);
  lenv_add_builtin(e, "join", builtin_join);
  lenv_add_builtin(e, "len",  builtin_len);

  lenv_add_builtin(e, "def", builtin_def);

  lenv_add_builtin(e, "+", builtin_add);
  lenv_add_builtin(e, "-", builtin_sub);
  lenv_add_builtin(e, "*", builtin_mul);
  lenv_add_builtin(e, "/", builtin_div);
  lenv_add_builtin(e, "%", builtin_mod);
}