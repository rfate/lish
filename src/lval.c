#include <math.h>
#include "lval.h"
#include "lenv.h"
#include "builtin.h"

#include "types/string.h"
#include "types/qexpr.h"
#include "types/number.h"
#include "types/table.h"

char*
ltype_name(int t)
{
  switch(t) {
    case LVAL_NIL:    return "Nil";
    case LVAL_ERR:    return "Error";
    case LVAL_INT:    return "Integer";
    case LVAL_BIGINT: return "BigInt";
    case LVAL_FLOAT:  return "Float";
    case LVAL_BOOL:   return "Boolean";
    case LVAL_STR:    return "String";
    case LVAL_SYM:    return "Symbol";
    case LVAL_FUN:    return "Function";
    case LVAL_SEXPR:  return "S-Expression";
    case LVAL_QEXPR:  return "Q-Expression";
    case LVAL_TABLE:  return "Table";
    default:          return "Unknown";
  }
}

/// Constructors
lval_t*
lval_nil(void)
{
  lval_t *v = malloc(sizeof(lval_t));
  v->type = LVAL_NIL;

  return v;
}

lval_t*
lval_bool(int x)
{
  lval_t *v = malloc(sizeof(lval_t));
  v->type     = LVAL_BOOL;
  v->num = (x != FALSE);

  return v;
}

lval_t*
lval_err(char *fmt, ...)
{
  lval_t *v = malloc(sizeof(lval_t));
  v->type = LVAL_ERR;

  va_list va;
  va_start(va, fmt);

  v->err = malloc(512);
  vsnprintf(v->err, 511, fmt, va);

  v->err = realloc(v->err, strlen(v->err) + 1);

  va_end(va);

  return v;
}

lval_t*
lval_sym(char *sym, int lit)
{
  lval_t *v = malloc(sizeof(lval_t));
  v->type          = LVAL_SYM;
  v->sym.lit  = lit;
  v->sym.name = malloc(strlen(sym) + 1);
  strcpy(v->sym.name, sym);

  return v;
}

lval_t*
lval_fun(lbuiltin func)
{
  lval_t *v = malloc(sizeof(lval_t));
  v->type              = LVAL_FUN;
  v->func.builtin = func;

  return v;
}

lval_t*
lval_sexpr(void)
{
  lval_t *v = malloc(sizeof(lval_t));
  v->type            = LVAL_SEXPR;
  v->expr.count = 0;
  v->expr.cell  = NULL;

  return v;
}

lval_t*
lval_lambda(lval_t *formals, lval_t *body)
{
  lval_t *v = malloc(sizeof(lval_t));
  v->type              = LVAL_FUN;
  v->func.env     = lenv_new();
  v->func.builtin = NULL;
  v->func.formals = formals;
  v->func.body    = body;

  return v;
}
///

void
lval_del(lval_t *v)
{
  switch (v->type) {
    case LVAL_BIGINT:
      mpz_clear(v->bignum);
      return;

    case LVAL_TABLE:
      lval_table_del(v);
      return;

    case LVAL_FUN:
      if (!v->func.builtin) {
        lenv_del(v->func.env);
        lval_del(v->func.formals);
        lval_del(v->func.body);
      }
      break;

    case LVAL_STR: free(v->str); break;
    case LVAL_ERR: free(v->err); break;
    case LVAL_SYM: free(v->sym.name); break;

    case LVAL_SEXPR:
    case LVAL_QEXPR:
      for (int i = 0; i < v->expr.count; ++i) {
        lval_del(v->expr.cell[i]);
      }

      free(v->expr.cell);
      break;
  }

  free(v);
}

lval_t*
lval_truthy(lval_t *v)
{
  uint8_t b = FALSE;

  switch (v->type) {
    case LVAL_NIL:
      b = FALSE;
      break;

    case LVAL_INT:
    case LVAL_FLOAT:
    case LVAL_BIGINT:
    case LVAL_STR:
    case LVAL_FUN:
    case LVAL_SYM:
      b = TRUE;
      break;

    case LVAL_SEXPR:
    case LVAL_QEXPR:
      b = (v->expr.count > 0);
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

int
lval_eq(lval_t *x, lval_t *y)
{
  if ((x->type == LVAL_INT || x->type == LVAL_FLOAT)
   && (y->type == LVAL_INT || y->type == LVAL_FLOAT)) {
    return (x->num == y->num);
  } else if (x->type != y->type) {
    return FALSE;
  }

  switch (x->type) {
    case LVAL_NIL:
      return TRUE;

    case LVAL_BOOL:
      return (x->num == y->num);

    case LVAL_STR:
      return (strcmp(x->str, y->str) == 0);

    case LVAL_ERR:
      return (strcmp(x->err, y->err) == 0);

    case LVAL_SYM:
      return (strcmp(x->sym.name, y->sym.name) == 0
            && x->sym.lit == y->sym.lit);

    case LVAL_FUN:
      if (x->func.builtin || y->func.builtin) {
        return (x->func.builtin == y->func.builtin);
      } else {
        return lval_eq(x->func.formals, y->func.formals)
            && lval_eq(x->func.body,    y->func.body);
      }

    case LVAL_SEXPR:
    case LVAL_QEXPR:
      if (x->expr.count != y->expr.count) {
        return FALSE;
			}

      for (int i = 0; i < x->expr.count; ++i) {
        if (!lval_eq(x->expr.cell[i], y->expr.cell[i]))
          return FALSE;
      }

      return TRUE;

    case LVAL_TABLE:
      if (x->table.count != y->table.count)
        return FALSE;

      for (int i = 0; i < x->table.count; ++i) {
        if (!lval_eq(x->table.keys[i], y->table.keys[i]))
          return FALSE;

        if (!lval_eq(x->table.vals[i], y->table.vals[i]))
          return FALSE;
      }

      return TRUE;
  }

  printf("Warning! Failed to compare types %s.\n", ltype_name(x->type));
  return FALSE;
}

lval_t*
lval_add(lval_t *v, lval_t *x)
{
  v->expr.count++;
  v->expr.cell = realloc(v->expr.cell, sizeof(lval_t*) * v->expr.count);
  v->expr.cell[v->expr.count-1] = x;

  return v;
}

lval_t*
lval_call(lenv_t *e, lval_t *f, lval_t *a)
{
  if (f->func.builtin)
    return f->func.builtin(e, a);

  int given = a->expr.count;
  int total = f->func.formals->expr.count;

  while (a->expr.count) {
    if (f->func.formals->expr.count == 0) {
      lval_del(a);
      lval_err("Function passed too many arguments. Expected %d, got %d.",
        total, given);
    }

    lval_t *sym = lval_pop(f->func.formals, 0);
    lval_t *val = lval_pop(a, 0);

    lenv_set(f->func.env, sym, val);

    lval_del(sym);
    lval_del(val);
  }

  lval_del(a);

  if (f->func.formals->expr.count == 0) {
    f->func.env->parent = e;

    return builtin_eval(f->func.env, lval_add(lval_sexpr(), lval_copy(f->func.body)));
  }

  return lval_copy(f);
}

lval_t*
lval_read_int(mpc_ast_t *t)
{
  errno = 0;
  long n = strtol(t->contents, NULL, 10);

  if (errno == ERANGE) {
    return lval_bigint(t->contents);
  } else {
    return lval_int(n);
  }
}

lval_t*
lval_read_float(mpc_ast_t *t)
{
  errno = 0;
  double x = strtod(t->contents, NULL);

  return errno != ERANGE ? lval_float(x) : lval_err("invalid float");
}

lval_t*
lval_read_bool(mpc_ast_t *t)
{
  uint8_t x = (strcmp(t->contents, "true") == 0);

  return lval_bool(x);
}

lval_t*
lval_read_str(mpc_ast_t *t)
{
  // trim ending quote
  t->contents[strlen(t->contents) - 1] = '\0';

  char *unescaped = malloc(strlen(t->contents + 1) + 1);
  strcpy(unescaped, t->contents + 1);

  unescaped = mpcf_unescape(unescaped);

  lval_t *str = lval_str(unescaped);
  free(unescaped);

  return str;
}

lval_t*
lval_read_table_pair(mpc_ast_t *t)
{
  lval_t *x = lval_qexpr();

  for (int i = 0; i < t->children_num; ++i) {
    if (strcmp(t->children[i]->contents, "=") == 0) {
			continue;
		}

    lval_t *v = lval_read(t->children[i]);

    x = lval_add(x, v);
  }

  return x;
}

lval_t*
lval_read_table(mpc_ast_t *t)
{
  lval_t *x = lval_table();

  for (int i = 0; i < t->children_num; ++i) {
    if (strstr(t->children[i]->tag, "tablepair")) {
      lval_t *p = lval_read_table_pair(t->children[i]);
      lval_table_set(x, p->expr.cell[0], p->expr.cell[1]);
      lval_del(p);
    }
  }

  return x;
}

lval_t*
lval_read(mpc_ast_t *t)
{
  if (strstr(t->tag, "nil")) {
    return lval_nil();
	}

  if (strstr(t->tag, "integer")) {
    return lval_read_int(t);
	}

  if (strstr(t->tag, "float")) {
    return lval_read_float(t);
	}

  if (strstr(t->tag, "boolean")) {
    return lval_read_bool(t);
	}

  if (strstr(t->tag, "litsymbol")) {
    return lval_sym(t->children[1]->contents, TRUE);
	}

  if (strstr(t->tag, "symbol") || strstr(t->tag, "operator")) {
    return lval_sym(t->contents, FALSE);
	}

  if (strstr(t->tag, "string")) {
    return lval_read_str(t);
	}

  if (strstr(t->tag, "table")) {
    return lval_read_table(t);
	}

  lval_t *x = NULL;
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

void
lval_expr_print_r(lval_t *v)
{
  for (int i = 0; i < v->expr.count; ++i) {
    lval_print(v->expr.cell[i]);

    // Print trailing space if not last element.
    if (i != v->expr.count-1) {
      putchar(' ');
    }
  }
}

void
lval_expr_print(lval_t *v, char open, char close)
{
  putchar(open);

  lval_expr_print_r(v);

  putchar(close);
}

void
lval_float_print(lval_t *v)
{
  // ends in zero, print with a trailing zero
  if (fmod(v->num, 1) == 0) {
    printf("%.01lf", v->num);
  } else {
    printf("%g", v->num);
  }
}

void
lval_print_r(lval_t *v, int root)
{
  switch (v->type) {
    case LVAL_NIL:
      printf("nil");
      break;
    case LVAL_INT:
      printf("%ld", (long int) v->num);
      break;
    case LVAL_FLOAT:
      lval_float_print(v);
      break;
    case LVAL_BIGINT:
      gmp_printf("%Zd", v->bignum);
      break;
    case LVAL_BOOL:
      printf("%s", (v->num == FALSE) ? "false" : "true");
      break;
    case LVAL_STR:
      lval_str_print(v);
      break;
    case LVAL_ERR:
      printf("Error: %s", v->err);
      break;
    case LVAL_SYM:
      if (v->sym.lit) {
        printf("@%s", v->sym.name);
      } else {
        printf("%s", v->sym.name);
      }
      break;
    case LVAL_FUN:
      if (v->func.builtin) {
        printf("<builtin>");
      } else {
        printf("(lambda ");
        lval_print(v->func.formals);
        putchar(' ');
        lval_print(v->func.body);
        putchar(')');
      }
      break;
    case LVAL_SEXPR:
      if (root) {
        lval_expr_print_r(v);
			} else {
        lval_expr_print(v, '(', ')');
			}
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

void
lval_print(lval_t *v)
{
  lval_print_r(v, FALSE);
}

void
lval_println(lval_t *v)
{
  lval_print_r(v, TRUE);
  putchar('\n');
}

lval_t*
lval_eval_sexpr(lenv_t *e, lval_t *v)
{
  for (int i = 0; i < v->expr.count; ++i) {
    v->expr.cell[i] = lval_eval(e, v->expr.cell[i]);
  }

 // for (int i = 0; i < v->expr.count; ++i) {
 //   if (v->expr.cell[i]->type == LVAL_ERR)
 //     return lval_take(v, i);
 // }

  if (v->expr.count == 0) {
    return v;
	}

	// If single element, reduce.
  if (v->expr.count == 1) {
    return lval_take(v, 0);
	}

  lval_t *f = lval_pop(v, 0);

  // Ensure first lval is a function
  if (f->type == LVAL_FUN) {
    return lval_call(e, f, v);
  }

  if (f->type == LVAL_SEXPR) {
    return lval_eval_sexpr(e, f);
  }

  if (f->type == LVAL_ERR) {
    return f;
  }

  lval_t *err = lval_err("S-Expression starts with incorrect type.%s.",
      ltype_name(f->type));

  lval_del(f);
  lval_del(v);

  return err;
}

lval_t*
lval_eval(lenv_t *e, lval_t *v)
{
  if (v->type == LVAL_SYM) {
    lval_t *x = lenv_get(e, v);
    lval_del(v);

    return x;
  }

  if (v->type == LVAL_SEXPR) {
    return lval_eval_sexpr(e, v);
	}

  return v;
}

lval_t*
lval_copy(lval_t *v)
{
  if (v->type == LVAL_TABLE) {
    return lval_table_copy(v);
	}

  lval_t *x = malloc(sizeof(lval_t));
  x->type = v->type;

  switch (v->type) {
    case LVAL_INT:
    case LVAL_FLOAT:
    case LVAL_BOOL:
      x->num = v->num;
      break;
    case LVAL_BIGINT:
      mpz_init_set(x->bignum, v->bignum);
      break;
    case LVAL_FUN:
      if (v->func.builtin) {
        x->func.builtin = v->func.builtin;
      } else {
        x->func.env     = lenv_copy(v->func.env);
        x->func.builtin = NULL;
        x->func.formals = lval_copy(v->func.formals);
        x->func.body    = lval_copy(v->func.body);
      }
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
      x->sym.lit  = v->sym.lit;
      x->sym.name = malloc(strlen(v->sym.name) + 1);
      strcpy(x->sym.name, v->sym.name);
      break;

    case LVAL_SEXPR:
    case LVAL_QEXPR:
      x->expr.count = v->expr.count;
      x->expr.cell  = malloc(sizeof(lval_t*) * v->expr.count);

      for (int i = 0; i < v->expr.count; ++i) {
        x->expr.cell[i] = lval_copy(v->expr.cell[i]);
      }
      break;
  }

  return x;
}

lval_t*
lval_pop(lval_t *v, int i)
{
  lval_t *x = v->expr.cell[i];

  memmove(&v->expr.cell[i], &v->expr.cell[i+1],
      sizeof(lval_t*) * (v->expr.count-i-1));

  v->expr.count--;

  v->expr.cell = realloc(v->expr.cell,
      sizeof(lval_t*) * v->expr.count);

  return x;
}

lval_t*
lval_take(lval_t *v, int i)
{
  lval_t *x = lval_pop(v, i);
  lval_del(v);

  return x;
}

lval_t*
lval_join(lval_t *x, lval_t *y)
{
  while (y->expr.count) {
    x = lval_add(x, lval_pop(y, 0));
	}

  lval_del(y);

  return x;
}
