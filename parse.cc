#include <stdlib.h>
#include <iostream>
#include "globals.h"

/* Syntax Analyzer for the calculator language */

static TokenType token; /* holds current token */

/* Function newExpNode creates a new expression 
 * node for syntax tree construction
 */
TreeNode * newNode(TokenType tType) {
  TreeNode * t = (TreeNode *) malloc(sizeof(TreeNode));
  int i;
  if (t==NULL) {
    fail("Out of memory error at line %d\n");
  }
  else {
    for (i=0;i < MAXCHILDREN;i++) t->child[i] = NULL;
    t->op = tType.TokenClass;
  }
  return t;
}

//consume the terminal, match current, guide pointer to next
//so the token is forever next token
static void advance(int expected) {
  if (token.TokenClass == expected) token = getToken();
 else {
    cerr << "unexpected token -> " << token.TokenString << endl; 
  }
}

TreeNode * exp(void);

TreeNode * factor(void) {
  TreeNode * t = NULL;
  switch (token.TokenClass) {
  case NUM :
    t = newNode(token);
    if ((t!=NULL) && (token.TokenClass==NUM))
      t->val = atoi(token.TokenString.c_str());
    advance(NUM);
    break;
  case ID :
    t = newNode(token);
    if ((t!=NULL) && (token.TokenClass==ID))
      t->id = token.TokenString[0];
    advance(ID);
    break;
  case LPAREN :
    advance(LPAREN);
    t = exp();
    advance(RPAREN);
    break;
  default: 
    cerr << "unexpected token -> " << token.TokenString << endl;
    exit(1);
    break;
  }
  return t;
}

TreeNode * exp_prime(TreeNode * left) {
  if ((token.TokenClass==PLUS) 
      || (token.TokenClass==SUB)
      || (token.TokenClass==TIMES)
      || (token.TokenClass==DIV)) {
    TreeNode *p = newNode(token);
    
    advance(token.TokenClass);
    if (p!=NULL) {
      p->child[0] = left;
      p->child[1] = factor();
      return exp_prime(p);
    }
    else return NULL;
  }
  else return left;
}

TreeNode * exp(void) { 
  TreeNode * t = factor();
  return exp_prime(t);
}

/*==========  My code  ==========*/


TreeNode * declaration(void) { 
  TreeNode *t = NULL;
  string type, id;

  type = type();
  id = token.TokenString;
  advance(ID); //consume ID, move to the next

  t = generic_declaration();

  //assemble the node
  t->id = id;
  t->type = type;

  return t;
}

TreeNode * generic_declaration(void) {
  TreeNode * t = NULL;
  int size_info;

  //the current token is always the next one

  if (token.TokenClass == SEMI || token.TokenClass == LSBRACKT) {
    t = newNode(token);
    size_info = var_declaration();
    t->kind = "Variable declaration";
    if (size_info >= 0) {
      t->val = size_info; //array's t->val holds size value
    }
  }
  else if (token.TokenClass == LPAREN) {
    t = fun_declaration();
    t->kind = "Function declaration";
  }
  else {
    fail("unexpected token -> " + token.TokenString);
  }

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

//Done right? I hope.
TreeNode * fun_declaration(void) { 

  TreeNode *p = newNode(token);

  advance(LPAREN);
  p->child[0] = formals();
  advance(RPAREN);
  p->child[1] = block();

  return p;
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
  else if (token.TokenString == INT){
    t = parameters();
    //not consuming anything because non-terminal
  }
  else {
    fail("unexpected type value: " + token.TokenString);
  }

  return t;
}

//be careful! Might be broken LOL
//the linked list part looks sketchy
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

  string type_info = non_void_type(); //already moved to ID

  if (token.TokenClass != ID) {
    fail("unexpected token, after parameter type should be ID.");
  }

  string id = token.TokenString;
  t = newNode(token);

  advance(ID); //consume ID; move to entity

  //check for if empty rule applies

  if (token.TokenString == LSBRACKT) {
    advance(LSBRACKT);
    advance(RSBRACKT);
    t->type = "Array[" + type_info + "]";
    t->kind = "Parameter";
  }

  if (token.TokenString == COMMA || token.TokenString == RPAREN) {
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
TreeNode * block(void) {
  TreeNode *t = NULL;

  advance(LBRACKT);
  t->child[0] = local_vars();
  t->child[1] = statements();
  advance(RBRACKT);

  t->kind = "Compound Statement";
  return t;
}

TreeNode * local_vars(void) {

}

TreeNode * statements(void) {

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
    
    //not gonna use
    if (token.TokenClass==OUT) {
      advance(token.TokenClass);
      p->child[0] = exp();
    }
    else if (token.TokenClass==ID) { /* assignment */
      p->op = ASSIGN;
      p->id = token.TokenString[0];
      
      advance(token.TokenClass);
      if (token.TokenClass != ASSIGN) {
        cerr << "Invalid assignment statement " 
             << p->id << " " << token.TokenString << endl;
        exit(1);
      }
      advance(ASSIGN);
      p->child[0] = exp();
    }
    else {
      cerr << "non-lvalue assignment\n";
      exit(1);
    }
    //below, write my own code



    
     /* more statements */
    if (token.TokenClass == SEMI) {
      token = getToken();
    }
    else if (token.TokenClass != ENDFILE) {
      cerr << "unexpected end of statement: " << token.TokenString << "\n";
      exit(1);
    }
  }

  return h;
}