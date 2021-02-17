#ifndef SYNTACTICAL_H
#define SYNTACTICAL_H

#include<iostream>
#include<cstdlib>
#include<string>
#include<cstdio>
#include <stack>
#include "lexical.h"

#define SYMBOL_TABLE_SIZE 200

#define INSTRUCTION_TABLE_SIZE 400

using namespace std;

class InstructionTableEntry {
public:
	string op, operand;
	int address;
	InstructionTableEntry(int address, string op, string operand) {
		this->op = op;
		this->address = address;
		this->operand = operand;
	}
};


class InstructionTable {
private:
	InstructionTableEntry** t;
	stack <int> jump_stack;
	int inst_address, label_counter;

public:
	InstructionTable() {
		t = new InstructionTableEntry * [INSTRUCTION_TABLE_SIZE];
		inst_address = label_counter = 0;
		for (int i = 0; i < INSTRUCTION_TABLE_SIZE; i++) {
			t[i] = NULL;
		}
	}

	int GenInstrAddress() {
		return inst_address;
	}

	void GenInstr(string op, string operand) {
		t[inst_address] = new InstructionTableEntry(inst_address + 1, op, operand);
		inst_address++;
	}

	void GenLabelInstr() {
		t[inst_address] = new InstructionTableEntry(inst_address + 1, "Label" + to_string(label_counter), "");
		inst_address++;
		label_counter++;
	}

	void PushJumpStack() {
		jump_stack.push(inst_address);
		inst_address++;
	}

	void SetAtJumpStackTop(string op) {
		int inst = jump_stack.top();
		jump_stack.pop();
		t[inst] = new InstructionTableEntry(inst + 1, op, to_string(inst_address + 1));
	}

	void Print() {
		cout << "\nInstruction Table\naddress\t\top\t\toperand\n";
		for (int i = 0; i < INSTRUCTION_TABLE_SIZE; i++)
			if (t[i] != NULL)
				cout << t[i]->address << "\t\t" << t[i]->op << "\t\t" << t[i]->operand << endl;
	}

	~InstructionTable() {

		for (int i = 0; i < INSTRUCTION_TABLE_SIZE; i++) {
			if (t[i] != NULL)
				delete t[i];
		}
	}
};

class HashTableEntry {
public:
	string id, type;
	int location;
	HashTableEntry(string id, string type, int location) {
		this->id = id;
		this->type = type;
		this->location = location;
	}
};

class SymbolTable {
private:
	HashTableEntry** t;
	int memory_address;

	unsigned int HashFunc(string k) {
		unsigned int hash = 5381;
		for (int i = 0; i < k.length(); i++) hash = ((hash << 5) + hash) + k.at(i); /* hash * 33 + k[i] */
		return hash % SYMBOL_TABLE_SIZE;
	}

public:
	
	SymbolTable() {
		t = new HashTableEntry * [SYMBOL_TABLE_SIZE];
		memory_address = 5000;
		for (int i = 0; i < SYMBOL_TABLE_SIZE; i++) {
			t[i] = NULL;
		}
	}

	int Insert(string id, string type) {
		unsigned int h = HashFunc(id);
		if (t[h] != NULL) return -1;
		t[h] = new HashTableEntry(id, type, memory_address);
		memory_address += 1;
		return 0;
	}

	int SearchKey(string id) {
		unsigned int h = HashFunc(id);
		return t[h] == NULL ? -1 : h;
	}

	unsigned int GetLocation(unsigned int h) {
		return t[h]->location;
	}

	string GetType(unsigned int h) {
		return t[h]->type;
	}

	void Print() {
		
		cout << "\nSymbol Table\nid\t\tlocation\t\ttype  \n";
		for (int i = 0; i < SYMBOL_TABLE_SIZE; i++)
			if (t[i] != NULL)
				cout << t[i]->id << "\t\t" << t[i]->location << "\t\t\t" << t[i]->type << endl;
	}

	~SymbolTable() {
		for (int i = 0; i < SYMBOL_TABLE_SIZE; i++) {
			if (t[i] != NULL)
				delete t[i];
		}
	}
};

class Syntactical {
private:
	// attributes
	vector<Token> tokens;
	int idNextToken;
	Token currToken;
	int nErrors;
	ofstream out;
	string currInstr;

	SymbolTable symbolTable;
	InstructionTable instructionTable;

	// Function for getting the next lexeme
	void nextToken() {
		// Printing the current token
		if (idNextToken <= tokens.size() and idNextToken > 0) {
			out << "Token: " << currToken.type << "\t Lexeme: " << currToken.lexeme << endl;
			out << currInstr << endl;
			currInstr = "";
		}
		// Getting the next token
		if (idNextToken < tokens.size()) currToken = tokens[idNextToken++];
		else currToken = Token(currToken.line, "None", "");
	}

	// Function for printing errors
	void printError(string lexeme, string expected) {
		out << "Lexeme: " << lexeme << " Syntax Error in line " << currToken.line << ": " << expected << " expected" << endl;
		nErrors++;
	}

public:
	// Constructor Function
	Syntactical() : idNextToken(0), nErrors(0) {}
	// Launcher Function
	bool analyze(vector<Token> tks) {
		out.open("result_syntax.txt");
		tokens = tks;
		nextToken();
		RL_RAT20SU();
		if (currToken.type != "None") {
			out << "Semantic Error in line " << currToken.line << ": Cannot declare it twice!" << endl;
			nErrors++;
		}
		out.close();
		return (nErrors == 0);
	}

	// ============ FUNCTIONS FOR THE SYNTAX RULES =================
	void RL_RAT20SU() {
		
		// Firsts set for this rule
		string firsts[] = { "$$" };
		// Processing rule
		if (isInArray(firsts, 1, currToken.lexeme)) {
			// Concat the instruction of this rule
			currInstr += "\t<RAT20SU> -> $$ <OPT_DEC_LIST> <STATEMENT_LIST> $$\n";
			// Consume last token
			nextToken();
			// Now let's process the rest of the rule
			RL_OPT_DEC_LIST();
			RL_STATEMENT_LIST();
			if (currToken.lexeme == "$$") {
				nextToken();

				instructionTable.Print();
				symbolTable.Print();

			}
			else printError(currToken.lexeme, "$$");
		}
		else
			printError(currToken.lexeme, "$$");
		
	}

	void RL_OPT_DEC_LIST() {
		// Firsts set for this rule
		string firsts[] = { "integer", "boolean" };
		// in firsts is also included epsilon then we will also need to the follows for validation
		string follows[] = { "{", "if", "get", "put", "while" }; // Also include TT:Identifier (TokenType:Identifier)
		// Processing rule
		if (isInArray(firsts, 2, currToken.lexeme)) {
			// Concat the instruction of this rule
			currInstr += "\t<OPT_DEC_LIST> -> <DECLARATION_LIST> | <EMPTY>\n";
			// Now let's process the rest of the rule
			RL_DECLARATION_LIST();
		}
		else if (!isInArray(follows, 5, currToken.lexeme) and currToken.type != "identifier")
			// printError("'integer', 'boolean', {', 'if', 'get', 'put' or 'while'");
			printError(currToken.lexeme, "A first of a declaration or a statement list");

	}

	void RL_DECLARATION_LIST() {
		// Firsts set for this rule
		string firsts[] = { "integer", "boolean" };
		// Processing rule
		if (isInArray(firsts, 2, currToken.lexeme)) {
			// Concat the instruction of this rule
			currInstr += "\t<DECLARATION_LIST> -> <DECLARATION> ; <DECLARATION_LIST_PRIME>\n";
			// Now let's process the rest of the rule
			RL_DECLARATION();
			if (currToken.lexeme == ";") nextToken();
			else printError(currToken.lexeme, ";");
			RL_DECLARATION_LIST_PRIME();
		}
		else
			printError(currToken.lexeme, "'integer' or 'boolean'");
	}

	void RL_DECLARATION_LIST_PRIME() {
		// Firsts set for this rule
		string firsts[] = { "integer", "boolean" };
		// in firsts is also included epsilon then we will also need to the follows for validation
		string follows[] = { "{", "if", "get", "put", "while" }; // Also include TT:Identifier
		// Processing rule
		if (isInArray(firsts, 2, currToken.lexeme)) {
			// Concat the instruction of this rule
			currInstr += "\t<DECLARATION_LIST_PRIME> -> <DECLARATION_LIST> | <EMPTY>\n";
			// Now let's process the rest of the rule
			RL_DECLARATION_LIST();
		}
		else if (!isInArray(follows, 5, currToken.lexeme) and currToken.type != "identifier")
			// printError("'integer', 'boolean', '{', 'if', 'get', 'put', 'while' or Identifier");
			printError(currToken.lexeme, "A first of a declaration or a statement list");
	}

	void RL_DECLARATION() {
		// Firsts set for this rule
		string firsts[] = { "integer", "boolean" };
		// Processing rule
		if (isInArray(firsts, 2, currToken.lexeme)) {
			// Concat the instruction of this rule
			currInstr += "\t<DECLARATION> -> <QUALIFIER> <TT_Identifier>\n";
			// Now let's process the rest of the rule
			string type = RL_QUALIFIER();
			if (currToken.type == "identifier") {
				int ret = symbolTable.Insert(currToken.lexeme, type);
				if (ret == -1) printError(currToken.lexeme, "Redeclaration of Identifier");
				nextToken();
			}
			else printError(currToken.lexeme, "Identifier");
		}
		else
			printError(currToken.lexeme, "'integer' or 'boolean'");
	}

	string RL_QUALIFIER() {
		// Firsts set for this rule
		string firsts[] = { "integer", "boolean" };
		// Processing rule
		if (isInArray(firsts, 2, currToken.lexeme)) {
			// Concat the instruction of this rule
			currInstr += "\t<QUALIFIER> -> integer | boolean\n";
			// Just Consume last token
			string type = currToken.lexeme;
			nextToken();
			return type;
		}
		else {
			printError(currToken.lexeme, "'integer' or 'boolean'");
			return "";
		}
	}

	void RL_STATEMENT_LIST() {
		// Firsts set for this rule
		string firsts[] = { "{", "if", "get", "put", "while" }; // Also include TT:Identifier
		// Processing rule
		if (isInArray(firsts, 5, currToken.lexeme) or currToken.type == "identifier") {
			// Concat the instruction of this rule
			currInstr += "\t<STATEMENT_LIST> -> <STATEMENT> <STATEMENT_LIST_PRIME>\n";
			// Now let's process the rest of the rule
			RL_STATEMENT();
			RL_STATEMENT_LIST_PRIME();
		}
		else
			printError(currToken.lexeme, "A first symbol of Statement");
		// printError("'{', 'if', 'get', 'put', 'while' or Identifier");

	}

	void RL_STATEMENT_LIST_PRIME() {
		// Firsts set for this rule
		string firsts[] = { "{", "if", "get", "put", "while" }; // Also include TT:Identifier
		// in firsts is also included epsilon then we will also need to the follows for validation
		string follows[] = { "$$", "}" };
		// Processing rule
		if (isInArray(firsts, 5, currToken.lexeme) or currToken.type == "identifier") {
			// Concat the instruction of this rule
			currInstr += "\t<STATEMENT_LIST_PRIME> -> <STATEMENT_LIST> | <EMPTY>\n";
			// Now let's process the rest of the rule
			RL_STATEMENT_LIST();
		}
		else if (!isInArray(follows, 2, currToken.lexeme))
			printError(currToken.lexeme, "A first of Statement or $$");
		// printError("'{', 'if', 'get', 'put', 'while', '$$', '}' or Identifier");

	}

	void RL_STATEMENT() {
		// Firsts set for this rule
		string firsts[] = { "{", "if", "get", "put", "while" }; // Also include TT:Identifier
		// Processing rule
		if (isInArray(firsts, 5, currToken.lexeme) or currToken.type == "identifier") {
			// Concat the instruction of this rule
			currInstr += "\t<STATEMENT> -> <COMPOUND> | <ASSIGN> | <IF> | <GET> | <PUT> | <WHILE>\n";
			// Now let's process the rest of the rule
			if (currToken.lexeme == "{") RL_COMPOUND();
			else if (currToken.lexeme == "if") RL_IF();
			else if (currToken.lexeme == "get") RL_GET();
			else if (currToken.lexeme == "put") RL_PUT();
			else if (currToken.lexeme == "while") RL_WHILE();
			else RL_ASSIGN();
		}
		else
			printError(currToken.lexeme, "First symbol of Compound, Assign, If, Get, Put or While");

	}

	void RL_COMPOUND() {
		// Firsts set for this rule
		// The only first for this rule is "{"
		if (currToken.lexeme == "{") {
			// Concat the instruction of this rule
			currInstr += "\t<COMPOUND> -> { <STATEMENT> <STATEMENT_LIST_PRIME> }\n";
			// Consume last token
			nextToken();
			// Now let's process the rest of the rule
			RL_STATEMENT();
			RL_STATEMENT_LIST();
			if (currToken.lexeme == "}") nextToken();
			else printError(currToken.lexeme, "'}'");
		}
		else
			printError(currToken.lexeme, "'{'");
	}

	void RL_ASSIGN() {
		// The only first for this rule is an Identifier(Token Type)
		if (currToken.type == "identifier") {
			// Concat the instruction of this rule
			currInstr += "\t<ASSIGN> -> <TT_Identifier> = <EXPRESSION> ;\n";
			// Consume last token
			int h = symbolTable.SearchKey(currToken.lexeme);
			if (h == -1) printError(currToken.lexeme, "Variable used for assignment without being declared before");
			nextToken();

			if (currToken.lexeme == "=") nextToken();
			else printError(currToken.lexeme, "'='");

			string type = RL_EXPRESSION();
			if (symbolTable.GetType(h) != type) {
				cout << "Semantic Error in line " << currToken.line << ": Type mismatch" << endl;
				exit(0);
			}
			instructionTable.GenInstr("POPM", to_string(symbolTable.GetLocation(h)));
			if (currToken.lexeme == ";") nextToken();
			else printError(currToken.lexeme, "';'");
		}
		else
			printError(currToken.lexeme, "Identifier");
	}

	void RL_IF() {
		// Firsts set for this rule
		// The only first for this rule is "if"
		if (currToken.lexeme == "if") {
			// Concat the instruction of this rule
			currInstr += "\t<IF> -> if ( <CONDITION> ) <STATEMENT> <IF_PRIME>\n";
			// Consume last token
			nextToken();

			if (currToken.lexeme == "(") nextToken();
			else printError(currToken.lexeme, "'('");

			RL_CONDITION();

			if (currToken.lexeme == ")") nextToken();
			else printError(currToken.lexeme, "')'");

			RL_STATEMENT();
			RL_IF_PRIME();
		}
		else
			printError(currToken.lexeme, "'if'");


	}

	void RL_IF_PRIME() {
		// Firsts set for this rule
		string firsts[] = { "fi", "otherwise" };
		// Processing rule
		if (isInArray(firsts, 2, currToken.lexeme)) {
			// Concat the instruction of this rule
			currInstr += "\t<IF_PRIME> -> fi | otherwise <STATEMENT> fi\n";

			// Now let's process the rest of the rule
			if (currToken.lexeme == "fi") {
				nextToken();
				instructionTable.SetAtJumpStackTop("JUMPZ");
			}
			else {
				nextToken();
				instructionTable.SetAtJumpStackTop("JUMPZ");
				RL_STATEMENT();
				if (currToken.lexeme == "fi") nextToken();
				else printError(currToken.lexeme, "'fi'");
			}
		}
		else
			printError(currToken.lexeme, "'fi' or 'otherwise'");
	}

	void RL_PUT() {
		// Firsts set for this rule
		// The only first for this rule is "put"
		if (currToken.lexeme == "put") {
			// Concat the instruction of this rule
			currInstr += "\t<PUT> -> put ( <TT_Identifier> ) ;\n";
			// Consume last token
			nextToken();

			if (currToken.lexeme == "(") nextToken();
			else printError(currToken.lexeme, "'('");

			if (currToken.type == "identifier") {
				int h = symbolTable.SearchKey(currToken.lexeme);
				if (h == -1) printError(currToken.lexeme, "Variable used for output without being declared before");
				nextToken();
				instructionTable.GenInstr("PUSHM", to_string(symbolTable.GetLocation(h)));
				instructionTable.GenInstr("STDOUT", "");
			}
			else printError(currToken.lexeme, "Identifier");

			if (currToken.lexeme == ")") nextToken();
			else printError(currToken.lexeme, "')'");

			if (currToken.lexeme == ";") nextToken();
			else printError(currToken.lexeme, "';'");
		}
		else
			printError(currToken.lexeme, "'put'");

	}

	void RL_GET() {
		// Firsts set for this rule
		// The only first for this rule is "get"
		if (currToken.lexeme == "get") {
			// Concat the instruction of this rule
			currInstr += "\t<GET> -> get ( <TT_Identifier> ) ;\n";
			// Consume last token
			nextToken();

			if (currToken.lexeme == "(") nextToken();
			else printError(currToken.lexeme, "'('");

			if (currToken.type == "identifier") {
				int h = symbolTable.SearchKey(currToken.lexeme);
				if (h == -1) printError(currToken.lexeme, "Variable used for input without being declared before");
				nextToken();
				instructionTable.GenInstr("STDIN", "");
				instructionTable.GenInstr("POPM", to_string(symbolTable.GetLocation(h)));
			}
			else printError(currToken.lexeme, "Identifier");

			if (currToken.lexeme == ")") nextToken();
			else printError(currToken.lexeme, "')'");

			if (currToken.lexeme == ";") nextToken();
			else printError(currToken.lexeme, "';'");
		}
		else
			printError(currToken.lexeme, "'get'");

	}

	void RL_WHILE() {
		// Firsts set for this rule
		// The only first for this rule is "while"
		if (currToken.lexeme == "while") {
			// Concat the instruction of this rule
			currInstr += "\t<WHILE> -> while ( <CONDITION> ) <STATEMENT>\n";
			// Consume last token
			nextToken();

			if (currToken.lexeme == "(") nextToken();
			else printError(currToken.lexeme, "'('");

			instructionTable.GenLabelInstr();
			int address = instructionTable.GenInstrAddress();

			RL_CONDITION();

			if (currToken.lexeme == ")") nextToken();
			else printError(currToken.lexeme, "')'");

			RL_STATEMENT();

			instructionTable.GenInstr("JUMP", to_string(address));
			instructionTable.SetAtJumpStackTop("JUMPZ");
		}
		else
			printError(currToken.lexeme, "'while'");

	}

	void RL_CONDITION() {
		// Firsts set for this rule
		string firsts[] = { "-", "(", "true", "false" }; // Also include TT:Identifier and TT:Integer
		// Processing rule
		if (isInArray(firsts, 4, currToken.lexeme) or currToken.type == "identifier" or currToken.type == "integer") {
			// Concat the instruction of this rule
			currInstr += "\t<CONDITION> -> <EXPRESSION> <RELOP> <EXPRESSION>\n";
			// Now let's process the rest of the rule
			RL_EXPRESSION();
			string token = RL_RELOP();
			RL_EXPRESSION();
			if (token == "==") {
				instructionTable.GenInstr("EQU", "");
				instructionTable.PushJumpStack();
			}
			else if (token == ">") {
				instructionTable.GenInstr("GRT", "");
				instructionTable.PushJumpStack();
			}
			else if (token == "<") {
				instructionTable.GenInstr("LES", "");
				instructionTable.PushJumpStack();
			}
		}
		else
			printError(currToken.lexeme, "First symbol of a Expression");
		// printError("'-', '(', 'true', 'false', Indentifier or Integer");
	}

	string RL_RELOP() {
		// Firsts set for this rule
		string firsts[] = { "==", ">", "<" };
		// Processing rule
		if (isInArray(firsts, 3, currToken.lexeme)) {
			// Concat the instruction of this rule
			currInstr += "\t<RELOP> -> == | > | <\n";
			// Just consume last token
			string token = currToken.lexeme;
			nextToken();
			return token;
		}
		else
			printError(currToken.lexeme, "Relational Operators: '==', '>' or '<'");
		return "";
	}

	string RL_EXPRESSION() {
		// Firsts set for this rule
		string firsts[] = { "-", "(", "true", "false" }; // Also include TT:Identifier and TT:Integer
		// Processing rule
		if (isInArray(firsts, 4, currToken.lexeme) or currToken.type == "identifier" or currToken.type == "integer") {
			// Concat the instruction of this rule
			currInstr += "\t<EXPRESSION> -> <TERM> <EXPRESSION_PRIME>\n";
			// Now let's process the rest of the rule
			string type1 = RL_TERM();
			string type2 = RL_EXPRESSION_PRIME();
			if (type2 != "" && (type1 != type2 || type2 == "boolean")) {
				cout << "Semantic Error in line " << currToken.line << ": Type mismatch" << endl;
				exit(0);
			}
			return type1;
		}
		else {
			printError(currToken.lexeme, "First symbol of a Term");
			return "";
		}
	}

	string RL_EXPRESSION_PRIME() {
		// Firsts set for this rule
		string firsts[] = { "+", "-" };
		// in firsts is also included epsilon then we will also need to the follows for validation
		string follows[] = { ";", "==", ">", "<", ")" };
		// Processing rule
		if (isInArray(firsts, 2, currToken.lexeme)) {
			// Concat the instruction of this rule
			currInstr += "\t<EXPRESSION_PRIME> -> + <TERM> <EXPRESSION_PRIME> | - <TERM> <EXPRESSION_PRIME> | <EMPTY>\n";
			// Consume last token
			string token = currToken.lexeme;
			nextToken();
			// Now let's process the rest of the rule
			string type = RL_TERM();
			instructionTable.GenInstr(token == "+" ? "ADD" : "SUB", "");
			RL_EXPRESSION_PRIME();
			return type;
		}
		else if (!isInArray(follows, 5, currToken.lexeme)) {
			printError(currToken.lexeme, "'+', '-', ';' or Relational Operators");
			return "";
		}
		return "";
	}

	string RL_TERM() {
		// Firsts set for this rule
		string firsts[] = { "-", "(", "true", "false" }; // Also include TT:Identifier and TT:Integer
		// Processing rule
		if (isInArray(firsts, 4, currToken.lexeme) or currToken.type == "identifier" or currToken.type == "integer") {
			// Concat the instruction of this rule
			currInstr += "\t<TERM> -> <FACTOR> <TERM_PRIME>\n";
			// Now let's process the rest of the rule
			string type1 = RL_FACTOR();
			string type2 = RL_TERM_PRIME();
			if (type2 != "" && (type1 != type2 || type2 == "boolean")) {
				cout << "Semantic Error in line " << currToken.line << ": Type mismatch" << endl;
				exit(0);
			}
			return type1;
		}
		else {
			printError(currToken.lexeme, "First of Factor");
			return "";
		}
	}

	string RL_TERM_PRIME() {
		// Firsts set for this rule
		string firsts[] = { "*", "/" };
		// in firsts is also included epsilon then we will also need to the follows for validation
		string follows[] = { "+", "-", ";", "==", ">", "<", ")" };
		// Processing rule
		if (isInArray(firsts, 2, currToken.lexeme)) {
			// Concat the instruction of this rule
			currInstr += "\t<TERM_PRIME> -> * <FACTOR> <TERM_PRIME> | / <FACTOR> <TERM_PRIME> | <EMPTY>\n";
			// Consume last token
			string token = currToken.lexeme;
			nextToken();
			// Now let's process the rest of the rule
			string type = RL_FACTOR();
			instructionTable.GenInstr(token == "*" ? "MUL" : "DIV", "");
			RL_TERM_PRIME();
			return type;
		}
		else if (!isInArray(follows, 7, currToken.lexeme)) {
			printError(currToken.lexeme, "'*', '/', '+', '-', ';', '==', '>', '<' or ')'");
			return "";
		}
		return "";
	}

	string RL_FACTOR() {
		// Firsts set for this rule
		string firsts[] = { "-", "(", "true", "false" }; // Also include TT:Identifier and TT:Integer
		// Processing rule
		if (isInArray(firsts, 4, currToken.lexeme) or currToken.type == "identifier" or currToken.type == "integer") {
			// Concat the instruction of this rule
			currInstr += "\t<FACTOR> -> - <PRIMARY>  | <PRIMARY>\n";
			// Now let's process the rest of the rule
			bool is_negative = false;
			if (currToken.lexeme == "-") {
				is_negative = true;
				nextToken();
				instructionTable.GenInstr("PUSHI", "0");
			}
			string type = RL_PRIMARY();
			if (is_negative) instructionTable.GenInstr("SUB", "");
			return type;
		}
		else {
			printError(currToken.lexeme, "'-', or Primary");
			return "";
		}
	}

	string RL_PRIMARY() {
		// Firsts set for this rule
		string firsts[] = { "-", "(", "true", "false" }; // Also include TT:Identifier and TT:Integer
		// Processing rule
		if (isInArray(firsts, 4, currToken.lexeme) or currToken.type == "identifier" or currToken.type == "integer") {
			// Concat the instruction of this rule
			currInstr += "\t<PRIMARY> -> <TT_Identifier> | <TT_Integer> | ( <FACTOR> <TERM_PRIME> <EXPRESSION_PRIME> ) | true | false\n";
			// Now let's process the rest of the rule
			if (currToken.lexeme == "(") {
				nextToken();
				RL_FACTOR();
				RL_TERM_PRIME();
				RL_EXPRESSION_PRIME();
				if (currToken.lexeme == ")") nextToken();
				else printError(currToken.lexeme, ")");
			}
			else {
				if (currToken.type == "identifier") {
					int h = symbolTable.SearchKey(currToken.lexeme);
					if (h == -1) printError(currToken.lexeme, "Variable used before declaration");
					instructionTable.GenInstr("PUSHM", to_string(symbolTable.GetLocation(h)));
					nextToken();
					return symbolTable.GetType(h);
				}
				else if (currToken.type == "integer") {
					instructionTable.GenInstr("PUSHI", currToken.lexeme);
					nextToken();
					return "integer";
				}
				else {
					instructionTable.GenInstr("PUSHI", currToken.lexeme == "true" ? "1" : "0");
					nextToken();
					return "boolean";
				}
			}

		}
		else {
			printError(currToken.lexeme, "'-', '(', 'true', 'false', Indentifier or Integer");
			return "";
		}
		return "";
	}

	void RL_EMPTY() {
		// in firsts is included just epsilon then we will need to the follows for validation
		string follows[] = { "+", "-", ";", "==", ">", "<", ")", "$$", "{", "}", "if", "get", "put", "while" }; // Also is included TT:Identifier
		// Processing rule
		if (!isInArray(follows, 14, currToken.lexeme) and currToken.type != "identifier" and currToken.type != "integer") {
			printError(currToken.lexeme, "'+', '-', ';', '==', '>', '<', ')', '$$', '{', '}', 'if', 'get', 'put' or 'while'");
		}
	}

};

#endif