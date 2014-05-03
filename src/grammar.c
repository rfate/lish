#include "grammar.h"

void grammar_init(void) {
  token_comment   = mpc_new("comment");
  token_integer   = mpc_new("integer");
  token_float     = mpc_new("float");
  token_boolean   = mpc_new("boolean");
  token_string    = mpc_new("string");
  token_symbol    = mpc_new("symbol");
  token_litsymbol = mpc_new("litsymbol");
  token_operator  = mpc_new("operator");
  token_tablepair = mpc_new("tablepair");
  token_table     = mpc_new("table");
  token_sexpr     = mpc_new("sexpr");
  token_qexpr     = mpc_new("qexpr");
  token_expr      = mpc_new("expr");
  grammar_lish    = mpc_new("lish");

  mpca_lang(MPC_LANG_DEFAULT,
    "                                                            \
      tablepair: <expr> \"=\" <expr>                            ;\
      table    : '[' (<tablepair>+ (/, */ <tablepair>)*)* ']'   ;\
                                                                 \
      integer  : /-?[0-9]+/                                     ;\
      float    : /-?[0-9]+\\.[0-9]+/                            ;\
      string   : /\"(\\\\.|[^\"])*\"/                           ;\
      symbol   : /[a-zA-Z0-9_\\/\\\\λ\\.!]+/                    ;\
      litsymbol: '@'<symbol>                                    ;\
      operator : /[\\?=<>&%+\\-*]+/                             ;\
      boolean  : \"true\" | \"false\"                           ;\
      comment  : /#[^\\r\\n]*/                                  ;\
      sexpr    : '(' <expr>* ')'                                ;\
      qexpr    : '{' <expr>* '}'                                ;\
                                                                 \
      expr     : <float> | <integer> | <string> | <boolean>      \
               | <sexpr> | <qexpr>  | <comment> | <litsymbol>    \
               | <symbol> | <operator> | <table>                ;\
                                                                 \
      lish     : /^/ <expr>* /$/                                ;\
    ",
			token_tablepair, token_table, token_integer, token_float,
      token_string, token_symbol, token_litsymbol, token_operator,
      token_boolean, token_comment, token_sexpr, token_qexpr,
      token_expr, grammar_lish);
}

void grammar_cleanup(void) {
  mpc_cleanup(13, token_tablepair, token_table, token_integer,
    token_float, token_string, token_symbol, token_operator,
    token_boolean, token_comment, token_litsymbol, token_sexpr,
    token_qexpr, token_expr, grammar_lish);
}

