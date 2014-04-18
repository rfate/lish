#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <editline/readline.h>
#include <editline/history.h>

#include "mpc.h"
#include "lish.h"
#include "lval.h"
#include "lenv.h"
#include "parser.h"

int main(int argc, char** argv) {
  Comment = mpc_new("comment");
	Number  = mpc_new("number");
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
			symbol  : /[a-zA-Z0-9_+\\-*\\/\\\\=<>!&%λ\\.]+/    ;\
      comment : /#[^\\r\\n]*/                            ;\
			sexpr   : '(' <expr>* ')'                          ;\
			qexpr   : '{' <expr>* '}'                          ;\
			expr    : <number> | <string> | <symbol>            \
              | <sexpr> | <qexpr> | <comment>            ;\
			lish    : /^/ <expr>* /$/                          ;\
		",
		Number, String, Symbol, Comment, Sexpr, Qexpr, Expr, Lish);


	lenv_t* env = lenv_new();
	lenv_add_builtins(env);

  bool repl = (argc < 2);

  // load stdlib
  lval_t* libName = lval_add(lval_sexpr(), lval_str("lib/core.lish"));
  lval_t* lib = builtin_load(env, libName);
  if (lib->type == LVAL_ERR) { lval_println(lib); }
  lval_del(lib);

	if (!repl) {
		for (int i = 1; i < argc; ++i) {
			lval_t* args = lval_add(lval_sexpr(), lval_str(argv[i]));

			lval_t* x = builtin_load(env, args);
			if (x->type == LVAL_ERR) { lval_println(x); }
			lval_del(x);
		}
  } else {
		puts("Lish " LISH_VERSION);
  }

	while (repl) {
		mpc_result_t r;

		char* input = readline("lish> ");
		add_history(input);

		if (mpc_parse("<stdin>", input, Lish, &r)) {
			lval_t* x = lval_eval(env, lval_read(r.output));
        		lval_println(x);
			lval_del(x);

			mpc_ast_delete(r.output);
		} else {
			mpc_err_print(r.error);
			mpc_err_delete(r.error);
		}

		free(input);
	}

	lenv_del(env);
	mpc_cleanup(8, Comment, Number, String, Symbol, Sexpr, Qexpr, Expr, Lish);

	return 0;
}
