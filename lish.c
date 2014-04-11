#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <editline/readline.h>
#include <editline/history.h>

#include "lish.h"
#include "mpc.h"

lval_t lval_num(double i) {
	lval_t v;
	v.type = LVAL_NUM;
	v.num  = i;

	return v;
}

lval_t lval_err(int e) {
	lval_t v;
	v.type = LVAL_ERR;
	v.err  = e;

	return v;
}

void lval_print(lval_t v) {
	switch (v.type) {
		case LVAL_NUM:
			printf("%lf", v.num);
			break;

		case LVAL_ERR:
			if (v.err == LERR_DIV_ZERO)
				printf("Error: division by zero.");
			if (v.err == LERR_BAD_OP)
				printf("Error: invalid operation.");
			if (v.err == LERR_BAD_NUM)
				printf("Error: invalid number.");

			break;
	}
}

void lval_println(lval_t v) {
	lval_print(v);
	putchar('\n');
}
///

lval_t eval(mpc_ast_t* t) {
	if (strstr(t->tag, "number")) {
		errno = 0;
		double v = strtod(t->contents, NULL);
		return (errno != ERANGE) ? lval_num(v) : lval_err(LERR_BAD_NUM);
	}

	// Operator is always second child.
	char* op = t->children[1]->contents;

	lval_t x = eval(t->children[2]);

	int i = 3;
	while (strstr(t->children[i]->tag, "expr")) {
		x = eval_op(op, x, eval(t->children[i]));
		++i;
	}

	return x;
}

lval_t eval_op(char* op, lval_t x, lval_t y) {
	if (x.type == LVAL_ERR)
		return x;
	if (y.type == LVAL_ERR)
		return y;

	if (strcmp(op, "+") == 0)
		return lval_num(x.num + y.num);
	if (strcmp(op, "-") == 0)
		return lval_num(x.num - y.num);
	if (strcmp(op, "*") == 0)
		return lval_num(x.num * y.num);
	if (strcmp(op, "^") == 0)
		return lval_num(pow(x.num, y.num));
	if (strcmp(op, "%") == 0)
		return (y.num == 0) ? lval_err(LERR_DIV_ZERO) : lval_num(fmod(x.num, y.num));
	if (strcmp(op, "/") == 0)
		return (y.num == 0) ? lval_err(LERR_DIV_ZERO) : lval_num(x.num / y.num);

	return lval_err(LERR_BAD_OP);
}

int main(int argc, const char* argv[]) {
	mpc_parser_t* Number   = mpc_new("number");
	mpc_parser_t* Operator = mpc_new("operator");
	mpc_parser_t* Expr     = mpc_new("expr");
	mpc_parser_t* Lish     = mpc_new("lish");

	mpca_lang(MPC_LANG_DEFAULT,
		"                                                    \
			number   : /-?[0-9]+(\\.[0-9]+)?/ ;                \
			operator : '+' | '-' | '*' | '/' | '%' | '^' ;     \
			expr     : <number> | '(' <operator> <expr>+ ')' ; \
			lish     : /^/ <operator> <expr>+ /$/ ;            \
		",
		Number, Operator, Expr, Lish);


	puts("Lish " LISH_VERSION);

	while (1) {
		char* input = readline("lish> ");

		add_history(input);

		mpc_result_t r;
		if (mpc_parse("<stdin>", input, Lish, &r)) {
			lval_t result = eval(r.output);
			lval_println(result);
			mpc_ast_delete(r.output);
		} else {
			mpc_err_print(r.error);
			mpc_err_delete(r.error);
		}

		free(input);
	}

	mpc_cleanup(4, Number, Operator, Expr, Lish);

	return 0;
}

