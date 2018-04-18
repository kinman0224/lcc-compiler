#include "lexer.h"

void Lexer::initializeSymbolList()
{
	symbolList[$LCMT] = "$LCMT";
	symbolList[$RCMT] = "$RCMT";
	symbolList[$CMT] = "$CMT";
	symbolList[$INT] = "$INT";
	symbolList[$VOID] = "$VOID";
	symbolList[$ID] = "$ID";
	symbolList[$NUM] = "$NUM";
	symbolList[$SEMICOLON] = "$SEMICOLON";
	symbolList[$COMMA] = "$COMMA";
	symbolList[$ASSIGN] = "$ASSIGN";
	symbolList[$LPAREN] = "$LPAREN";
	symbolList[$RPAREN] = "$RPAREN";
	symbolList[$LCURLY] = "$LCURLY";
	symbolList[$RCURLY] = "$RCURLY";
	symbolList[$RETURN] = "$RETURN";
	symbolList[$WHILE] = "$WHILE";
	symbolList[$IF] = "$IF";
	symbolList[$ELSE] = "$ELSE";
	symbolList[$LE] = "$LE";
	symbolList[$LT] = "$LT";
	symbolList[$GE] = "$GE";
	symbolList[$EQUAL] = "$EQUAL";
	symbolList[$NEQUAL] = "$NEQUAL";
	symbolList[$PLUS] = "$PLUS";
	symbolList[$MIN] = "$MIN";
	symbolList[$STAR] = "$STAR";
	symbolList[$SLASH] = "$SLASH";

}

int Lexer::_digit(char input)
{
	if (input >= '0' && input <= '9')
		return 1;

	return 0;
}

int Lexer::_letter(char input)
{
	if ((input >= 'a' && input <= 'z') || (input >= 'A' && input <= 'Z'))
		return 1;

	return 0;
}

int Lexer::_keyword(string input)
{
	if (input == "int") {
		return $INT;
	}
	else if (input == "void") {
		return $VOID;
	}
	else if (input == "if") {
		return $IF;
	}
	else if (input == "else") {
		return $ELSE;
	}
	else if (input == "while") {
		return $WHILE;
	}
	else if (input == "return") {
		return $RETURN;
	}

	return 0;
}

int Lexer::_id(string input)
{
	string::iterator iter = input.begin();

	if (_letter(*iter)) {
		for (++iter; iter != input.end(); ++iter) {
			if (_digit(*iter) || _letter(*iter)) {
				;
			}
			else {
				return 0;
			}
		}
	}
	else {
		return 0;
	}

	return 1;
}

int Lexer::_num(string input)
{
	string::iterator iter = input.begin();

	for (; iter != input.end(); ++iter) {
		if (!_digit(*iter)) {
			return 0;
		}
	}

	return 1;
}

int Lexer::_operator(string input)
{
	if (input == "+") {
		return $PLUS;
	}
	else if (input == "-") {
		return $MIN;
	}
	else if (input == "*") {
		return $STAR;
	}
	else if (input == "/") {
		return $SLASH;
	}
	else if (input == "=") {
		return $ASSIGN;
	}
	else if (input == "==") {
		return $EQUAL;
	}
	else if (input == "!=") {
		return $NEQUAL;
	}
	else if (input == ">") {
		return $GT;
	}
	else if (input == ">=") {
		return $GE;
	}
	else if (input == "<") {
		return $LT;
	}
	else if (input == "<=") {
		return $LE;
	}

	return 0;
}

int Lexer::_delimiter(string input)
{
	if (input == ";") {
		return $SEMICOLON;
	}
	else if (input == ",") {
		return $COMMA;
	}
	else if (input == "(") {
		return $LPAREN;
	}
	else if (input == ")") {
		return $RPAREN;
	}
	else if (input == "{") {
		return $LCURLY;
	}
	else if (input == "}") {
		return $RCURLY;
	}

	return 0;
}

void Lexer::analyze(string input)
{
	initializeSymbolList();

	string token;
	iter = input.begin();
	int annotation = 0;

	//the first line
	line = 1;

	for (; iter != input.end(); ++iter) {

		if (*iter == ' ' || *iter == '\t' || *iter == '\n' || *iter == '\r')
		{
			if (*iter == '\n')
				line = line + 1;

			if (*iter == '\n' && annotation == 1)
				annotation = 0;
			continue;
		}

		token += *iter;

		if (token == "#")
		{
			Word word = createWord($FINISH, token);
			wordList.push_back(word);
			token.clear();
		}
		else if (token == "//")
		{
			Word word = createWord($CMT, token);
			wordList.push_back(word);
			token.clear();
			annotation = 1;
		}
		else if (token == "/*")
		{
			Word word = createWord($LCMT, token);
			wordList.push_back(word);
			token.clear();
			annotation = 2;
		}
		else if (token == "*/")
		{
			Word word = createWord($CMT, token);
			wordList.push_back(word);
			token.clear();
			annotation = 0;
		}
		else if (int sym = _delimiter(token)) {
			Word word = createWord((TSymbol)sym, token);
			if (!annotation)
				wordList.push_back(word);
			token.clear();
		}
		else if (int sym = _operator(token)) {
			if (sym == $ASSIGN && *(iter + 1) == '=')
				continue;
			if (sym == $SLASH && (*(iter + 1) == '/' || *(iter + 1) == '*'))
				continue;
			if (sym == $STAR && *(iter + 1) == '/')
				continue;
			if (*(iter + 1) == '=')
				continue;

			Word word = createWord((TSymbol)sym, token);
			if (!annotation)
				wordList.push_back(word);
			token.clear();
		}
		else if (int sym = _keyword(token)) {
			string::iterator pit = iter + 1;
			if (iter + 1 == input.end() || !(_digit(*pit) || _letter(*pit))) {
				Word word = createWord((TSymbol)sym, token);
				if (!annotation)
					wordList.push_back(word);
				token.clear();
			}
		}
		else if (_id(token))
		{
			string::iterator pit = iter + 1;
			if (iter + 1 == input.end() || !(_digit(*pit) || _letter(*pit))) {
				Word word = createWord($ID, token);
				if (!annotation)
					wordList.push_back(word);
				token.clear();
			}
		}
		else if (_num(token))
		{
			string::iterator pit = iter + 1;
			if (iter + 1 == input.end() || !(_digit(*pit))) {
				Word word = createWord($NUM, token);
				if (!annotation)
					wordList.push_back(word);
				token.clear();
			}
		}
	}
}

Word Lexer::createWord(TSymbol symbol, string token)
{
	Word word;
	word.t_symbol = (TSymbol)symbol;
	word.token = token;
	word.wordline = line;
	return word;
}

void Lexer::printResult()
{
	vector<Word>::iterator it = wordList.begin();
	for (; it != wordList.end(); it++)
	{
		cout << it->token << " " << symbolList[it->t_symbol] << ":" << it->wordline << endl;
	}
}
