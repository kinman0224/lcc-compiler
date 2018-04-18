#include "error.h"

#include <vector>
#include <map>
#include <string>
#include <iostream>

using namespace std;

enum TSymbol {
	$LCMT = 1, $RCMT, $CMT,
	$INT, $VOID,
	$ID, $NUM,
	$SEMICOLON, $COMMA, $ASSIGN,
	$LPAREN, $RPAREN, $LCURLY, $RCURLY,
	$RETURN, $WHILE, $IF, $ELSE,
	$LE, $LT, $GE, $GT, $EQUAL, $NEQUAL,
	$PLUS, $MIN, $STAR, $SLASH,
	$FINISH
};

typedef map<TSymbol, string> SymbolList;

typedef struct Word {
	TSymbol t_symbol;	//symbol type
	string token;
	int wordline;
} Word;

class Lexer : protected Error {
protected:
	vector<Word> wordList;
	SymbolList symbolList;
	void initializeSymbolList();

private:
	int _keyword(string);
	int _id(string);
	int _num(string);
	int _operator(string);
	int _delimiter(string);
	int _digit(char);
	int _letter(char);

	Word createWord(TSymbol, string);
	string::iterator iter;
	int line;

public:
	void analyze(string input);
	void printResult();

};