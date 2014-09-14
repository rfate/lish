#ifndef _LISH_ASSERT_H
#define _LISH_ASSERT_H

#define LASSERT(args, cond, fmt, ...)           \
  if (!(cond)) {                                \
    lval_t* err = lval_err(fmt, ##__VA_ARGS__); \
    lval_del(args);                             \
    return err;                                 \
  }

#define LASSERT_ARG_COUNT(name, args, c)                                       \
  LASSERT(args, args->expr.count == c,                                    \
    "Builtin \"%s\" given incorrect number of arguments. Expected %d, got %d", \
    name, c, args->expr.count)

#define LASSERT_NONEMPTY_LIST(name, args, argn)                         \
  LASSERT(args, args->expr.cell[argn]->expr.count != 0,       \
    "Builtin \"%s\" given empty list for argument %d.", name, (argn+1))

#define LASSERT_ARG_TYPE(name, args, index, _type)                \
  LASSERT(args, args->expr.cell[index]->type == _type,       \
      "Builtin \"%s\" expected arg %d to be of type %s, got %s.", \
          name, (index+1), ltype_name(_type),                     \
          ltype_name(args->expr.cell[index]->type))          \

#define LASSERT_ARG_IS_NUM(name, args, index)                             \
  LASSERT(args, (args->expr.cell[index]->type == LVAL_INT            \
              || args->expr.cell[index]->type == LVAL_FLOAT),        \
          "Builtin \"%s\" expected arg %d to be Number, got %s.",         \
          name, (index+1), ltype_name(args->expr.cell[index]->type))

#define LASSERT_ARG_ITERABLE(name, args, index)                                  \
  LASSERT(args, (args->expr.cell[index]->type == LVAL_QEXPR                 \
              || args->expr.cell[index]->type == LVAL_STR                   \
              || args->expr.cell[index]->type == LVAL_TABLE),               \
    "Builtin \"%s\" expected arg %d of type %s to have length. Accepted types: " \
    "Q-Expression, String, Table",                                               \
     name, (index+1), ltype_name(args->expr.cell[index]->type));

#endif

