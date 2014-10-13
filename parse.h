#ifndef _parse_h
#define _parse_h

#include <stdio.h>

TreeNode * declaration(void);
TreeNode * generic_declaration(void);
TreeNode * fun_declaration(TreeNode*);
int var_declaration(void);
TreeNode * local_var_declaration(void);
TreeNode * formals(void);
TreeNode * parameters(void);
TreeNode * parameter(void);
TreeNode * block(void);
TreeNode * local_vars(void);
TreeNode * statements(void);
TreeNode * statement(void);
TreeNode * other_statement(void);
TreeNode * expr_stmt(void);
TreeNode * selection(void);
TreeNode * else_part(void);
TreeNode * comparison(void);
TreeNode * relop(void);
TreeNode * repetition(void);
TreeNode * return_stmt(void);
TreeNode * return_with_expr(void);
TreeNode * expression(void);
TreeNode * id_expression();
TreeNode * assignment_or_arith(void);
TreeNode * arith_expr(void);
TreeNode * arith_expr_prime(void);
TreeNode * arith_term(void);
TreeNode * arith_term_prime(void);
TreeNode * arith_factor(void);
TreeNode * var_or_call(void);
TreeNode * var(void);
TreeNode * var_add(void);
TreeNode * call(void);
TreeNode * call_add(void);
TreeNode * actuals(void);
TreeNode * arguments(void);
TreeNode * argument(void);
string type(void);
string non_void_type(void);
TreeNode * declarations(void);

#endif