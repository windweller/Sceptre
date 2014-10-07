#include <iostream>
#include <map>
#include <stdio.h>
#include <ctype.h>
#include "globals.h"

/* Lexical Analyzer for the calculator language */

/* states in the scanner DFA */
typedef enum { 
  START,INNUM, INDIG, INLARGE, INSMALL, INEQ, INEXCL, DONE
} StateType;


bool insideComment = false;

bool LayOutCharacter(char c, StateType state) {

  bool skipOrNot = false;
  char d;

  if (state == INDIG)
  {
    return false; //no skipping
  }

  if ((c == ' ') || (c == '\t') || (c == '\n')) skipOrNot = true;
  
  /* determine if entered a comment block */
  /* This is a state machine by itself */
  if (c == '/') {
    d = cin.get(); //s0
    if (d == '*') {
      insideComment = true;

      while(insideComment) {
        char s2Char = cin.get();
        if (s2Char == '*') {
          char s3Char = cin.get();
          if (s3Char == '/') {
            insideComment = false;
            skipOrNot = true; //skip current token
          }
        }
      }
    }else{
      //if not a *, we put it back, because it's a division
      //Or it might be EOF, should I handle EOF? It's still legal.
      cin.putback(d);
      return false;
    }
  }

  /* Whenever encountering EOF, we exit */
  if (c == EOF) {
    skipOrNot = false;
  }

  return skipOrNot;
}


/* This version uses not a DFA, but a reserved word list */
/* It tries to find the longest match possible */

TokenType getToken(void) {  
   /* holds current token to be returned */
   TokenType currentToken;
   /* current state - always begins at START */
   StateType state = START;
   /* next character */
   char c;
   /* holds all reserved key words */
   /* the key is the string, value is the defined int */
   map<string, int> reservedWords;

   reservedWords["int"] = INT;
   reservedWords["void"] = VOID;
   reservedWords["if"] = IF;
   reservedWords["else"] = ELSE;
   reservedWords["while"] = WHILE;
   reservedWords["return"] = RETURN;
   reservedWords["break"] = BREAK;

   /* If in a INDIG state, we preserve white space, end of line and tab */
   do {
     c = cin.get();
   } while (LayOutCharacter(c, state));

   while (state != DONE) {
     switch (state) {
     case START:
        /* If it is a digit, and not in the state of INDIG */
        /* We collect them as NUM */
        if (isdigit(c))
         state = INNUM;
        else if ('a' <= c && c <= 'z') {
          //start of keyword or ID should be letter
          state = INDIG; //enter state of INDIG
        }

       else {

    //state to DONE because we will handle all cases of 
    //operators using enough lookahead (CAREFUL!!!)
    //add them!!!

   state = DONE;
   switch (c)
           {
             case EOF: 
               currentToken.TokenClass = ENDFILE;
               break;
             case '+':
               currentToken.TokenClass = PLUS;
               currentToken.TokenString += c;
               break;
             case '-':
               currentToken.TokenClass = SUB;
               currentToken.TokenString += c;
               break;
             case '*':
               currentToken.TokenClass = TIMES;
               currentToken.TokenString += c;
               break;
             case '/':
               currentToken.TokenClass = DIV;
               currentToken.TokenString += c;
               break;
             case '(':
               currentToken.TokenClass = LPAREN;
               currentToken.TokenString += c;
               break;
             case ')':
               currentToken.TokenClass = RPAREN;
               currentToken.TokenString += c;
               break;
             case '{':
               currentToken.TokenClass = LBRACKT;
               currentToken.TokenString += c;
               break;
             case '}':
               currentToken.TokenClass = RBRACKT;
               currentToken.TokenString += c;
               break;
             case '[':
               currentToken.TokenClass = LSBRACKT;
               currentToken.TokenString += c;
               break;
             case ']':
               currentToken.TokenClass = RSBRACKT;
               currentToken.TokenString += c;
               break;
             case '!': 
               //this encompasses ! and !=
                state = INEXCL;
                currentToken.TokenClass = EXCL;
               break; 
             case '=': 
                state = INEQ;
                //well, you can assign it in INLARGE,
                //but why not here?
                currentToken.TokenClass = ASSIGN;
               break; 
             case '<': 
                state = INSMALL;
                currentToken.TokenClass = SMALL;                                   
               break; 
             case '>': 
                state = INLARGE;
                currentToken.TokenClass = LARGE;
               break;                     
             case '$':
               currentToken.TokenClass = OUT;
               currentToken.TokenString += c;
               break;
             case ';':
               currentToken.TokenClass = SEMI;
               currentToken.TokenString += c;
               break;
             default:
               currentToken.TokenClass = ERROR;
               break;
           }
         }
         break;
       case INLARGE:

         state = DONE;

          if (c == '=') {
            currentToken.TokenString += c;
            currentToken.TokenClass = LEQUAL;
          }else{
            cin.putback(c);
          }
        break;

       case INEQ:
         state = DONE;

          if (c == '=') {
            currentToken.TokenString += c;
            currentToken.TokenClass = EEQUAL;
          }else{
            cin.putback(c);
          }
        break;        

       case INSMALL:
         state = DONE;
          if (c == '=') {
            currentToken.TokenString += c;
            currentToken.TokenClass = SEQUAL;
          }else{
            cin.putback(c);
          }
        break;  

       case INEXCL:
          state = DONE;
          if (c == '=') {
            currentToken.TokenString += c;
            currentToken.TokenClass = UEQUAL;
          }else{
            cin.putback(c);
          }
        break; 

       case INDIG: //meaning it's an ID or keyword

          if (!isdigit(c) && (c < 'A' || c > 'z')) {
            state = DONE;
            cin.putback(c);
            currentToken.TokenClass = ID; //treat as ID first
          }

       break;
       case INNUM: //meaning it's number
         if (!isdigit(c))
         { 
         /* backup in the input */
           cin.putback(c);
           state = DONE;
           currentToken.TokenClass = NUM;
         }
         break;
       case DONE:
       default: /* should never happen */
         state = DONE;
         currentToken.TokenClass = ERROR;
         break;
     }
     
     /* This collects INNUM and INDIGIT characters */
     if (state != DONE) {
      currentToken.TokenString += c;
      c = cin.get();
     }
   }

   if (currentToken.TokenClass == ID) {
       map<string, int>::const_iterator it;
       it = reservedWords.find(currentToken.TokenString);
       if (it != reservedWords.end()) {
              //retrieve token class
          currentToken.TokenClass = it->second;
       }else{
          currentToken.TokenClass = ID;
       }
   }

   return currentToken;
} /* end getToken */

