#include "parser.h"

void parser_init(void) {
  Comment   = mpc_new("comment");
  Integer   = mpc_new("integer");
  Float     = mpc_new("float");
  Boolean   = mpc_new("boolean");
  String    = mpc_new("string");
  Symbol    = mpc_new("symbol");
  Operator  = mpc_new("operator");
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
      lish     : /^/ <expr>* /$/                                ;\
    ",
			TableKey, TableVal, TablePair, Table, Integer, Float, String, Symbol,
				Operator, Boolean, Comment, Sexpr, Qexpr, Expr, Lish);
}

void parser_cleanup(void) {
  mpc_cleanup(14, Integer, Float, String, Symbol, Operator, Boolean, Comment,
      TableKey, TableVal, Table, Sexpr, Qexpr, Expr, Lish);
}

