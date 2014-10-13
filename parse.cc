#include <stdlib.h>
#include <iostream>
#include "globals.h"
#include "parse.h"

/* Syntax Analyzer for the calculator language */

static TokenType token; /* holds current token */

/* Function newExpNode creates a new expression
 * node for syntax tree construction
 */
TreeNode * newNode(TokenType tType) {
    TreeNode* t = new TreeNode;
    int i;
    if (t==NULL) {
        fail("Out of memory error at line %d\n");
    }
    else {
        for (i=0;i < MAXCHILDREN;i++) t->child[i] = NULL;
        t->op = tType.TokenClass;
        t->id = tType.TokenString; //ah! turns out better leave this, we should manually add id
    }
    return t;
}

//consume the terminal, match current, guide pointer to next
//so the token is forever next token
static void advance(int expected) {
    if (token.TokenClass == expected) token = getToken();
    else {
        cerr << "advance unexpected token -> " << token.TokenString << endl;
    }
}

TreeNode * declaration(void) {
    TreeNode *t = NULL;
    
    //defer ID consumption to lower level
    t = generic_declaration();
    
    return t;
}

TreeNode * generic_declaration(void) {
    TreeNode * t = NULL;
    int size_info;
    
    string id_type = type();
    t = newNode(token); //ID node
    advance(ID);
    
    //the current token is always the next one
    
    if (token.TokenClass == SEMI || token.TokenClass == LSBRACKT) {
        //variable
        size_info = var_declaration();
        t->kind = "Variable declaration";
        if (size_info >= 0) {
            t->val = size_info; //array's t->val holds size value
            t->type = "Array[" + id_type + "]";
        }
        else {
            t->type = id_type;
        }
    }
    else if (token.TokenClass == LPAREN) {
        //function
        fun_declaration(t); //by doing this, we already changed t's value, because we are passing in a pointer/reference
        t->type = "return type: " + id_type;
    }
    else {
        fail("generic_declaration unexpected token -> " + token.TokenString);
    }
    
    return t;
}

//Done right? I hope.
TreeNode * fun_declaration(TreeNode * t) {
    
    advance(LPAREN);
    t->child[0] = formals();
    advance(RPAREN);
    t->child[1] = block();
    
    t->kind = "Function declaration";
    
    return t;
}

//if a normal var, return -1
//if an array, return the size of array
int var_declaration(void) {
    
    int size_info;
    
    if (token.TokenClass == SEMI) {
        size_info = -1;
        advance(SEMI); //consume the terminal, match current, guide pointer to next
    }
    else { //matching array int[size]
        advance(LSBRACKT);
        if (token.TokenClass == NUM) {
            size_info = atoi(token.TokenString.c_str());
            advance(NUM);
            advance(RSBRACKT);
            advance(SEMI);
        }
        else{
            fail("array declaration size is not a number -> " + token.TokenString);
        }
    }
    
    return size_info;
}

//basically functions like an alias of generic_declaration()
//Kinda done; I hope
TreeNode * local_var_declaration(void) {
    
    TreeNode * t = NULL;
    
    string type_info = non_void_type();
    t = newNode(token);
    t->id = token.TokenString;
    advance(ID);
    int size_info = var_declaration();
    t->kind = "Variable declaration";
    
    if (size_info >= 0) {
        t->val = size_info; //array's t->val holds size value
        t->type = "Array[" + type_info + "]";
    }
    else {
        t->type = type_info;
    }
    
    return t;
}

//remember when assembling this, check if the kind is null
//or tokenstring is "void"
//DONE
TreeNode * formals(void) {
    TreeNode * t = NULL;
    
    if (token.TokenClass == VOID) {
        t = newNode(token); //token.TokenString is already void
        advance(VOID); //consume token
    }
    else if (token.TokenClass == INT){
        t = parameters();
        //not consuming anything because non-terminal
    }
    else {
        fail("unexpected type value: " + token.TokenString);
    }
    
    return t;
}

TreeNode * parameters(void) {
    
    TreeNode *h = NULL, *t = NULL;
    //build a while loop
    //current token is INT
    //generate a new treeNode p for every new param
    while(token.TokenClass != RPAREN) {
        TreeNode * p = newNode(token);
        
        /* create a linked list of parameter declarations */
        if (h == NULL)  /* initially empty */
        h = t = p;
        else { /* generally non-empty */
            t->child[1] = p;
            t = p;
        }
        
        //if we have more types, add it here
        if (token.TokenClass == INT) {
            p->child[0] = parameter();
        }
        
        if (token.TokenClass == COMMA) {
            advance(COMMA);
        }
    }
    
    return h;
}

TreeNode * parameter(void) {
    TreeNode *t = NULL;
    
    string type_info = non_void_type(); //already advanced to ID
    
    string id = token.TokenString;
    t = newNode(token);
    
    advance(ID); //consume ID; move to entity
    
    //check for if empty rule applies
    
    if (token.TokenClass == LSBRACKT) {
        advance(LSBRACKT);
        advance(RSBRACKT);
        t->type = "Array[" + type_info + "]";
        t->kind = "Parameter";
    }
    else if (token.TokenClass == COMMA || token.TokenClass == RPAREN) {
        //empty rule applies, return the tree
        //or we simply reached the end, return the tree!
        t->type = type_info;
        t->kind = "Parameter";
    }
    else {
        fail("illegal expression: " + token.TokenString);
    }
    
    return t;
}


//DONE!
//NOTE: returned tree can have either 1 or 2 children!!!!
TreeNode * block(void) {
    TreeNode *t = newNode(token);
    
    advance(LBRACKT);
    
    t->child[0] = local_vars();
    
    //local_vars could return NULL, so we
    //replace it if it's NULL. I don't want to deal with NULL error!
    if (t->child[0] == NULL)
    {
        t->child[0] = statements();
    }
    else{
        t->child[1] = statements();
    }
    
    advance(RBRACKT);
    
    t->kind = "Compound Statement";
    return t;
}

//kinda done!
TreeNode * local_vars(void) {
    
    TreeNode *t = NULL;
    
    if (token.TokenClass == INT) {
        t = local_var_declaration();
    }
    else if((token.TokenClass == BREAK) //BREAK
            || (token.TokenClass == OUT) //$
            || (token.TokenClass == SEMI) //;
            || (token.TokenClass == ASSIGN) //=
            || (token.TokenClass == ID) // ID
            || (token.TokenClass == NUM) // NUM
            || (token.TokenClass == LPAREN) // (
            || (token.TokenClass == IF) // IF
            || (token.TokenClass == RETURN) // RETURN
            || (token.TokenClass == LBRACKT) // {
            || (token.TokenClass == WHILE)) {
        return NULL; //empty match
    }
    else {
        fail("unexpected token for local var declaration: " + token.TokenString);
    }
    
    //!!chance is h can be NULL because local_vars can go ε
    return t;
}

//Maybe done? not sure, if error, check this place twice!
TreeNode * statements(void) {
    
    TreeNode *h = NULL, *t = NULL;
    
    //if not } , we keep looking
    while(token.TokenClass != RBRACKT) {
        TreeNode * p = newNode(token);
        
        //this absolutely works just fine
        //the child is child[1]
        /* create a linked list of parameter declarations */
        if (h == NULL)  /* initially empty */
        h = t = p;
        else { /* generally non-empty */
            t->child[1] = p;
            t = p;
        }
        
        if ((token.TokenClass == BREAK) //BREAK
            || (token.TokenClass == OUT) //$
            || (token.TokenClass == SEMI) //;
            || (token.TokenClass == ID) // ID
            || (token.TokenClass == NUM) // NUM
            || (token.TokenClass == LPAREN) // (
            || (token.TokenClass == IF) // IF
            || (token.TokenClass == RETURN) // RETURN
            || (token.TokenClass == LBRACKT) // {
            || (token.TokenClass == WHILE)) {
            
            p->child[0] = statement();
        }
        
        //do we need to advance here???
        //probably not. Let's have some faith, limited may it be
        
    }
    
    //could be empty
    return h;
}

//Don't have to attach anything to kind or type :)
TreeNode * statement(void) {
    TreeNode *t = NULL;
    
    if (token.TokenClass == IF)
    {
        t = selection();
    }
    else if ((token.TokenClass == BREAK) //BREAK
             || (token.TokenClass == OUT) //$
             || (token.TokenClass == SEMI) //;
             || (token.TokenClass == ASSIGN) //=
             || (token.TokenClass == ID) // ID
             || (token.TokenClass == NUM) // NUM
             || (token.TokenClass == LPAREN) // (
             || (token.TokenClass == RETURN) // RETURN
             || (token.TokenClass == LBRACKT) // {
             || (token.TokenClass == WHILE)) {
        
        t = other_statement();
    }
    else {
        fail("illegal statement: " + token.TokenString);
    }
    
    return t;
}

TreeNode * other_statement(void) {
    TreeNode *t = NULL;
    
    switch (token.TokenClass) {
        case SEMI : {
        t = expr_stmt();
        break;
        }
        case ID : {
        t = expr_stmt();
        break;
        }
        case NUM : {
        t = expr_stmt();
        break;
        }
        case LPAREN : {
        t = expr_stmt();
        break;
        }
        case LBRACKT : {
        t = block(); //already moved beyond }
        break;
        }
        case BREAK : {// break ;
        t = newNode(token);
        advance(BREAK);
        advance(SEMI);
        break;
        }
        case WHILE : {
        t = repetition();
        break;
        }
        case RETURN : {
        t = return_stmt();
        break;
        }
        case OUT : {
        advance(OUT);
        t = expression();
        advance(SEMI);
        t->op = OUT;
        t->kind = "Out";
        break;
        }
        default : {
        fail("illegal statement: " + token.TokenString);
        break;
        }
    }
    
    return t;
}

//Could return NULL; Don't know what to return if the non-terminal is SEMI
TreeNode * expr_stmt(void) {
    TreeNode *t = NULL;
    
    if ((token.TokenClass == ID) // ID
        || (token.TokenClass == NUM) // NUM
        || (token.TokenClass == LPAREN)) // (
    {
        t = expression();
        advance(SEMI);
    }
    else if (token.TokenClass == SEMI)
    {
        advance(SEMI);
    }
    else {
        fail("illegal token: " + token.TokenString);
    }
    
    //could be NULL
    return t;
}

//Maybe done by now? Hopefully!
TreeNode * selection(void) {
    TreeNode *p = newNode(token);
    
    advance(IF);
    advance(LPAREN);
    
    p->child[0] = comparison();
    advance(RPAREN);
    p->child[1] = statement();
    p->child[2] = else_part(); //so else is always with closest if
    
    return p;
}

//could return NULL
TreeNode * else_part(void) {
    TreeNode *p = NULL;
    
    //as long as there's an else, we match for it
    if (token.TokenClass == ELSE)
    {
        p = newNode(token); //should I? make else an independent node?
        advance(ELSE);
        p = statement(); //ELSE node
        p->kind = "Else";
        p->id = "else";
    }
    
    return p;
}

//Currently not in use
// TreeNode * with_else(void) {
//   TreeNode *p = NULL;

//   if (token.TokenClass = IF)
//   {
//     p = newNode(token);
//     advance(IF);
//     advance(LPAREN);

//     p->child[0] = comparison();
//     advance(RPAREN);

//   }

//   return p;
// }

//should be done!
//be careful of assigning value to NULL
TreeNode * comparison(void) {
    TreeNode *p = newNode(token); // this will be "(" as node
    
    p->kind = "Comparison";
    
    if ((token.TokenClass == ID) || (token.TokenClass == NUM)
        || (token.TokenClass == LPAREN))
    {
        p->child[0] = arith_expr();
        p->child[1] = relop(); //assign attach op to p node
        p->child[2] = arith_expr();
    }
    else if (token.TokenClass == EXCL)
    {
        advance(EXCL);
        advance(LPAREN);
        p->op = EXCL;
        p->child[0] = comparison();
        advance(RPAREN);
    }
    
    return p;
}

//DONE!
TreeNode * relop(void) {
    TreeNode *t = NULL;
    int symbol;
    
    switch(token.TokenClass) {
        case SEQUAL :
        symbol = SEQUAL;
        break;
        case SMALL :
        symbol = SMALL;
        break;
        case LARGE :
        symbol = LARGE;
        break;
        case LEQUAL :
        symbol = LEQUAL;
        break;
        case EEQUAL :
        symbol = EEQUAL;
        break;
        case UEQUAL :
        symbol = UEQUAL;
        break;
        default:
        fail("unexpected relop symbol: " + token.TokenString);
        break;
    }
    
    t = newNode(token); //TokenString is converted to id //and op is a node!
    advance(token.TokenClass);
    
    return t;
}

//what to return? eh?
TreeNode * repetition(void) {
    TreeNode *p = newNode(token); //this is supposed to be WHILE
    p->kind = "While loop";
    
    advance(WHILE);
    advance(LPAREN);
    p->child[0] = comparison();
    advance(RPAREN);
    p->child[1] = statement();
    
    return p;
}

TreeNode * return_stmt(void) {
    TreeNode *t = newNode(token); //return as node
    advance(RETURN);
    t->child[0] = return_with_expr(); //might return null
    t->kind = "Return";
    
    return t;
}

TreeNode * return_with_expr(void) {
    TreeNode *t = NULL; // for expression or nothing?
    
    if ((token.TokenClass == ID) || (token.TokenClass == NUM)
        || (token.TokenClass == LPAREN))
    {
        t = expression(); //token taken from expression
        //rembmer? try to be flat!
    }
    else if (token.TokenClass == SEMI) {
        advance(SEMI);
        
        return NULL; //empty return
    }
    else {
        fail("unexpected token at return clause : " + token.TokenString);
    }
    
    return t;
}

//Done!
TreeNode * expression(void) {
    TreeNode *p = NULL;
    //There is no expression node, but ID or NUM or LPAREN, ID node is being passed up
    
    if (token.TokenClass == ID) {
        //we are supposed to consume ID here, but we defer
        //to ID-expression; it will receive a ID node
        p = id_expression();
    }
    else if (token.TokenClass == NUM) {
        p = newNode(token); //NUM is a node
        p->kind = "Const";
        advance(NUM);
        p->child[0] = arith_term_prime();
        p->child[1] = arith_expr_prime();
    }
    else if (token.TokenClass == LPAREN) {
        p = newNode(token);//LPAREN must be a node or otherwise you don't have enough children
        p->child[0] = expression();
        p->child[1] = arith_term_prime();
        p->child[2] = arith_expr_prime();
    }
    else {
        //if no match at all, probably error
        fail("illegal token in expression: " + token.TokenString);
    }
    
    return p;
}

//DONE!
TreeNode * id_expression(void) {
    TreeNode *t = newNode(token); //make ID a node and pass up!
    
    string id = token.TokenString;
    advance(ID);
    
    //then we decide between var-add or call-add
    if (token.TokenClass == LPAREN) {
        t->child[0] = call_add(); //t's child becomes the actuals node, because it could be empty
        t->id = id;
        t->kind = "Function call";
    }
    else if ((token.TokenClass == ASSIGN) // =
             || (token.TokenClass == TIMES) // *
             || (token.TokenClass == DIV) // DIV(/)
             || (token.TokenClass == PLUS) // +
             || (token.TokenClass == SUB)) { // -
        //var_add() will decide if it's empty match
        t->child[0] = var_add(); //could be empty the [expression] part
        t->kind = "Id";
        t->child[1] = assignment_or_arith();
    }
    else if(token.TokenClass == SEMI || token.TokenClass == RPAREN) {
        //match empty string
        //might be erroneous! Careful!!
        return t;
    }
    else {
        fail("id_expression unexpected token: " + token.TokenString);
    }
    
    return t;
}

//checked, valid!
TreeNode * assignment_or_arith(void) {
    TreeNode *t = NULL;
    
    if (token.TokenClass == ASSIGN) {
        //Assignment
        t = newNode(token); //ASSIGN "=" as node
        t->kind = "Assign to";
        advance(ASSIGN);
        t->child[0] = expression();
    }
    else if ((token.TokenClass == TIMES) // *
             || (token.TokenClass == DIV) // DIV(/)
             || (token.TokenClass == PLUS) // +
             || (token.TokenClass == SUB)) { // -
        //arith
        t = arith_term_prime();
        if (t == NULL) {
            t = arith_expr_prime();
        }
        else {
            t->child[2] = arith_expr_prime(); //because arith-term-prime already carried two children
        }
        
    }
    
    return t;
}

//this also affects the calculation order
TreeNode * arith_expr(void) {
    TreeNode *p = NULL;
    
    if ((token.TokenClass == ID) || (token.TokenClass == NUM)
        || (token.TokenClass == LPAREN))
    {
        p = newNode(token);
        p->child[0] = arith_term();
        p->child[1] = arith_expr_prime();
    }
    else {
        fail("arith_expr: unexpected token: " + token.TokenString);
    }
    
    return p;
}

//might return NULL; basically done...
TreeNode * arith_expr_prime(void) {
    TreeNode *p = newNode(token);
    if (token.TokenClass == PLUS)
    {
        p->op = PLUS;
        p->kind = "Op";
        advance(token.TokenClass);
        p->child[0] = arith_term();
        p->child[1] = arith_expr_prime();
    }
    else if (token.TokenClass == SUB)
    {
        p->op = SUB;
        p->kind = "Op";
        advance(token.TokenClass);
        p->child[0] = arith_term();
        p->child[1] = arith_expr_prime();
    }
    else if ((token.TokenClass == SEQUAL) // <=
             || (token.TokenClass == SMALL) // <
             || (token.TokenClass == LARGE) // >
             || (token.TokenClass == LEQUAL) // >=
             || (token.TokenClass == EEQUAL) // ==
             || (token.TokenClass == UEQUAL) // !=
             || (token.TokenClass == RPAREN) // (
             || (token.TokenClass == SEMI)) {  // ;
        
        //empty match, match complete
        return NULL;
    }
    else {
        fail("illegal arithmatic expression: " + token.TokenString);
    }
    
    return p;
}

//Should be DONE!
TreeNode * arith_term(void) {
    
    TreeNode *p = NULL; //no wrap up, take factor as main token (or otherwise not minimum)
    
    if ((token.TokenClass == ID) || (token.TokenClass == NUM)
        || (token.TokenClass == LPAREN)) {
        p = arith_factor();
        p->child[0] = arith_term_prime();
    }
    else {
        fail("arith_term unexpected token: " + token.TokenString);
    }
    
    return p;
}

//SORTA DONE!
TreeNode * arith_term_prime(void) {
    TreeNode *p = newNode(token); // this node is actually an op
    
    if (token.TokenClass == TIMES)
    {
        p->op = TIMES;
        p->kind = "Op";
        advance(token.TokenClass);
        p->child[0] = arith_factor();
        p->child[1] = arith_term_prime();
    }
    else if (token.TokenClass == DIV)
    {
        p->op = DIV;
        p->kind = "Op";
        advance(token.TokenClass);
        p->child[0] = arith_factor();
        p->child[1] = arith_term_prime();
    }
    else if ((token.TokenClass == PLUS) //+
             || (token.TokenClass == SUB) // -
             || (token.TokenClass == SEQUAL) // <=
             || (token.TokenClass == SMALL) // <
             || (token.TokenClass == LARGE) // >
             || (token.TokenClass == LEQUAL) // >=
             || (token.TokenClass == EEQUAL) // ==
             || (token.TokenClass == UEQUAL) // !=
             || (token.TokenClass == RPAREN) // (
             || (token.TokenClass == SEMI)) {  // ;
        
        //empty match, match complete
        return NULL;
    }
    else{
        fail("arith_term_prime does not accept token: " + token.TokenString);
    }
    
    return p;
}

//I think it's done
TreeNode * arith_factor(void) {
    TreeNode *p = NULL;
    
    //this returns one node that has id, kind, and maybe child[] node for actuals
    //depth = 1
    if (token.TokenClass == ID)
    {
        //we are supposed to advance ID here, but we defer it to var_or_call
        //because that would form better node structure
        p = var_or_call();
    }
    else if (token.TokenClass == NUM)
    {
        p = newNode(token); //wrap NUM in a token
        p->kind = "Const";
        advance(NUM); //consume it
    }
    else if (token.TokenClass == LPAREN)
    {
        advance(LPAREN);
        p = expression(); //yup! it can be replaced by expreesion node
        advance(RPAREN);
    }
    else {
        //because we did a full match, anything not matching is an error!
        fail("illegal token: " + token.TokenString);
    }
    
    return p;
}

//var-or-call -> var-add | call-add
//return a super flat node
//will always return something
TreeNode * var_or_call(void) {
    
    TreeNode *t = newNode(token);
    
    string id = token.TokenString;
    advance(ID);
    
    //meaning it's call-add
    if (token.TokenClass == LPAREN) {
        t->child[0] = call_add(); //t's child becomes the actuals node, because it could be empty
        t->id = id;
        t->kind = "Function call";
    }
    else { //var_add() will decide if it's empty match
        t->child[0] = var_add(); //could be empty the [expression] part
        t->kind = "Id";
    }
    
    return t;
}

//I hope this is correct. We shall see.
TreeNode * var(void) {
    TreeNode *t = newNode(token);
    
    string id = token.TokenString;
    advance(ID);
    t->child[0] = var_add();
    t->kind = "Id";
    
    return t;
}

//return a node or NULL, get attached to ID node in var()
//add kind = "Id" here
TreeNode * var_add(void) {
    TreeNode *t = NULL;
    
    if (token.TokenClass == LSBRACKT) {
        advance(LSBRACKT);
        t = expression();
        advance(RSBRACKT);
    }
    else if ((token.TokenClass == PLUS) //+
             || (token.TokenClass == SUB) // -
             || (token.TokenClass == SEQUAL) // <=
             || (token.TokenClass == SMALL) // <
             || (token.TokenClass == LARGE) // >
             || (token.TokenClass == LEQUAL) // >=
             || (token.TokenClass == EEQUAL) // ==
             || (token.TokenClass == UEQUAL) // !=
             || (token.TokenClass == LPAREN) // )
             || (token.TokenClass == SEMI) //;
             || (token.TokenClass == DIV) // DIV (/)
             || (token.TokenClass == TIMES) //*
             || (token.TokenClass == ASSIGN) // =
             || (token.TokenClass == COMMA)) {   //, it's here yup!
        return NULL;
    }
    else { //if can't match anything
        fail("illegal token: " + token.TokenString);
    }
    
    return t;
}

//should work...let's hope
TreeNode * call(void) {
    TreeNode *t = newNode(token);
    string id = token.TokenString;
    advance(ID);
    
    //meaning it's call-add
    if (token.TokenClass == LPAREN) {
        t->child[0] = call_add(); //t's child becomes the actuals node, because it could be empty
        t->id = id;
        t->kind = "Function call";
    }
    else {
        fail("You must add parentheses for function calls");
    }
    return t;
}

//to return any additional arguments inside parentheses
TreeNode * call_add(void) {
    TreeNode *t = NULL; // ( is encapsulated as token here
    
    advance(LPAREN);
    t = actuals();
    advance(RPAREN);
    
    return t;
}

TreeNode * actuals(void) {
    TreeNode *t = NULL;
    
    if (token.TokenClass == RPAREN) {
        return NULL;
    }
    else if ((token.TokenClass == ID) || (token.TokenClass == NUM)
             || (token.TokenClass == LPAREN)) {
        //go to arguments while loop
        t = arguments();
    }
    
    return t;
}

//have a while loop
//should be done!
TreeNode * arguments(void) {
    TreeNode *h = NULL, *t = NULL;
    
    //if not ) , we keep looking
    while(token.TokenClass != RPAREN) {
        TreeNode * p = newNode(token);
        
        /* create a linked list of parameter declarations */
        if (h == NULL)  /* initially empty */
        h = t = p;
        else { /* generally non-empty */
            t->child[1] = p;
            t = p;
        }
        
        if ((token.TokenClass == ID) || (token.TokenClass == NUM)
            || (token.TokenClass == LPAREN)) {
            //go to arguments while loop
            p->child[0] = argument();
        }
        
        if (token.TokenClass == COMMA) {
            advance(COMMA);
        }
        
    }
    
    //could be empty
    return h;
    
}

//argument is almost like an alis to expression
//there's absolutely no point
TreeNode * argument(void) {
    TreeNode *t = NULL;
    t = expression();
    return t;
}


//returns the type string
string type(void) {
    
    string type;
    
    if (token.TokenClass == INT) {
        type = "int";
    }
    else if (token.TokenClass == VOID) {
        type = "void";
    }
    else {
        fail("unexpected type value: " + token.TokenString);
    }
    advance(token.TokenClass); //move on to the next
    return type;
}

string non_void_type(void) {
    string type;
    if (token.TokenClass == INT) {
        type = "int";
    }
    else{
        fail("unexpected type value: " + token.TokenString);
    }
    advance(token.TokenClass);
    return type;
}

/****************************************/
/* the primary function of the parser   */
/****************************************/
/* Function statements returns the newly 
 * constructed syntax tree
 */

//TODO: Maybe done??
TreeNode * declarations(void) { 
    TreeNode *h = NULL, *t = NULL;
    
    /* prime the token */
    token = getToken();
    
    while (token.TokenClass != ENDFILE) {
        TreeNode * p = newNode(token);
        
        /* create a linked list of statements */
        if (h == NULL)  /* initially empty */
        h = t = p;
        else { /* generally non-empty */
            t->child[1] = p;
            t = p;
        }
        
        p->child[0] = declaration();
        
        /* more declarations */
        // if (token.TokenClass == SEMI) {
        //   token = getToken();
        // }
        // else if (token.TokenClass != ENDFILE) {
        //   cerr << "unexpected end of statement: " << token.TokenString << "\n";
        //   exit(1);
        // }
    }
    
    return h;
}