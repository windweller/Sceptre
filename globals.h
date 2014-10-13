#ifndef _GLOBALS_H
#define _GLOBALS_H

#define ENDFILE 256
#define ERROR   257
#define NUM     258
#define PLUS    259
#define TIMES   260
#define LPAREN  261 // (
#define RPAREN  262 // )
#define SUB     263
#define DIV     264
#define ID      265
#define ASSIGN  266 // =
#define OUT     267
#define SEMI    268
#define COMMA   297 // ,

//Reserved Words
#define INT     269
#define VOID    270
#define LBRACKT 271 //  {
#define RBRACKT 272 //  }
#define LSBRACKT 273 // [
#define RSBRACKT 274 // ]
#define IF      275
#define ELSE    276
#define WHILE   277
#define RETURN  288
#define BREAK   296
#define EXCL    289 // !
#define SEQUAL  290 // <=
#define SMALL   291 // <
#define LARGE   292 // >
#define LEQUAL  293 // >=
#define EEQUAL  294 // ==
#define UEQUAL  295 // !=

using namespace std;

#include <cstdlib>
#include <iostream> //added
#include <list>

typedef struct {
    int TokenClass;  /* one of the above */
    string TokenString;
} TokenType;

#define MAXCHILDREN 3 //should I change it?

typedef struct treeNode {
    struct treeNode * child[MAXCHILDREN];
    int op;
    int val; //for array, this stores size
    string id;
    string kind; //declaration (the left-hand side of production rule) (not always)
    string type; //array would be array[int]
} TreeNode;

#define ST_SIZE 26

#define CODESIZE 100

typedef enum {RO,RM} OpCodeType;

typedef struct {
    string opcode;
    OpCodeType ctype;
    int rand1;
    int rand2;
    int rand3;
} CodeType;

/* function getToken returns the
 * next token in source file
 */
TokenType getToken(void);
TreeNode * declarations(void);
void analyze(TreeNode*);
void codeGenStmt(TreeNode*);
void emit(string,OpCodeType,int,int,int);
void printCode(void);
void fail(string);
void printAST(TreeNode*);

#endif