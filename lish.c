#include <stdio.h>
#include <stdlib.h>

#include <editline/readline.h>
#include <editline/history.h>

#include "mpc.h"
#include "lish.h"
#include "lval.h"

int main(int argc, const char* argv[]) {
	mpc_parser_t* Number = mpc_new("number");
	mpc_parser_t* Symbol = mpc_new("symbol");
	mpc_parser_t* Sexpr  = mpc_new("sexpr");
	mpc_parser_t* Qexpr  = mpc_new("qexpr");
	mpc_parser_t* Expr   = mpc_new("expr");
	mpc_parser_t* Lish   = mpc_new("lish");

	mpca_lang(MPC_LANG_DEFAULT,
		"                                                    \
			number : /-?[0-9]+(\\.[0-9]+)?/                   ;\
			symbol : '+' | '-' | '*' | '/' | '%' | '^'         \
			       | \"list\" | \"head\" | \"tail\" | \"join\" \
			       | \"eval\"                                 ;\
			sexpr  : '(' <expr>* ')'                          ;\
			qexpr  : '{' <expr>* '}'                          ;\
			expr   : <number> | <symbol> | <sexpr> | <qexpr>  ;\
			lish   : /^/ <expr>* /$/                          ;\
		",
		Number, Symbol, Sexpr, Qexpr, Expr, Lish);


	puts("Lish " LISH_VERSION);

	while (1) {
		char* input = readline("lish> ");

		add_history(input);

		mpc_result_t r;
		if (mpc_parse("<stdin>", input, Lish, &r)) {
			lval_t* x = lval_eval(lval_read(r.output));
			lval_println(x);
			lval_del(x);
			mpc_ast_delete(r.output);
		} else {
			mpc_err_print(r.error);
			mpc_err_delete(r.error);
		}

		free(input);
	}

	mpc_cleanup(6, Number, Symbol, Sexpr, Qexpr, Expr, Lish);

	return 0;
}
