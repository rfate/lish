#include <math.h>
#include "lval.h"
#include "lenv.h"
#include "builtin.h"

char* ltype_name(int t) {
  switch(t) {
    case LVAL_ERR:   return "Error";
    case LVAL_INT:   return "Integer";
    case LVAL_FLOAT: return "Float";
    case LVAL_NUM:   return "Number";
    case LVAL_BOOL:  return "Boolean";
    case LVAL_STR:   return "String";
    case LVAL_SYM:   return "Symbol";
    case LVAL_FUN:   return "Function";
    case LVAL_SEXPR: return "S-Expression";
    case LVAL_QEXPR: return "Q-Expression";
    case LVAL_TABLE: return "Table";
    default:         return "Unknown";
  }
}

/// Constructors
lval_t* lval_int(long int x) {
  lval_t* v = malloc(sizeof(lval_t));
  v->type = LVAL_INT;
  v->num  = x;

  return v;
}

lval_t* lval_float(double x) {
  lval_t* v = malloc(sizeof(lval_t));
  v->type = LVAL_FLOAT;
  v->num  = x;

  return v;
}

lval_t* lval_bool(int x) {
  lval_t* v = malloc(sizeof(lval_t));
  v->type = LVAL_BOOL;
  v->num  = (x != 0);

  return v;
}

lval_t* lval_str(char* str) {
  lval_t* v = malloc(sizeof(lval_t));
  v->type = LVAL_STR;
  v->str  = malloc(strlen(str) + 1);
  strcpy(v->str, str);
  
  return v;
}

lval_t* lval_err(char* fmt, ...) {
  lval_t* v = malloc(sizeof(lval_t));
  v->type = LVAL_ERR;

  va_list va;
  va_start(va, fmt);

  v->err = malloc(512);
  vsnprintf(v->err, 511, fmt, va);

  v->err = realloc(v->err, strlen(v->err) + 1);

  va_end(va);

  return v;
}

lval_t* lval_sym(char* sym) {
  lval_t* v = malloc(sizeof(lval_t));
  v->type = LVAL_SYM;
  v->sym  = malloc(strlen(sym) + 1);
  strcpy(v->sym, sym);

  return v;
}

lval_t* lval_fun(lbuiltin func) {
  lval_t* v = malloc(sizeof(lval_t));
  v->type    = LVAL_FUN;
  v->builtin = func;

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

lval_t* lval_lambda(lval_t* formals, lval_t* body) {
  lval_t* v = malloc(sizeof(lval_t));
  v->type    = LVAL_FUN;
  v->builtin = NULL;
  v->env     = lenv_new();
  v->formals = formals;
  v->body    = body;

  return v;
}

lval_t* lval_table(void) {
  lval_t* v = malloc(sizeof(lval_t));
  v->type = LVAL_TABLE;
  v->env = lenv_new();

  return v;
}
///

void lval_del(lval_t* v) {
  switch (v->type) {
    case LVAL_INT:
    case LVAL_FLOAT:
    case LVAL_BOOL:
      break;

    case LVAL_TABLE:
      lenv_del(v->env);
      break;

    case LVAL_FUN:
      if (!v->builtin) {
        lenv_del(v->env);
        lval_del(v->formals);
        lval_del(v->body);
      }
      break;

    case LVAL_STR: free(v->str); break;
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

lval_t* lval_truthy(lval_t* v) {
  int b;

  switch (v->type) {
    case LVAL_INT:
    case LVAL_FLOAT:
    case LVAL_STR:
    case LVAL_FUN:
      b = 1;
      break;
    
    case LVAL_SEXPR:
    case LVAL_QEXPR:
      b = (v->count > 0);
      break;

    case LVAL_BOOL:
      b = v->num;
      break;

    default:
      return lval_err("Cannot discern truthiness of type %s", ltype_name(v->type));
      break;
  }

  return lval_bool(b);
}

int lval_eq(lval_t* x, lval_t* y) {
  if ((x->type == LVAL_INT || x->type == LVAL_FLOAT)
    && (y->type == LVAL_INT || y->type == LVAL_FLOAT)) {
    return (x->num == y->num);
  } else if (x->type != y->type) {
    return 0;
  }

  switch (x->type) {
    case LVAL_BOOL:
      return (x->num == y->num);

    case LVAL_STR:
      return (strcmp(x->str, y->str) == 0);

    case LVAL_ERR:
      return (strcmp(x->err, y->err) == 0);

    case LVAL_SYM:
      return (strcmp(x->sym, y->sym) == 0);

    case LVAL_FUN:
      if (x->builtin || y->builtin) {
        return (x->builtin == y->builtin);
      } else {
        return lval_eq(x->formals, y->formals) && lval_eq(x->body, y->body);
      }

    case LVAL_SEXPR:
    case LVAL_QEXPR:
      if (x->count != y->count)
        return 0;
      
      for (int i = 0; i < x->count; ++i) {
        if (!lval_eq(x->cell[i], y->cell[i]))
          return 0;
      }
      return 1;
      break;
  }

  printf("Warning! Comparison reached default case!\n");
  return 0;
}

lval_t* lval_add(lval_t* v, lval_t* x) {
  v->count++;
  v->cell = realloc(v->cell, sizeof(lval_t*) * v->count);
  v->cell[v->count-1] = x;

  return v;
}

lval_t* lval_call(lenv_t* e, lval_t* f, lval_t* a) {
  if (f->builtin) return f->builtin(e, a);

  int given = a->count;
  int total = f->formals->count;

  while (a->count) {
    if (f->formals->count == 0) {
      lval_del(a);
      lval_err("Function passed too many arguments. Expected %d, got %d.",
        total, given);
    }

    lval_t* sym = lval_pop(f->formals, 0);
    lval_t* val = lval_pop(a, 0);

    lenv_set(f->env, sym, val);

    lval_del(sym);
    lval_del(val);
  }

  lval_del(a);

  if (f->formals->count == 0) {
    f->env->parent = e;

    return builtin_eval(f->env, lval_add(lval_sexpr(), lval_copy(f->body)));
  }
    
  return lval_copy(f);
}

lval_t* lval_read_int(mpc_ast_t* t) {
  errno = 0;
  long x = strtol(t->contents, NULL, 10);

  return errno != ERANGE ? lval_int(x) : lval_err("invalid integer");
}

lval_t* lval_read_float(mpc_ast_t* t) {
  errno = 0;
  double x = strtod(t->contents, NULL);

  return errno != ERANGE ? lval_float(x) : lval_err("invalid float");
}

lval_t* lval_read_bool(mpc_ast_t* t) {
  int x = (strcmp(t->contents, "true") == 0);

  return lval_bool(x);
}

lval_t* lval_read_str(mpc_ast_t* t) {
  // trim ending quote
  t->contents[strlen(t->contents) - 1] = '\0';

  char* unescaped = malloc(strlen(t->contents + 1) + 1);
  strcpy(unescaped, t->contents + 1);

  unescaped = mpcf_escape(unescaped);

  lval_t* str = lval_str(unescaped);
  free(unescaped);

  return str;
}

lval_t* lval_read_table_pair(mpc_ast_t* t) {
  lval_t* x = lval_qexpr();

  for (int i = 0; i < t->children_num; ++i) {
    if (strcmp(t->children[i]->contents, "=") == 0)  { continue; }

    x = lval_add(x, lval_read(t->children[i]));
  }

  return x;
}

lval_t* lval_read_table(mpc_ast_t* t) {
  lval_t* x = lval_table();

	mpc_ast_print(t);

  for (int i = 0; i < t->children_num; ++i) {
    if (strstr(t->children[i]->tag, "tablepair")) {
      lval_t* p = lval_read_table_pair(t->children[i]);
      lenv_set(x->env, p->cell[0], p->cell[1]);
      lval_del(p);
    }
  }

  return x;
}

lval_t* lval_read(mpc_ast_t* t) {
  if (strstr(t->tag, "integer"))
    return lval_read_int(t);

  if (strstr(t->tag, "float"))
    return lval_read_float(t);

  if (strstr(t->tag, "boolean"))
    return lval_read_bool(t);

  if (strstr(t->tag, "symbol"))
    return lval_sym(t->contents);

  if (strstr(t->tag, "string"))
    return lval_read_str(t);

  if (strstr(t->tag, "table")) 
    return lval_read_table(t);

  lval_t* x = NULL;
  if (strcmp(t->tag, ">") == 0) { x = lval_sexpr(); }
  if (strstr(t->tag, "sexpr"))  { x = lval_sexpr(); }
  if (strstr(t->tag, "qexpr"))  { x = lval_qexpr(); }

  for (int i = 0; i < t->children_num; ++i) {
    if (strcmp(t->children[i]->contents, "(") == 0)  { continue; }
    if (strcmp(t->children[i]->contents, ")") == 0)  { continue; }
    if (strcmp(t->children[i]->contents, "{") == 0)  { continue; }
    if (strcmp(t->children[i]->contents, "}") == 0)  { continue; }
    if (strcmp(t->children[i]->tag,  "regex") == 0)  { continue; }
    if (strstr(t->children[i]->tag, "comment"))      { continue; }
    x = lval_add(x, lval_read(t->children[i]));
  }

  return x;
}

void lval_expr_print_r(lval_t* v) {
  for (int i = 0; i < v->count; ++i) {
    lval_print(v->cell[i]);

    // Print trailing space if not last element.
    if (i != v->count-1) {
      putchar(' ');
    }
  }
}

void lval_expr_print(lval_t* v, char open, char close) {
  putchar(open);

  lval_expr_print_r(v);

  putchar(close);
}

void lval_str_print(lval_t* v) {
  char* escaped = malloc(strlen(v->str) + 1);
  strcpy(escaped, v->str);

  escaped = mpcf_escape(escaped);

  printf("\"%s\"", escaped);

  free(escaped);
}

void lval_float_print(lval_t* v) {
  // ends in zero, print with a trailing zero
  if (fmod(v->num, 1) == 0) {
    printf("%.01lf", v->num);
  } else {
    printf("%g", v->num);
  }
}

void lval_table_print(lval_t* v) {
  printf("[");

  for (int i = 0; i < v->env->count; ++i) {
    if (i > 0) { printf(","); }

    printf(" %s = ", v->env->syms[i]);
    lval_print(v->env->vals[i]);
  }

  printf(" ]");
}

void lval_print_r(lval_t* v, int root) {
  switch (v->type) {
    case LVAL_INT:
      printf("%ld", (long int) v->num);
      break;
    case LVAL_FLOAT:
      lval_float_print(v);
      break;
    case LVAL_BOOL:
      printf("%s", (v->num == 0) ? "false" : "true");
      break;
    case LVAL_STR:
      lval_str_print(v);
      break;
    case LVAL_ERR:
      printf("Error: %s", v->err);
      break;
    case LVAL_SYM:
      printf("%s", v->sym);
      break;
    case LVAL_FUN:
      if (v->builtin) {
        printf("<builtin>");
      } else {
        printf("(lambda ");
        lval_print(v->formals);
        putchar(' ');
        lval_print(v->body);
        putchar(')');
      }
      break;
    case LVAL_SEXPR:
      if (root)
        lval_expr_print_r(v);
      else
        lval_expr_print(v, '(', ')');
      break;
    case LVAL_QEXPR:
      lval_expr_print(v, '{', '}');
      break;
    case LVAL_TABLE:
      lval_table_print(v);
      break;
    default:
      printf("<unprintable type>");
      break;
  }
}

void lval_print(lval_t* v) {
  lval_print_r(v, 0);
}

void lval_println(lval_t* v) {
  lval_print_r(v, 1);
  putchar('\n');
}

lval_t* lval_eval_sexpr(lenv_t* e, lval_t* v) {
  for (int i = 0; i < v->count; ++i)
  {
    v->cell[i] = lval_eval(e, v->cell[i]);
  }

  for (int i = 0; i < v->count; ++i) {
    if (v->cell[i]->type == LVAL_ERR)
      return lval_take(v, i);
  }

  if (v->count == 0)
    return v;

  if (v->count == 1)
    return lval_take(v, 0);

  // Ensure first lval is a function
  lval_t* f = lval_pop(v, 0);
  if (f->type != LVAL_FUN) {
    lval_t* err = lval_err("S-Expression starts with incorrect type. Expected %s, got %s.",
      ltype_name(f->type), ltype_name(LVAL_FUN));

    lval_del(f);
    lval_del(v);

    return err;
  }

  lval_t* result = lval_call(e, f, v);
  return result;
}

lval_t* lval_eval(lenv_t* e, lval_t* v) {
  if (v->type == LVAL_SYM) {
    lval_t* x = lenv_get(e, v);
    lval_del(v);

    return x;
  }

  if (v->type == LVAL_SEXPR)
    return lval_eval_sexpr(e, v);

  return v;
}

lval_t* lval_copy(lval_t* v) {
  lval_t* x = malloc(sizeof(lval_t));
  x->type = v->type;

  switch (v->type) {
    case LVAL_INT:
    case LVAL_FLOAT:
    case LVAL_BOOL:
      x->num = v->num;
      break;
    case LVAL_FUN:
      if (v->builtin) {
        x->builtin = v->builtin;
      } else {
        x->builtin = NULL;
        x->env     = lenv_copy(v->env);
        x->formals = lval_copy(v->formals);
        x->body    = lval_copy(v->body);
      }
      break;

    case LVAL_TABLE:
      x->env = lenv_copy(v->env);
      break;

    case LVAL_STR:
      x->str = malloc(strlen(v->str) + 1);
      strcpy(x->str, v->str);
      break;

    case LVAL_ERR:
      x->err = malloc(strlen(v->err) + 1);
      strcpy(x->err, v->err);
      break;

    case LVAL_SYM:
      x->sym = malloc(strlen(v->sym) + 1);
      strcpy(x->sym, v->sym);
      break;

    case LVAL_SEXPR:
    case LVAL_QEXPR:
      x->count = v->count;
      x->cell = malloc(sizeof(lval_t*) * v->count);
      for (int i = 0; i < v->count; ++i) {
        x->cell[i] = lval_copy(v->cell[i]);
      }

      break;
  }

  return x;
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

lval_t* lval_join(lval_t* x, lval_t* y) {
  while (y->count)
    x = lval_add(x, lval_pop(y, 0));

  lval_del(y);

  return x;
}
