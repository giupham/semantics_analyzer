#ifndef LEXICAL_H
#define LEXICAL_H

#include <iostream>
#include <cctype>
#include <string>
#include <fstream>
#include <vector>
using namespace std;

// Function that says if an element is part of an array (both must be the same type)
template<class Type>
bool isInArray(const Type array[], int size, Type element) {
	for (int i = 0; i < size; i++) {
		if (element == array[i]) return true;
	}
	return false;
}

template<class Type>
int indexOf(const Type array[], int size, Type element) {
	for (int i = 0; i < size; i++) {
		if (element == array[i]) return i;
	}
	return -1;
}

// Our Token Structure
class Token {
public:
	int line; // line where the token was found
	string type; // For example: "integer"
	string lexeme; // For example: "123"
	string comment; // For any aditional comments
	// Constructors
	Token() {}
	Token(int ln, string tp, string lx) : line(ln), type(tp), lexeme(lx) {}
	Token(int ln, string tp, string lx, string cm) : line(ln), type(tp), lexeme(lx), comment(cm) {}
};

class Lexer {
private:
	// attributes for our lexer class
	const string KEYS[10] = { "integer", "boolean", "if", "fi", "otherwise", "put", "get", "while", "true", "false" };
	const char OPERATORS[8] = { '+', '<', '>', '/', '*', '-', '=', '|' };
	const char SEPARATORS[6] = { '[' , '{', '}', ';', '(', ')' };

	enum STATES {
		ST_ERROR = -1,
		ST_START = 0,
		ST_IDENTIFIER_S, // single letter
		ST_IDENTIFIER_WU, // with underline
		ST_IDENTIFIER_WL, // with more letters
		ST_INTEGER,
	};

	int nLines;
	int nErrors;
	vector<Token> tokens;

public:
	// Constructor
	Lexer() : nLines(0), nErrors(0) {}
	// Getters and setters
	vector<Token> getTokens() { return tokens; }

	// This is our FSM for integers and identifiers
	void FSM(int& state, char k) {
		if (state == ST_START) {
			if (isalpha(k))
				state = ST_IDENTIFIER_S;
			else if (isdigit(k))
				state = ST_INTEGER;
			else
				state = ST_ERROR;
		}
		else if (state == ST_INTEGER and isdigit(k))
			state = ST_INTEGER;
		else if (state == ST_IDENTIFIER_S) {
			if (isalpha(k))
				state = ST_IDENTIFIER_WL;
			else if (k == '_')
				state = ST_IDENTIFIER_WU;
			else state = ST_ERROR;
		}
		else if (state == ST_IDENTIFIER_WL) {
			if (k == '_')
				state = ST_IDENTIFIER_WU;
			else if (!isalpha(k))
				state = ST_ERROR;
		}
		else if (state == ST_IDENTIFIER_WU) {
			if (isalpha(k))
				state = ST_IDENTIFIER_WL;
			else if (k != '_')
				state = ST_ERROR;
		}
		else
			state = ST_ERROR;
	}

	// Function just for traverse our tokens and save them into an output file
	void saveResult() {
		// output file
		ofstream out("result_lexemes.txt");
		// save every token in our vector
		out << "    TOKEN\t\tLEXEME" << endl;
		out << "---------------------------------------" << endl;
		for (int i = 0; i < tokens.size(); i++) {
			out << "    " << tokens[i].type << "\t\t" << tokens[i].lexeme;
			if (tokens[i].comment != "") out << "    LEXICAL ERROR: " << tokens[i].comment;
			out << endl;
		}
		// close file
		out.close();
	}

	// Function to add a token to the list
	void addToken(string& lexeme, int& state) {
		//cout << "test addToken current lexeme is " << lexeme << endl;
		// Validation for avoid to push empty strings when are consecutive spaces
		if (state == ST_START) return;
		// If token is an identifier (validate first if it is a keyword)
		if (state == ST_IDENTIFIER_S or state == ST_IDENTIFIER_WU or state == ST_IDENTIFIER_WL) {
			if (isInArray(KEYS, 10, lexeme)) tokens.push_back(Token(nLines, "keyword", lexeme));
			// Validating maximum size
			else
				tokens.push_back(Token(nLines, "identifier", lexeme));
		}
		// If token is an integer
		else if (state == ST_INTEGER) {
			// Validating maximum size
			if (lexeme.length() <= 10)
				tokens.push_back(Token(nLines, "integer", lexeme));
			else {
				cout << "-Lexical Error in line " << nLines << ": Integer '" << lexeme << "' too long (must be at most 10 digits)" << endl;
				tokens.push_back(Token(nLines, "integer", lexeme, "too long (must be at most 10 digits)"));
				nErrors++;
			}
		}
		else {
			cout << "-Lexical Error in line " << nLines << ": Lexeme '" << lexeme << "' is not recognized." << endl;
			tokens.push_back(Token(nLines, "UNKNOWN", lexeme, "token could not be identified"));
			nErrors++;
		}
		// Clean our variables
		lexeme = "";
		state = ST_START;
	}

	// Function that reads line by line and stores all tokens into a vector
	bool analyze(string inputName) {
		ifstream in(inputName);
		// Just a open validation
		if (in.is_open()) {
			// Vars Definition
			string line, lexeme = "";
			bool isComment = false;
			int state = ST_START;
			// Process line by line
			while (getline(in, line)) {
				nLines++;
				for (int i = 0; i < line.length(); i++) {
					//cout << "test curr lexeme is " << lexeme << endl;
					// Closing comments when "*]" is found
					if (i < line.length() - 1 and line[i] == '*' and line[i + 1] == ']')
						isComment = false, i += 1;
					// Open comments when "[*" is found
					else if (i < line.length() - 1 and line[i] == '[' and line[i + 1] == '*')
						isComment = true, i += 1;
					// Process token just if is not inside a comment
					else if (!isComment) {
						// Separator $$
						if (i < line.length() - 1 and line[i] == '$' and line[i + 1] == '$') {
							addToken(lexeme, state); // finish the previous lexeme and add it
							tokens.push_back(Token(nLines, "separator", "$$")), i += 1;
						}
						// Rest of separators
						else if (isInArray(OPERATORS, 8, line[i])) {
							if (i < line.length() - 1 and isInArray(OPERATORS, 8, line[i]) and isInArray(OPERATORS, 8, line[i + 1])) {
								int j = i + 1;
								while (isInArray(OPERATORS, 8, line[j + 1])) {
									j++;
								}
								if ((line[i] == '=' && line[i + 1] == '=')) {
									addToken(lexeme, state); // finish the previous lexeme and add it
									tokens.push_back(Token(nLines, "operator", "==")), i += 1;
								}
								else {
									addToken(lexeme, state);
									tokens.push_back(Token(nLines, "UNKNOWN", line.substr(i, j - i + 1)));
								}
								i = j;
							}
							else {
								addToken(lexeme, state);
								tokens.push_back(Token(nLines, "operator", string(1, line[i])));
							}
						}
						// Operators
						else if (isInArray(SEPARATORS, 6, line[i])) {
							addToken(lexeme, state); // finish the previous lexeme and add it
							tokens.push_back(Token(nLines, "separator", string(1, line[i])));
						}
						// When read an space, finish the current token read (and restart FSM)
						else if (isspace(line[i])) {
							addToken(lexeme, state);
						}
						else {
							lexeme += line[i];
							FSM(state, line[i]);
						}
					}
				}
				// just in case the last line is ended without space
				// remember that if is an empty space the token will not be added
				addToken(lexeme, state);
			}
			// closing files
			in.close();
			// saving results in the output
			saveResult();
		}
		else {
			string inputNameAgain = "";
			cout << "File does not exist. Please re-enter file name: ";
			cin >> inputNameAgain;
			analyze(inputNameAgain);
		}
		// Returning the tokens
		return (nErrors == 0);
	}
};

#endif
