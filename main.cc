#include <iostream>
#include "globals.h"

void printTree(TreeNode*);

int main(void) {

	TokenType token = getToken();

	while(token.TokenClass != ENDFILE) {

		cout << token.TokenString << " : " << token.TokenClass << endl;

		//start getting the next token
		token = getToken();
	}
  
  return 0;
}
