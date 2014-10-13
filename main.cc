#include <iostream>
#include <fstream>
#include "globals.h"
#include "parse.h"

static int indentation;

int main(int argc, const char * argv[]) {
    
    indentation = 0;
    
    string file_address(*argv);
    
    cout << "type in the file address: ";
    cin >> file_address;
    ifstream file(file_address);
    
    streambuf *cinbuf = cin.rdbuf(); //save old buff
    cin.rdbuf(file.rdbuf());
    
    TreeNode* icode = declarations();
    if (icode == NULL) {
        fail("no top-level expression\n");
    }
    
    printAST(icode);
    
    cin.rdbuf(cinbuf); //put back the old buffer
    return 0;
}

/* Best be implemented recursively */
void printAST(TreeNode* ast) {
    
    //this is one level beneath the leaf node
  if (ast == NULL) {
      return;
  }
    
    //print out for this one node
    
  if (!ast->kind.empty()) {
    
    if (ast->id != "int" && ast->id != "void" && ast->id != "{" && ast->id != "return") {
      cout << ast->kind << ": ";
    }
    else{
      cout << ast->kind << ": " << endl;
    }
  }
    
  if (!ast->id.empty()) {
    //don't print out connection node for int and void (serving as declaration node)
    if (ast->id != "int" && ast->id != "void" && ast->id != "{" && ast->id != "return" && ast->id != "$") {
      
      if (ast->type.empty()) {
        cout << ast->id << " " << endl;
      }
      else{
        cout << ast->id << " ";
      }
    }
  }
    
  if (!ast->type.empty()) {
    if (ast->type.find("return type: ") != string::npos) cout << "(" << ast->type << ")" << endl;
    else cout << "(type: " << ast->type << ")" << endl;
  }
    
  //this is the traverse part
  for (int i = 0; i < 3; i++) {
    if (ast->child[i] != NULL) {
      printAST(ast->child[i]);
    }
  }
    
}

void printIdent() {
    //directly prints out space
    for (int i = 0; i < indentation; i++) {
            cout << " ";
    }
    
}

void fail(string errorMessage) {
    cerr << errorMessage << endl;
    exit(1);
}