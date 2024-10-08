#include <stdio.h>
#include <stdlib.h>

#include "mpc.h"

#ifdef _WIN32
#include <string.h>

#define INPUT_BUFFER_SIZE 2048
static char buffer[INPUT_BUFFER_SIZE];

/* Fake readline function */
char *readline(char *prompt) {
  fputs(prompt, stdout);
  fgets(buffer, INPUT_BUFFER_SIZE, stdin);
  char *cpy = malloc(strlen(buffer) + 1);
  strncpy(cpy, INPUT_BUFFER_SIZE, buffer);
}

/* Fake add_history function */
void add_history(char *unused) {}

#else
/* arch linux specific, history.h for other linux distros */
#include <editline/readline.h>
/* #include <editline/history.h> */
#endif

#define LASSERT(args, cond, fmt, ...)                                          \
  if (!(cond)) {                                                               \
    lval *err = lval_err(fmt, ##__VA_ARGS__);                                  \
    lval_del(args);                                                            \
    return err;                                                                \
  }

#define LASSERT_TYPE(args, func, arg, expected)                                \
  LASSERT(args, args->cell[arg]->type == expected,                             \
          "Function '%s' passed incorrect type for argument %i. "              \
          "Got %s, Expected %s.",                                              \
          func, arg, ltype_name(args->cell[arg]->type), ltype_name(expected));

#define LASSERT_ARG_COUNT(args, func, expected)                                \
  LASSERT(args, args->count == expected,                                       \
          "Function '%s' passed too many arguments! "                          \
          "got %i, expected %i",                                               \
          args->count, expected);

struct lval;
struct lenv;
typedef struct lval lval;
typedef struct lenv lenv;

/* Create Enumeration of Possible lval Types */
enum { LVAL_NUM, LVAL_ERR, LVAL_SYM, LVAL_STR, LVAL_FUN, LVAL_SEXPR, LVAL_QEXPR };
/* Create Enumeration of Possible Error Types */
enum { LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM };

/* Declare New Lisp Value struct */
typedef lval *(*lbuiltin)(lenv *, lval *);

struct lval {
  int type;

  // Basics
  long num;
  char *err;
  char *sym;
  char *str;

  // Function
  lbuiltin builtin;
  lenv *env;
  lval *formals;
  lval *body;

  // Expression
  int count;
  lval **cell;
};

struct lenv {
  lenv *par;
  int count;
  char **syms;
  lval **vals;
};

char *ltype_name(int t) {
  switch (t) {
  case LVAL_FUN:
    return "Function";
  case LVAL_NUM:
    return "Number";
  case LVAL_ERR:
    return "Error";
  case LVAL_SYM:
    return "Symbol";
  case LVAL_STR:
    return "String";
  case LVAL_SEXPR:
    return "S-Expression";
  case LVAL_QEXPR:
    return "Q-Expression";
  default:
    return "Unknown";
  }
}

lval *lval_sexpr(void);
lval *lval_qexpr(void);
lval *lval_sym(char *s);
lval *lval_read_str(mpc_ast_t* t);
lval *lval_err(char *fmt, ...);
lval *lval_num(long x);
lval *lval_str(char *s);
lval *lval_fun(lbuiltin func);
lval *lval_lambda(lval *formals, lval *body);

int number_of_nodes(mpc_ast_t *t);
lval *eval_op(lval *x, char *op, lval *y);
lval *eval(mpc_ast_t *t);
lval *lval_read_num(mpc_ast_t *t);
lval *lval_read(mpc_ast_t *t);
lval *lval_add(lval *v, lval *x);
lval *lval_copy(lval *v);
void lval_expr_print(lenv *e, lval *v, char open, char close);
void lval_del(lval *v);
lval *lval_pop(lval *v, int i);

lval *builtin(lenv *e, lval *a, char *func);
lval *builtin_op(lenv *e, lval *a, char *op);
lval *builtin_head(lenv *e, lval *a);
lval *builtin_tail(lenv *e, lval *a);
lval *builtin_list(lenv *e, lval *a);
lval *builtin_eval(lenv *e, lval *a);
lval *builtin_join(lenv *e, lval *a);
lval *builtin_add(lenv *e, lval *a);
lval *builtin_sub(lenv *e, lval *a);
lval *builtin_mul(lenv *e, lval *a);
lval *builtin_div(lenv *e, lval *a);
lval *builtin_def(lenv *e, lval *a);
lval *builtin_put(lenv *e, lval *a);
lval *builtin_var(lenv *e, lval *a, char *func);
lval *builtin_lambda(lenv *e, lval *a);
lval *builtin_ord(lenv *e, lval *a, char *op);
lval *builtin_gt(lenv *e, lval *a);
lval *builtin_lt(lenv *e, lval *a);
lval *builtin_ge(lenv *e, lval *a);
lval *builtin_le(lenv *e, lval *a);
lval *builtin_cmp(lenv *e, lval *a, char *op);
lval *builtin_eq(lenv *e, lval *a);
lval *builtin_ne(lenv *e, lval *a);
lval *builtin_if(lenv *e, lval *a);

lval *lval_join(lenv *e, lval *x, lval *y);
lval *lval_take(lenv *e, lval *v, int i);
lval *lval_eval(lenv *e, lval *v);
lval *lval_eval_sexpr(lenv *e, lval *v);
lval *lval_call(lenv *e, lval *f, lval *a);
int lval_eq(lval *x, lval *y);
void lval_print(lenv *e, lval *v);
void lval_println(lenv *e, lval *v);

mpc_parser_t *Number;
mpc_parser_t *Symbol;
mpc_parser_t *String;
mpc_parser_t *Comment;
mpc_parser_t *Sexpr;
mpc_parser_t *Qexpr;
mpc_parser_t *Expr;
mpc_parser_t *Lispy;



lenv *lenv_new(void) {
  lenv *e = malloc(sizeof(lenv));
  e->par = NULL;
  e->count = 0;
  e->syms = NULL;
  e->vals = NULL;
  return e;
}

void lenv_del(lenv *e);
void lenv_del(lenv *e) {
  for (int i = 0; i < e->count; i++) {
    free(e->syms[i]);
    lval_del(e->vals[i]);
  }
  free(e->syms);
  free(e->vals);
  free(e);
}

lval *lenv_get(lenv *e, lval *k) {
  for (int i = 0; i < e->count; i++) {
    if (strcmp(e->syms[i], k->sym) == 0) {
      return lval_copy(e->vals[i]);
    }
  }

  if (e->par) {
    return lenv_get(e->par, k);
  } else {
    return lval_err("Unbound Symbol '%s'", k->sym);
  }
  /* TODO: enhance this with suggestions: did you mean xy */
  return lval_err("unbound symbol '%s'!", k->sym);
}

void lenv_put(lenv *e, lval *k, lval *v) {
  for (int i = 0; i < e->count; i++) {
    if (strcmp(e->syms[i], k->sym) == 0) {
      lval_del(e->vals[i]);
      e->vals[i] = lval_copy(v);
      return;
    }
  }

  /* if no entry exists -> allocate space for new entry */
  e->count++;
  e->vals = realloc(e->vals, sizeof(lval *) * e->count);
  e->syms = realloc(e->syms, sizeof(char *) * e->count);

  e->vals[e->count - 1] = lval_copy(v);
  e->syms[e->count - 1] = malloc(strlen(k->sym) + 1);
  strcpy(e->syms[e->count - 1], k->sym);
}

void lenv_def(lenv *e, lval *k, lval *v) {
  /*   iterate till e has no parent */
  while (e->par) {
    e = e->par;
  }
  lenv_put(e, k, v);
}

lenv *lenv_copy(lenv *e) {
  lenv *n = malloc(sizeof(lenv));
  n->par = e->par;
  n->count = e->count;
  n->vals = malloc(sizeof(lval *) * n->count);
  n->syms = malloc(sizeof(char *) * n->count);
  for (int i = 0; i < e->count; i++) {
    n->syms[i] = malloc(strlen(e->syms[i]) + 1);
    strcpy(n->syms[i], e->syms[i]);
    n->vals[i] = lval_copy(e->vals[i]);
  }
  return n;
}

lval* builtin_load(lenv* e, lval* a) {
  LASSERT_ARG_COUNT(a, "load", 1);
  LASSERT_TYPE(a, "load", 0, LVAL_STR);

  /*   parse file given by string name */
  mpc_result_t r;
  if (mpc_parse_contents(a->cell[0]->str, Lispy, &r)) {
    /*     read contents */
    lval* expr = lval_read(r.output);
    mpc_ast_delete(r.output);

    /*     evaluate each expression */
    while (expr->count) {
      lval* x = lval_eval(e, lval_pop(expr, 0));
      /*       if evaluation leads to error print it */
      if (x->type == LVAL_ERR) { lval_println(e, x); }
      lval_del(x);
    }

    /*     delete expressions and arguments */
    lval_del(expr);
    lval_del(a);

    return lval_sexpr();
  } else {
    /*     get parse error as string */
    char* err_msg = mpc_err_string(r.error);
    mpc_err_delete(r.error);

    /*     create new error message using it */
    lval* err = lval_err("Could not load library %s", err_msg);
    free(err_msg);
    lval_del(a);

    return err;
  }
}

lval* builtin_print(lenv* e, lval* a) {
  /*   print each argument followed by a space */
  for (int i = 0; i < a->count; i++) {
    lval_print(e, a->cell[i]); putchar(' ');
  }

  putchar('\n');
  lval_del(a);

  return lval_sexpr();
}

lval* builtin_error(lenv* e, lval* a) {
  LASSERT_ARG_COUNT(a, "error", 1);
  LASSERT_TYPE(a, "error", 0, LVAL_STR);

  lval* err = lval_err(a->cell[0]->str);

  lval_del(a);
  return err;
}

void lenv_add_builtin(lenv *e, char *name, lbuiltin func) {
  lval *k = lval_sym(name);
  lval *v = lval_fun(func);
  lenv_put(e, k, v);
  lval_del(k);
  lval_del(v);
}

void lenv_add_builtins(lenv *e) {
  lenv_add_builtin(e, "load", builtin_load);
  lenv_add_builtin(e, "print", builtin_print);
  lenv_add_builtin(e, "error", builtin_print);


  lenv_add_builtin(e, "list", builtin_list);
  lenv_add_builtin(e, "head", builtin_head);
  lenv_add_builtin(e, "tail", builtin_tail);
  lenv_add_builtin(e, "eval", builtin_eval);
  lenv_add_builtin(e, "join", builtin_join);

  lenv_add_builtin(e, "def", builtin_def);
  lenv_add_builtin(e, "\\", builtin_lambda);
  lenv_add_builtin(e, "=", builtin_put);


  lenv_add_builtin(e, "+", builtin_add);
  lenv_add_builtin(e, "-", builtin_sub);
  lenv_add_builtin(e, "*", builtin_mul);
  lenv_add_builtin(e, "/", builtin_div);

  lenv_add_builtin(e, ">", builtin_gt);
  lenv_add_builtin(e, ">=", builtin_ge);
  lenv_add_builtin(e, "<", builtin_lt);
  lenv_add_builtin(e, "<=", builtin_le);

  lenv_add_builtin(e, "!=", builtin_ne);
  lenv_add_builtin(e, "==", builtin_eq);
  lenv_add_builtin(e, "if", builtin_if);
}


int main(int argc, char **argv) {
  Number = mpc_new("number");
  Symbol = mpc_new("symbol");
  String = mpc_new("string");
  Comment = mpc_new("comment");
  Sexpr = mpc_new("sexpr");
  Qexpr = mpc_new("qexpr");
  Expr = mpc_new("expr");
  Lispy = mpc_new("lispy");

  mpca_lang(MPCA_LANG_DEFAULT,
            "                                                 \
      number     : /-?[0-9]+/ ;                       \
      symbol     : /[a-zA-Z0-9_+\\-*\\/\\\\=<>!&]+/ ; \
      string     : /\"(\\\\.|[^\"])*\"/ ; \
      comment    : /;[^\\r\\n]*/ ; \
      sexpr      : '(' <expr>* ')' ;             \
      qexpr      : '{' <expr>* '}' ;             \
      expr       : <number> | <symbol> | <string> \
                 | <comment> | <sexpr> | <qexpr>; \
      lispy      : /^/ <expr>* /$/ ;    \
    ",
            Number, Symbol, String, Comment, Sexpr, Qexpr, Expr, Lispy);

  puts("Lispy Version 0.0.0.0.1");
  puts("Press Ctrl+c to Exit\n");

  lenv *e = lenv_new();
  lenv_add_builtins(e);

  if (argc >= 2) {
    for (int i = 1; i < argc; i++) {
      lval *args = lval_add(lval_sexpr(), lval_str(argv[i]));

      lval* x = builtin_load(e, args);

      if (x->type == LVAL_ERR) {
        lval_println(e, x);
      }
      lval_del(x);
    }
  } else {
    while (1) {
      /* fgets don't let you edit the line by navigating with arrow keys
       * e.g. */
      /* fgets(input, INPUT_BUFFER, stdin); */

      char *input = readline("lispy> ");

      add_history(input);

      mpc_result_t r;
      if (mpc_parse("<stdin>", input, Lispy, &r)) {
        /* On success evaluate the AST */
        /* lval* result = eval(r.output); */
        lval *x = lval_eval(e, lval_read(r.output));
        lval_println(e, x);
        lval_del(x);
        mpc_ast_delete(r.output);
      } else {
        /* Otherwise Print the Error */
        mpc_err_print(r.error);
        mpc_err_delete(r.error);
      }
      free(input);
    }
  }
  lenv_del(e);

  mpc_cleanup(8, Number, Symbol, String, Comment, Sexpr, Qexpr, Expr, Lispy);
  return 0;
}

lval *eval(mpc_ast_t *t) {

  /* If tagged as number return it directly. */
  if (strstr(t->tag, "number")) {
    errno = 0;
    long x = strtol(t->contents, NULL, 10);
    return errno != ERANGE ? lval_num(x) : lval_err("invalid number");
  }

  /* The operator is always second child. */
  char *op = t->children[1]->contents;

  /* We store the third child in `x` */
  lval *x = eval(t->children[2]);

  /* Iterate the remaining children and combining. */
  int i = 3;
  while (strstr(t->children[i]->tag, "expr")) {
    x = eval_op(x, op, eval(t->children[i]));
    i++;
  }

  return x;
}

lval *eval_op(lval *x, char *op, lval *y) {
  if (x->type == LVAL_ERR) {
    return x;
  }
  if (y->type == LVAL_ERR) {
    return y;
  }

  if (strcmp(op, "+") == 0) {
    return lval_num(x->num + y->num);
  }
  if (strcmp(op, "-") == 0) {
    return lval_num(x->num - y->num);
  }
  if (strcmp(op, "*") == 0) {
    return lval_num(x->num * y->num);
  }
  if (strcmp(op, "/") == 0) {
    return y->num == 0 ? lval_err(LERR_DIV_ZERO) : lval_num(x->num / y->num);
  }
  return lval_err("invalid operator");
}

lval *lval_read_num(mpc_ast_t *t) {
  errno = 0;
  long x = strtol(t->contents, NULL, 10);
  return errno != ERANGE ? lval_num(x) : lval_err("invalid number");
}

lval *lval_read(mpc_ast_t *t) {
  if (strstr(t->tag, "number")) {
    return lval_read_num(t);
  }
  if (strstr(t->tag, "symbol")) {
    return lval_sym(t->contents);
  }
  if (strstr(t->tag, "string")) {
    return lval_read_str(t);
  }

  lval *x = NULL;
  if (strcmp(t->tag, ">") == 0) {
    x = lval_sexpr();
  }
  if (strstr(t->tag, "sexpr")) {
    x = lval_sexpr();
  }
  if (strstr(t->tag, "qexpr")) {
    x = lval_qexpr();
  }

  for (int i = 0; i < t->children_num; i++) {
    if (strcmp(t->children[i]->contents, "(") == 0) {
      continue;
    }
    if (strcmp(t->children[i]->contents, ")") == 0) {
      continue;
    }
    if (strcmp(t->children[i]->contents, "{") == 0) {
      continue;
    }
    if (strcmp(t->children[i]->contents, "}") == 0) {
      continue;
    }
    if (strcmp(t->children[i]->tag, "regex") == 0) {
      continue;
    }
    if (strstr(t->children[i]->tag, "comment")) {
      continue;
    }
    x = lval_add(x, lval_read(t->children[i]));
  }
  return x;
}

lval *lval_copy(lval *v) {
  lval *x = malloc(sizeof(lval));
  x->type = v->type;

  switch (v->type) {
  /* copy functions and numbers directly */
  case LVAL_FUN:
    if (v->builtin) {
      x->builtin = v->builtin;
    } else {
      x->builtin = NULL;
      x->env = lenv_copy(v->env);
      x->formals = lval_copy(v->formals);
      x->body = lval_copy(v->body);
    }
    break;
  case LVAL_NUM:
    x->num = v->num;
    break;

  /* copy strings using malloc and strcpy */
  case LVAL_ERR:
    x->err = malloc(strlen(v->err) + 1);
    strcpy(x->err, v->err);
    break;
  case LVAL_SYM:
    x->sym = malloc(strlen(v->sym) + 1);
    strcpy(x->sym, v->sym);
    break;
  case LVAL_STR:
    x->str = malloc(strlen(v->str) + 1);
    strcpy(x->str, v->str);
    break;

  /* copy lists by copying each sub-expression */
  case LVAL_SEXPR:
  case LVAL_QEXPR:
    x->count = v->count;
    x->cell = malloc(sizeof(lval *) * x->count);
    for (int i = 0; i < x->count; i++) {
      x->cell[i] = lval_copy(v->cell[i]);
    }
    break;
  }
  return x;
}

lval *lval_add(lval *v, lval *x) {
  v->count++;
  v->cell = realloc(v->cell, sizeof(lval *) * v->count);
  v->cell[v->count - 1] = x;
  return v;
}

void lval_expr_print(lenv *e, lval *v, char open, char close) {
  putchar(open);
  for (int i = 0; i < v->count; i++) {
    lval_print(e, v->cell[i]);

    if (i != (v->count - 1)) {
      putchar(' ');
    }
  }
  putchar(close);
}

void lval_print_str(lval* v) {
  // make a copy of the string
  char* escaped = malloc(strlen(v->str) + 1);
  strcpy(escaped, v->str);

  /*   pass it through the escape function */
  escaped = mpcf_escape(escaped);
  /*   print it between " characters  */
  printf("\"%s\"", escaped);
  free(escaped);
}

void lval_print(lenv *e, lval *v) {
  switch (v->type) {
  case LVAL_NUM:
    printf("%li", v->num);
    break;
  case LVAL_ERR:
    printf("Error: %s", v->err);
    break;
  case LVAL_SYM:
    printf("%s", v->sym);
    break;
  case LVAL_STR:
    lval_print_str(v);
    break;
  case LVAL_SEXPR:
    lval_expr_print(e, v, '(', ')');
    break;
  case LVAL_QEXPR:
    lval_expr_print(e, v, '{', '}');
    break;
  case LVAL_FUN:
    if (v->builtin) {
      printf("<builtin>");
    } else {
      printf("(\\ ");
      lval_print(e, v->formals);
      putchar(' ');
      lval_print(e, v->body);
      putchar(')');
    }
    break;
  }
}

void lval_println(lenv *e, lval *v) {
  lval_print(e, v);
  putchar('\n');
}

void lval_del(lval *v) {
  switch (v->type) {
  case LVAL_NUM:
    break;
  case LVAL_ERR:
    free(v->err);
    break;
  case LVAL_SYM:
    free(v->sym);
    break;
  case LVAL_STR:
    free(v->str);
    break;
  case LVAL_FUN:
    if (!v->builtin) {
      lenv_del(v->env);
      lval_del(v->formals);
      lval_del(v->body);
    }
    break;
  case LVAL_QEXPR:
  case LVAL_SEXPR:
    for (int i = 0; i < v->count; i++) {
      lval_del(v->cell[i]);
    }
    free(v->cell);
    break;
  }
  free(v);
}
lval *lval_num(long x) {
  lval *v = malloc(sizeof(lval));
  v->type = LVAL_NUM;
  v->num = x;
  return v;
}

lval *lval_str(char *s) {
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_STR;
  v->str = malloc(strlen(s) + 1);
  strcpy(v->str, s);
  return v;
}

lval *lval_err(char *fmt, ...) {
  lval *v = malloc(sizeof(lval));
  v->type = LVAL_ERR;

  /* create a va list and initialize it */
  va_list va;
  va_start(va, fmt);

  /* TODO: what if we create bigger err messages
   * is it possible that the user produces buffer overflow
   * with symbol names?
   * */
  /* allocate 512 bytes of space */
  v->err = malloc(512);

  /* prinf the error string with a maximum of 511 characters */
  vsnprintf(v->err, 511, fmt, va);

  /* reallocate to number of bytes actually used */
  v->err = realloc(v->err, strlen(v->err) + 1);

  /* cleanup our va list */
  va_end(va);

  return v;
}

lval *lval_sym(char *s) {
  lval *v = malloc(sizeof(lval));
  v->type = LVAL_SYM;
  v->sym = malloc(strlen(s) + 1);
  strcpy(v->sym, s);
  return v;
}

lval *lval_read_str(mpc_ast_t* t) {
  /*   cut off the final quote character */
  t->contents[strlen(t->contents) -1] = '\0';
  /*   copy the string missing out the first quote character  */
  char* unescaped = malloc(strlen(t->contents+1) +1);
  strcpy(unescaped, t->contents+1);
  /*   pass through the unescape function */
  unescaped = mpcf_unescape(unescaped);
  /*   construct a new lval using the string */
  lval* str = lval_str(unescaped);
  free(unescaped);
  return str;
}

lval *lval_sexpr(void) {
  lval *v = malloc(sizeof(lval));
  v->type = LVAL_SEXPR;
  v->count = 0;
  v->cell = NULL;
  return v;
}

lval *lval_qexpr(void) {
  lval *v = malloc(sizeof(lval));
  v->type = LVAL_QEXPR;
  v->count = 0;
  v->cell = NULL;
  return v;
}

lval *lval_fun(lbuiltin func) {
  lval *v = malloc(sizeof(lval));
  v->type = LVAL_FUN;
  v->builtin = func;
  return v;
}

lval *lval_lambda(lval *formals, lval *body) {
  lval *v = malloc(sizeof(lval));
  v->type = LVAL_FUN;

  // set builtin to null
  v->builtin = NULL;

  // build new env
  v->env = lenv_new();

  // set formula and body
  v->formals = formals;
  v->body = body;
  return v;
}

lval *lval_pop(lval *v, int i) {
  /* find the item at i */
  lval *x = v->cell[i];

  /* shift memory after the item at i over the top */
  memmove(&v->cell[i], &v->cell[i + 1], sizeof(lval *) * (v->count - i - 1));

  /* decrease the count of items in the list */
  v->count--;

  /* Reallocate the memory used */
  v->cell = realloc(v->cell, sizeof(lval *) * v->count);
  return x;
}

lval *lval_take(lenv *e, lval *v, int i) {
  lval *x = lval_pop(v, i);
  lval_del(v);
  return x;
}

lval *builtin_add(lenv *e, lval *a) { return builtin_op(e, a, "+"); }

lval *builtin_sub(lenv *e, lval *a) { return builtin_op(e, a, "-"); }

lval *builtin_mul(lenv *e, lval *a) { return builtin_op(e, a, "*"); }

lval *builtin_div(lenv *e, lval *a) { return builtin_op(e, a, "/"); }

lval *builtin_def(lenv *e, lval *a) { return builtin_var(e, a, "def"); }

lval *builtin_put(lenv *e, lval *a) { return builtin_var(e, a, "="); }

lval *builtin_var(lenv *e, lval *a, char *func) {
  LASSERT_TYPE(a, func, 0, LVAL_QEXPR);

  /* first argument is symbol list */
  lval *syms = a->cell[0];

  /* ensure all elements of first list are symbols */
  for (int i = 0; i < syms->count; i++) {
    LASSERT_TYPE(syms, func, i, LVAL_SYM);
  }

  /* check correct number of symbols and values */
  LASSERT(a, syms->count == a->count - 1,
          "function '%s' passed to many arguments for symbols. "
          "Got %i, Expected %i.",
          func, syms->count, a->count - 1);

  /* assign copies of values to symbols */
  for (int i = 0; i < syms->count; i++) {
    /*     if 'def' define in globally. if put define in locally */
    if (strcmp(func, "def") == 0) {
      lenv_def(e, syms->cell[i], a->cell[i + 1]);
    }

    if (strcmp(func, "=") == 0) {
      lenv_put(e, syms->cell[i], a->cell[i + 1]);
    }
  }

  lval_del(a);
  return lval_sexpr();
}

lval *builtin_lambda(lenv *e, lval *a) {
  /*   check two arguments, each of which are q expressions */
  LASSERT_ARG_COUNT(a, "lambda", 2);
  LASSERT_TYPE(a, "lambda", 0, LVAL_QEXPR);
  LASSERT_TYPE(a, "lambda", 1, LVAL_QEXPR);

  /*   check first q expression contains only symbols */
  for (int i = 0; i < a->cell[0]->count; i++) {
    LASSERT(a, (a->cell[0]->cell[i]->type == LVAL_SYM),
            "Cannot define non-symbol. Got %s, Expected %s.",
            ltype_name(a->cell[0]->cell[i]->type), ltype_name(LVAL_SYM));
  }

  /*   pop first two arguments and pass them to lval_lambda */
  lval *formals = lval_pop(a, 0);
  lval *body = lval_pop(a, 0);
  lval_del(a);
  return lval_lambda(formals, body);
}

lval *builtin_op(lenv *e, lval *a, char *op) {
  /* ensure all arguments are numbers  */
  /* TODO: or symbols that generates/carries numbers */
  for (int i = 0; i < a->count; i++) {
    LASSERT_TYPE(a, "op", i, LVAL_NUM);
  }

  /* pop the first element */
  lval *x = lval_pop(a, 0);

  /* if no arguments and sub then perform unary negation */
  if ((strcmp(op, "-") == 0) && a->count == 0) {
    x->num = -x->num;
  }

  /* while there are still elements remaining */
  while (a->count > 0) {
    /* pop the next element  */
    lval *y = lval_pop(a, 0);

    if (strcmp(op, "+") == 0) {
      x->num += y->num;
    }
    if (strcmp(op, "-") == 0) {
      x->num -= y->num;
    }
    if (strcmp(op, "*") == 0) {
      x->num *= y->num;
    }
    if (strcmp(op, "/") == 0) {
      if (y->num == 0) {
        lval_del(x);
        lval_del(y);
        x = lval_err("Division by zero!");
        break;
      }
      x->num /= y->num;
    }
    lval_del(y);
  }
  lval_del(a);
  return x;
}

lval *builtin_ord(lenv *e, lval *a, char *op) {
  LASSERT_ARG_COUNT(a, "ord", 2);
  LASSERT_TYPE(a, "ord", 0, LVAL_NUM);
  LASSERT_TYPE(a, "ord", 1, LVAL_NUM);

  int r;
  if (strcmp(op, ">") == 0) {
    r = (a->cell[0]->num > a->cell[1]->num);
  }
  if (strcmp(op, ">=") == 0) {
    r = (a->cell[0]->num >= a->cell[1]->num);
  }
  if (strcmp(op, "<") == 0) {
    r = (a->cell[0]->num < a->cell[1]->num);
  }
  if (strcmp(op, "<=") == 0) {
    r = (a->cell[0]->num <= a->cell[1]->num);
  }

  lval_del(a);
  return lval_num(r);
}

lval *builtin_gt(lenv *e, lval *a) { return builtin_ord(e, a, ">"); }
lval *builtin_lt(lenv *e, lval *a) { return builtin_ord(e, a, "<"); }
lval *builtin_ge(lenv *e, lval *a) { return builtin_ord(e, a, ">="); }
lval *builtin_le(lenv *e, lval *a) { return builtin_ord(e, a, "<="); }

lval *builtin_cmp(lenv *e, lval *a, char *op) {
  LASSERT_ARG_COUNT(a, "cmp", 2);
  int r;
  if (strcmp(op, "==") == 0) {
    r = lval_eq(a->cell[0], a->cell[1]);
  }
  if (strcmp(op, "!=") == 0) {
    r = !lval_eq(a->cell[0], a->cell[1]);
  }
  lval_del(a);
  return lval_num(r);
}
lval *builtin_eq(lenv *e, lval *a) { return builtin_cmp(e, a, "=="); }

lval *builtin_ne(lenv *e, lval *a) { return builtin_cmp(e, a, "!="); }

lval *builtin_if(lenv *e, lval *a) {
  LASSERT_ARG_COUNT(a, "if", 3);
  LASSERT_TYPE(a, "if", 0, LVAL_NUM);
  LASSERT_TYPE(a, "if", 1, LVAL_QEXPR);
  LASSERT_TYPE(a, "if", 2, LVAL_QEXPR);

  /* mark both expressions as evaluable */
  lval *x;
  a->cell[1]->type = LVAL_SEXPR;
  a->cell[2]->type = LVAL_SEXPR;

  if (a->cell[0]->num) {
    x = lval_eval(e, lval_pop(a, 1));
  } else {
    x = lval_eval(e, lval_pop(a, 2));
  }

  lval_del(a);
  return x;
}

lval *lval_eval(lenv *e, lval *v) {
  if (v->type == LVAL_SYM) {
    lval *x = lenv_get(e, v);
    lval_del(v);
    return x;
  }

  /* Evaluate S-expressions */
  if (v->type == LVAL_SEXPR) {
    return lval_eval_sexpr(e, v);
  }
  /* All other lval types remain the same */
  return v;
}

lval *lval_eval_sexpr(lenv *e, lval *v) {
  /* Evaluate children */
  for (int i = 0; i < v->count; i++) {
    v->cell[i] = lval_eval(e, v->cell[i]);
  }

  /* Error checking */
  for (int i = 0; i < v->count; i++) {
    if (v->cell[i]->type == LVAL_ERR) {
      return lval_take(e, v, i);
    }
  }

  /* Empty Expression */
  if (v->count == 0) {
    return v;
  }

  /* Single Expression */
  if (v->count == 1) {
    return lval_take(e, v, 0);
  }

  /* Ensure first element is a function after evaluation */
  lval *f = lval_pop(v, 0);
  if (f->type != LVAL_FUN) {
    lval *err = lval_err("S-Expression starts with incorrect type. "
                         "Got %s, Expected %s.",
                         ltype_name(f->type), ltype_name(LVAL_FUN));
    lval_del(f);
    lval_del(v);
    return err;
  }

  /* Call builtin with operator  */
  lval *result = lval_call(e, f, v);
  lval_del(f);
  return result;
}

lval *lval_call(lenv *e, lval *f, lval *a) {
  /*   if builtin then simply call that  */
  if (f->builtin) {
    return f->builtin(e, a);
  }

  /*   record argument counts */
  int given = a->count;
  int total = f->formals->count;

  /*   while arguments still remain to be processed  */
  while (a->count) {
    /*     if we've ran out of formal arguments to bind */
    // FIXME: we can check this before iterate over
    if (f->formals->count == 0) {
      lval_del(a);
      return lval_err("Function passed to many arguments. "
                      "Got %i, Expected %i.",
                      given, total);
    }

    /*     pop the first symbol from the formals */
    lval *sym = lval_pop(f->formals, 0);

    /*     special case to deal with '&' for variable arguments */
    if (strcmp(sym->sym, "&") == 0) {
      // ensure '&' is followed by another symbol
      if (f->formals->count != 1) {
        lval_del(a);
        return lval_err("Function format invalid. "
                        "Symbol '&' not followed by single symbol.");
      }
      /*       next formal should be bound to remaining arguments */
      lval *nsym = lval_pop(f->formals, 0);
      lenv_put(f->env, nsym, builtin_list(e, a));
      lval_del(sym);
      lval_del(nsym);
      break;
    }

    /*     pop the next argument from the list  */
    lval *val = lval_pop(a, 0);

    /*     bind a copy into the function's environment */
    lenv_put(f->env, sym, val);

    /*     delete symbol and value */
    lval_del(sym);
    lval_del(val);
  }

  lval_del(a);

  /*   if '&' remains in formal list bind to empty list */
  if (f->formals->count > 0 && strcmp(f->formals->cell[0]->sym, "&") == 0) {

    /*     check to ensure that & is not passed invalidly.  */
    if (f->formals->count != 2) {
      return lval_err("Function format invalid. "
                      "Symbol '&' not followed by single symbol.");
    }

    /*     pop and delete '&' symbol */
    lval_del(lval_pop(f->formals, 0));

    /*     pop next symbol and create empty list  */
    lval *sym = lval_pop(f->formals, 0);
    lval *val = lval_qexpr();

    /*     bind to environment and delete */
    lenv_put(f->env, sym, val);
    lval_del(sym);
    lval_del(val);
  }

  /*   if all formula have been bound evaluate */
  if (f->formals->count == 0) {
    /*     set environment parent to evaluation environment  */
    f->env->par = e;

    /*     evaluate and return  */
    return builtin_eval(f->env, lval_add(lval_sexpr(), lval_copy(f->body)));
  } else {
    /*     otherwise return partially evaluated function */
    return lval_copy(f);
  }

  return builtin_eval(f->env, lval_add(lval_sexpr(), lval_copy(f->body)));
}

int lval_eq(lval *x, lval *y) {
  /* different types are always unequal */
  if (x->type != y->type) {
    return 0;
  }

  /* compare based upon type */
  switch (x->type) {
  case LVAL_NUM:
    return (x->num == y->num);
  case LVAL_ERR:
    return (strcmp(x->err, y->err) == 0);
  case LVAL_SYM:
    return (strcmp(x->sym, y->sym) == 0);
  case LVAL_STR:
    return (strcmp(x->str, y->str) == 0);
  case LVAL_FUN:
    if (x->builtin || y->builtin) {
      return x->builtin == y->builtin;
    } else {
      return lval_eq(x->formals, y->formals) && lval_eq(x->body, y->body);
    }
  case LVAL_QEXPR:
  case LVAL_SEXPR:
    if (x->count != y->count) {
      return 0;
    }
    for (int i = 0; i < x->count; i++) {
      if (!lval_eq(x->cell[i], y->cell[i])) {
        return 0;
      }
    }
    return 1;
    break;
  }
  return 0;
}

lval *builtin(lenv *e, lval *a, char *func) {
  if (strcmp("list", func) == 0) {
    return builtin_list(e, a);
  }
  if (strcmp("head", func) == 0) {
    return builtin_head(e, a);
  }
  if (strcmp("tail", func) == 0) {
    return builtin_tail(e, a);
  }
  if (strcmp("join", func) == 0) {
    return builtin_join(e, a);
  }
  if (strcmp("eval", func) == 0) {
    return builtin_eval(e, a);
  }
  if (strstr("+-/*", func)) {
    return builtin_op(e, a, func);
  }
  lval_del(a);
  return lval_err("Unknown Function!");
}

lval *builtin_head(lenv *e, lval *a) {
  /* check error conditions */
  LASSERT(a, a->count == 1,
          "Function 'head' passed too many arguments! "
          "got %i, expected %i",
          a->count, 1);
  LASSERT_TYPE(a, "head", 0, LVAL_QEXPR);
  LASSERT(a, a->cell[0]->count != 0, "Function 'head' passed {}!");

  /* otherwise take first argument  */
  lval *v = lval_take(e, a, 0);

  /* delete all elements that are not head and return  */
  while (v->count > 1) {
    lval_del(lval_pop(v, 1));
  }
  return v;
}

lval *builtin_tail(lenv *e, lval *a) {
  /* check error conditions */
  LASSERT(a, a->count == 1, "Function 'tail' passed too many arguments!");
  LASSERT_TYPE(a, "tail", 0, LVAL_QEXPR);
  LASSERT(a, a->cell[0]->count != 0, "Function 'tail' passed {}!");

  /* take first element */
  lval *v = lval_take(e, a, 0);

  lval_del(lval_pop(v, 0));
  return v;
}

lval *builtin_list(lenv *e, lval *a) {
  a->type = LVAL_QEXPR;
  return a;
}

lval *builtin_eval(lenv *e, lval *a) {
  LASSERT(a, a->count == 1, "Function 'eval' passed too many arguments!");
  LASSERT_TYPE(a, "eval", 0, LVAL_QEXPR);

  lval *x = lval_take(e, a, 0);
  x->type = LVAL_SEXPR;
  return lval_eval(e, x);
}

lval *builtin_join(lenv *e, lval *a) {
  for (int i = 0; i < a->count; i++) {
    LASSERT_TYPE(a, "join", 0, LVAL_QEXPR);
  }

  lval *x = lval_pop(a, 0);

  while (a->count) {
    lval_join(e, x, lval_pop(a, 0));
  }

  lval_del(a);
  return x;
}

lval *lval_join(lenv *e, lval *x, lval *y) {
  /* for each cell in 'y' add it to 'x' */
  while (y->count) {
    x = lval_add(x, lval_pop(y, 0));
  }

  lval_del(y);
  return x;
}

int number_of_nodes(mpc_ast_t *t) {
  if (t->children_num == 0) {
    return 1;
  }
  if (t->children_num >= 1) {
    int total = 1;
    for (int i = 0; i < t->children_num; i++) {
      total = total + number_of_nodes(t->children[i]);
    }
    return total;
  }
  return 0;
}
