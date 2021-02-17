#include <iostream>
#include <vector>
#include "lexical.h"
#include "syntactical.h"
using namespace std;


int main() {
	// variables definition
	string inputName;
	Lexer lex;
	Syntactical synt;

	cout << "Enter the name of the file you would like to Analyze: ";
	cin >> inputName;
	cout << endl;
	// Lexical part of the compiler
	if (!lex.analyze(inputName)) cout << endl << "Compilation aborted because of lexical errors!!" << endl << endl;
	// Syntactical part of the compiler
	else {
		synt.analyze(lex.getTokens());		
	}

	return 0;
}