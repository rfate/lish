#include "linenoise.h"
#include "lish.h"
#include "parser.h"
#include "mpc.h"

lish_t* lish_new(void) {
  lish_t* in = malloc(sizeof(lish_t));

  Comment   = mpc_new("comment");
  Integer   = mpc_new("integer");
  Float     = mpc_new("float");
  Boolean   = mpc_new("boolean");
  String    = mpc_new("string");
  Symbol    = mpc_new("symbol");
  Table     = mpc_new("table");
  TableKey  = mpc_new("tablekey");
  TableVal  = mpc_new("tableval");
  TablePair = mpc_new("tablepair");
  Sexpr     = mpc_new("sexpr");
  Qexpr     = mpc_new("qexpr");
  Expr      = mpc_new("expr");
  Lish      = mpc_new("lish");

  mpca_lang(MPC_LANG_DEFAULT,
    "                                                            \
      tablekey : <symbol> | <string>                            ;\
      tableval : <float> | <integer> | <string> | <symbol>      ;\
      tablepair: <tablekey> \"=\" <tableval>                    ;\
      table    : '[' <tablepair>+ (/, */ <tablepair>)* ']' ;\
                                                                 \
      integer  : /-?[0-9]+/                                     ;\
      float    : /-?[0-9]+\\.[0-9]+/                            ;\
      string   : /\"(\\\\.|[^\"])*\"/                           ;\
      symbol   : /[a-zA-Z0-9_+\\-*\\/\\\\=<>!&%λ\\.\\?]+/       ;\
      boolean  : \"true\" | \"false\"                           ;\
      comment  : /#[^\\r\\n]*/                                  ;\
      sexpr    : '(' <expr>* ')'                                ;\
      qexpr    : '{' <expr>* '}'                                ;\
                                                                 \
      expr     : <float> | <integer> | <string> | <boolean>      \
               | <sexpr> | <qexpr>  | <comment> | <symbol>       \
               | <table>                                        ;\
      lish     : /^/ <expr>* /$/                                ;\
    ",
			TableKey, TableVal, TablePair, Table, Integer, Float, String, Symbol,
				Boolean, Comment, Sexpr, Qexpr, Expr, Lish);

  in->env = lenv_new();
  lenv_add_builtins(in->env);

  lish_load_file(in, "lib/core.lish", 1);

  return in;
}

void lish_set_argv(lish_t* in, int argc, char** argv) {
  lval_t* argvKey   = lval_sym("ARGV");
  lval_t* argvConst = lval_qexpr();

  for (int i = 1; i < argc; ++i) {
    argvConst = lval_add(argvConst, lval_str(argv[i]));
  }

  lenv_def(in->env, argvKey, argvConst);
  lval_del(argvKey);
  lval_del(argvConst);
}

void lish_load_file(lish_t* in, char* filename, int usePath) {
  lval_t* libArgs = lval_sexpr();
  libArgs = lval_add(libArgs, lval_str(filename));
  libArgs = lval_add(libArgs, lval_bool(usePath));

  lval_t* lib = builtin_load(in->env, libArgs);
  if (lib->type == LVAL_ERR) { lval_println(lib); }
  lval_del(lib);
}

void lish_del(lish_t* in) {
  lenv_del(in->env);
  free(in);
  mpc_cleanup(11, Integer, Float, String, Symbol, Boolean, Comment,
      TableKey, TableVal, Table, Sexpr, Qexpr, Expr, Lish);
}

void lish_repl(lish_t* in) {
  puts("Lish " LISH_VERSION);

  while (1) {
    mpc_result_t r;

    char* input = linenoise("lish> ");

    if (!input)
      break;

    linenoiseHistoryAdd(input);

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
