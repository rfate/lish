#include "parser.h"

void parser_init(void) {
  parser_comment   = mpc_new("comment");
  parser_integer   = mpc_new("integer");
  parser_float     = mpc_new("float");
  parser_boolean   = mpc_new("boolean");
  parser_string    = mpc_new("string");
  parser_symbol    = mpc_new("symbol");
  parser_operator  = mpc_new("operator");
  parser_tablekey  = mpc_new("tablekey");
  parser_tableval  = mpc_new("tableval");
  parser_tablepair = mpc_new("tablepair");
  parser_table     = mpc_new("table");
  parser_sexpr     = mpc_new("sexpr");
  parser_qexpr     = mpc_new("qexpr");
  parser_expr      = mpc_new("expr");
  parser_lish      = mpc_new("lish");

  mpca_lang(MPC_LANG_DEFAULT,
    "                                                            \
      tablekey : <symbol> | <string>                            ;\
      tableval : <float> | <integer> | <string> | <symbol>      ;\
      tablepair: <tablekey> \"=\" <tableval>                    ;\
      table    : '[' <tablepair>+ (/, */ <tablepair>)* ']'      ;\
                                                                 \
      integer  : /-?[0-9]+/                                     ;\
      float    : /-?[0-9]+\\.[0-9]+/                            ;\
      string   : /\"(\\\\.|[^\"])*\"/                           ;\
      symbol   : /[a-zA-Z0-9_\\/\\\\λ\\.]+/                     ;\
      operator : /[\\?=<>!&%+\\-*]+/                            ;\
      boolean  : \"true\" | \"false\"                           ;\
      comment  : /#[^\\r\\n]*/                                  ;\
      sexpr    : '(' <expr>* ')'                                ;\
      qexpr    : '{' <expr>* '}'                                ;\
                                                                 \
      expr     : <float> | <integer> | <string> | <boolean>      \
               | <sexpr> | <qexpr>  | <comment> | <symbol>       \
               | <operator> | <table>                           ;\
                                                                 \
      lish     : /^/ <expr>* /$/                                ;\
    ",
			parser_tablekey, parser_tableval, parser_tablepair, parser_table,
      parser_integer, parser_float, parser_string, parser_symbol,
      parser_operator, parser_boolean, parser_comment, parser_sexpr,
      parser_qexpr, parser_expr, parser_lish);
}

void parser_cleanup(void) {
  mpc_cleanup(14, parser_tablekey, parser_tableval, parser_tablepair,
    parser_table, parser_integer, parser_float, parser_string,
    parser_symbol, parser_operator, parser_boolean, parser_comment,
    parser_sexpr, parser_qexpr, parser_expr, parser_lish);
}

