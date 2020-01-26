#pragma once
#include "pch.h"
#include "token.h"

class Lexer {

public:
	Lexer(std::istream& _is):is(_is),line(1),col(1),finish(Token(FINISH,defaultPos)){
		tokens = _getTokens();
		keywordsInit();
	}
	~Lexer() {
		for (auto word : words)
			delete word.second;
	}
	std::vector<Token*> getTokens() {
		return tokens;
	}

private:

	static const std::string SYMBOLS;
	static const PosPair defaultPos;
	Token finish;
	std::istream &is;
	std::vector<Token*> tokens;
	std::unordered_map<std::string, Token*> words;
	int col, line;

	std::vector<Token*> _getTokens();

	void skipSpaces();

	Token *nextToken();
	
	Token *scanString();

	Token *scanNumber();

	Token *scanIdentifier();

	Token *scanSymbol();

	bool isNextChar(char c);

	bool isSymbolChar(char ch) const;

	bool isIdentifierChar(char ch) const;

	Token *newWord(const std::string &str, Tag _tag, const Position &st, const Position &ed);

	void keywordsInit();

	Position currentPos() { return Position(line,col); }
};