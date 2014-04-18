#include <editline/readline.h>
#include <editline/history.h>

#include "interpreter.h"
#include "parser.h"
#include "mpc.h"

interpreter_t* interpreter_new(void) {
  interpreter_t* in = malloc(sizeof(interpreter_t));

  Comment = mpc_new("comment");
  Number  = mpc_new("number");
  Boolean = mpc_new("boolean");
  String  = mpc_new("string");
  Symbol  = mpc_new("symbol");
  Sexpr   = mpc_new("sexpr");
  Qexpr   = mpc_new("qexpr");
  Expr    = mpc_new("expr");
  Lish    = mpc_new("lish");

  mpca_lang(MPC_LANG_DEFAULT,
    "                                                     \
      number  : /-?[0-9]+(\\.[0-9]+)?/                   ;\
      string  : /\"(\\\\.|[^\"])*\"/                     ;\
      symbol  : /[a-zA-Z0-9_+\\-*\\/\\\\=<>!&%λ\\.\\?]+/ ;\
      boolean : \"true\" | \"false\"                     ;\
      comment : /#[^\\r\\n]*/                            ;\
      sexpr   : '(' <expr>* ')'                          ;\
      qexpr   : '{' <expr>* '}'                          ;\
      expr    : <number> | <string> | <boolean>           \
              | <sexpr> | <qexpr> | <comment>             \
              | <symbol>                                 ;\
      lish    : /^/ <expr>* /$/                          ;\
    ",
    Number, String, Symbol, Boolean, Comment, Sexpr, Qexpr, Expr, Lish);

  in->env = lenv_new();
  lenv_add_builtins(in->env);

  interpreter_load_file(in, "lib/core.lish", 1);

  return in;
}

void interpreter_load_file(interpreter_t* in, char* filename, int usePath) {
  lval_t* libArgs = lval_sexpr();
  libArgs = lval_add(libArgs, lval_str(filename));
  libArgs = lval_add(libArgs, lval_bool(usePath));

  lval_t* lib = builtin_load(in->env, libArgs);
  if (lib->type == LVAL_ERR) { lval_println(lib); }
  lval_del(lib);
}

void interpreter_del(interpreter_t* in) {
  lenv_del(in->env);
  free(in);
  mpc_cleanup(9, Comment, Boolean, Number, String, Symbol, Sexpr, Qexpr, Expr, Lish);
}

void interpreter_repl(interpreter_t* in) {
  puts("Lish " LISH_VERSION);

  while (1) {
    mpc_result_t r;

    char* input = readline("lish> ");
    add_history(input);

    if (mpc_parse("<stdin>", input, Lish, &r)) {
      lval_t* x = lval_eval(in->env, lval_read(r.output));
      lval_println(x);
      lval_del(x);

      mpc_ast_delete(r.output);
    } else {
      mpc_err_print(r.error);
      mpc_err_delete(r.error);
    }

    free(input);
  }
}
