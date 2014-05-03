#include <math.h>
#include "lval.h"
#include "lenv.h"
#include "builtin.h"

char* ltype_name(int t) {
  switch(t) {
    case LVAL_ERR:   return "Error";
    case LVAL_INT:   return "Integer";
    case LVAL_FLOAT: return "Float";
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
  v->type     = LVAL_INT;
  v->data.num = x;

  return v;
}

lval_t* lval_float(double x) {
  lval_t* v = malloc(sizeof(lval_t));
  v->type     = LVAL_FLOAT;
  v->data.num = x;

  return v;
}

lval_t* lval_bool(int x) {
  lval_t* v = malloc(sizeof(lval_t));
  v->type     = LVAL_BOOL;
  v->data.num = (x != 0);

  return v;
}


lval_t* lval_err(char* fmt, ...) {
  lval_t* v = malloc(sizeof(lval_t));
  v->type = LVAL_ERR;

  va_list va;
  va_start(va, fmt);

  v->data.err = malloc(512);
  vsnprintf(v->data.err, 511, fmt, va);

  v->data.err = realloc(v->data.err, strlen(v->data.err) + 1);

  va_end(va);

  return v;
}

lval_t* lval_sym(char* sym, int lit) {
  lval_t* v = malloc(sizeof(lval_t));
  v->type          = LVAL_SYM;
  v->data.sym.lit  = lit;
  v->data.sym.name = malloc(strlen(sym) + 1);
  strcpy(v->data.sym.name, sym);

  return v;
}

lval_t* lval_fun(lbuiltin func) {
  lval_t* v = malloc(sizeof(lval_t));
  v->type              = LVAL_FUN;
  v->data.func.builtin = func;

  return v;
}

lval_t* lval_sexpr(void) {
  lval_t* v = malloc(sizeof(lval_t));
  v->type            = LVAL_SEXPR;
  v->data.expr.count = 0;
  v->data.expr.cell  = NULL;

  return v;
}

lval_t* lval_lambda(lval_t* formals, lval_t* body) {
  lval_t* v = malloc(sizeof(lval_t));
  v->type              = LVAL_FUN;
  v->data.func.env     = lenv_new();
  v->data.func.builtin = NULL;
  v->data.func.formals = formals;
  v->data.func.body    = body;

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
      lval_table_del(v);
      return;

    case LVAL_FUN:
      if (!v->data.func.builtin) {
        lenv_del(v->data.func.env);
        lval_del(v->data.func.formals);
        lval_del(v->data.func.body);
      }
      break;

    case LVAL_STR: free(v->data.str); break;
    case LVAL_ERR: free(v->data.err); break;
    case LVAL_SYM: free(v->data.sym.name); break;

    case LVAL_SEXPR:
    case LVAL_QEXPR:
      for (int i = 0; i < v->data.expr.count; ++i) {
        lval_del(v->data.expr.cell[i]);
      }

      free(v->data.expr.cell);
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
      b = (v->data.expr.count > 0);
      break;

    case LVAL_BOOL:
      b = v->data.num;
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
    return (x->data.num == y->data.num);
  } else if (x->type != y->type) {
    return 0;
  }

  switch (x->type) {
    case LVAL_BOOL:
      return (x->data.num == y->data.num);

    case LVAL_STR:
      return (strcmp(x->data.str, y->data.str) == 0);

    case LVAL_ERR:
      return (strcmp(x->data.err, y->data.err) == 0);

    case LVAL_SYM:
      return (strcmp(x->data.sym.name, y->data.sym.name) == 0
            && x->data.sym.lit == y->data.sym.lit);

    case LVAL_FUN:
      if (x->data.func.builtin || y->data.func.builtin) {
        return (x->data.func.builtin == y->data.func.builtin);
      } else {
        return lval_eq(x->data.func.formals, y->data.func.formals)
            && lval_eq(x->data.func.body,    y->data.func.body);
      }

    case LVAL_SEXPR:
    case LVAL_QEXPR:
      if (x->data.expr.count != y->data.expr.count)
        return 0;
      
      for (int i = 0; i < x->data.expr.count; ++i) {
        if (!lval_eq(x->data.expr.cell[i], y->data.expr.cell[i]))
          return 0;
      }
      return 1;
      break;
 
    case LVAL_TABLE:
      if (x->data.table.count != y->data.table.count)
        return 0;

      for (int i = 0; i < x->data.table.count; ++i) {
        if (!lval_eq(x->data.table.keys[i], y->data.table.keys[i]))
          return 0;

        if (!lval_eq(x->data.table.vals[i], y->data.table.vals[i]))
          return 0;
      }

      return 1;

      break;
  }

  printf("Warning! Comparison reached default case!\n");
  return 0;
}

lval_t* lval_add(lval_t* v, lval_t* x) {
  v->data.expr.count++;
  v->data.expr.cell = realloc(v->data.expr.cell, sizeof(lval_t*) * v->data.expr.count);
  v->data.expr.cell[v->data.expr.count-1] = x;

  return v;
}

lval_t* lval_call(lenv_t* e, lval_t* f, lval_t* a) {
  if (f->data.func.builtin)
    return f->data.func.builtin(e, a);

  int given = a->data.expr.count;
  int total = f->data.func.formals->data.expr.count;

  while (a->data.expr.count) {
    if (f->data.func.formals->data.expr.count == 0) {
      lval_del(a);
      lval_err("Function passed too many arguments. Expected %d, got %d.",
        total, given);
    }

    lval_t* sym = lval_pop(f->data.func.formals, 0);
    lval_t* val = lval_pop(a, 0);

    lenv_set(f->data.func.env, sym, val);

    lval_del(sym);
    lval_del(val);
  }

  lval_del(a);

  if (f->data.func.formals->data.expr.count == 0) {
    f->data.func.env->parent = e;

    return builtin_eval(f->data.func.env, lval_add(lval_sexpr(), lval_copy(f->data.func.body)));
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

  for (int i = 0; i < t->children_num; ++i) {
    if (strstr(t->children[i]->tag, "tablepair")) {
      lval_t* p = lval_read_table_pair(t->children[i]);
      lval_table_set(x, p->data.expr.cell[0], p->data.expr.cell[1]);
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

  if (strstr(t->tag, "litsymbol"))
    return lval_sym(t->children[1]->contents, 1);

  if (strstr(t->tag, "symbol") || strstr(t->tag, "operator"))
    return lval_sym(t->contents, 0);

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
  for (int i = 0; i < v->data.expr.count; ++i) {
    lval_print(v->data.expr.cell[i]);

    // Print trailing space if not last element.
    if (i != v->data.expr.count-1) {
      putchar(' ');
    }
  }
}

void lval_expr_print(lval_t* v, char open, char close) {
  putchar(open);

  lval_expr_print_r(v);

  putchar(close);
}

void lval_float_print(lval_t* v) {
  // ends in zero, print with a trailing zero
  if (fmod(v->data.num, 1) == 0) {
    printf("%.01lf", v->data.num);
  } else {
    printf("%g", v->data.num);
  }
}

void lval_print_r(lval_t* v, int root) {
  switch (v->type) {
    case LVAL_INT:
      printf("%ld", (long int) v->data.num);
      break;
    case LVAL_FLOAT:
      lval_float_print(v);
      break;
    case LVAL_BOOL:
      printf("%s", (v->data.num == 0) ? "false" : "true");
      break;
    case LVAL_STR:
      lval_str_print(v);
      break;
    case LVAL_ERR:
      printf("Error: %s", v->data.err);
      break;
    case LVAL_SYM:
      if (v->data.sym.lit) {
        printf("@%s", v->data.sym.name);
      } else {
        printf("%s", v->data.sym.name);
      }
      break;
    case LVAL_FUN:
      if (v->data.func.builtin) {
        printf("<builtin>");
      } else {
        printf("(lambda ");
        lval_print(v->data.func.formals);
        putchar(' ');
        lval_print(v->data.func.body);
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
      printf("<unprintable type: %s>", ltype_name(v->type));
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
  for (int i = 0; i < v->data.expr.count; ++i) {
    v->data.expr.cell[i] = lval_eval(e, v->data.expr.cell[i]);
  }

  for (int i = 0; i < v->data.expr.count; ++i) {
    if (v->data.expr.cell[i]->type == LVAL_ERR)
      return lval_take(v, i);
  }

  if (v->data.expr.count == 0)
    return v;

  if (v->data.expr.count == 1)
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
  if (v->type == LVAL_TABLE)
    return lval_table_copy(v);

  lval_t* x = malloc(sizeof(lval_t));
  x->type = v->type;

  switch (v->type) {
    case LVAL_INT:
    case LVAL_FLOAT:
    case LVAL_BOOL:
      x->data.num = v->data.num;
      break;
    case LVAL_FUN:
      if (v->data.func.builtin) {
        x->data.func.builtin = v->data.func.builtin;
      } else {
        x->data.func.env     = lenv_copy(v->data.func.env);
        x->data.func.builtin = NULL;
        x->data.func.formals = lval_copy(v->data.func.formals);
        x->data.func.body    = lval_copy(v->data.func.body);
      }
      break;

    case LVAL_STR:
      x->data.str = malloc(strlen(v->data.str) + 1);
      strcpy(x->data.str, v->data.str);
      break;

    case LVAL_ERR:
      x->data.err = malloc(strlen(v->data.err) + 1);
      strcpy(x->data.err, v->data.err);
      break;

    case LVAL_SYM:
      x->data.sym.lit = v->data.sym.lit;
      x->data.sym.name = malloc(strlen(v->data.sym.name) + 1);
      strcpy(x->data.sym.name, v->data.sym.name);
      break;

    case LVAL_SEXPR:
    case LVAL_QEXPR:
      x->data.expr.count = v->data.expr.count;
      x->data.expr.cell = malloc(sizeof(lval_t*) * v->data.expr.count);
      for (int i = 0; i < v->data.expr.count; ++i) {
        x->data.expr.cell[i] = lval_copy(v->data.expr.cell[i]);
      }

      break;
  }

  return x;
}

lval_t* lval_pop(lval_t* v, int i) {
  lval_t* x = v->data.expr.cell[i];

  memmove(&v->data.expr.cell[i], &v->data.expr.cell[i+1],
      sizeof(lval_t*) * (v->data.expr.count-i-1));

  v->data.expr.count--;

  v->data.expr.cell = realloc(v->data.expr.cell,
      sizeof(lval_t*) * v->data.expr.count);

  return x;
}

lval_t* lval_take(lval_t* v, int i) {
  lval_t* x = lval_pop(v, i);
  lval_del(v);

  return x;
}

lval_t* lval_join(lval_t* x, lval_t* y) {
  while (y->data.expr.count)
    x = lval_add(x, lval_pop(y, 0));

  lval_del(y);

  return x;
}
