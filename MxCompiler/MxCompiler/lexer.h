
#pragma once
#include "pch.h"

#include "token.h"

class Lexer {

public:
	Lexer(std::istream& _is):is(_is),line(1),col(1){
		finish = std::shared_ptr<Token>(new Word("finish" ,FINISH, defaultPos));
		keywordsInit();
		tokens = _getTokens();
	}
	std::vector< std::shared_ptr<Token> > &getTokens() {
		return tokens;
	}

private:

	static const std::string SYMBOLS;
	static const PosPair defaultPos;
	std::shared_ptr<Token> finish;
	std::istream &is;
	std::vector<std::shared_ptr<Token> > tokens;
	std::map<std::string, std::shared_ptr<Token> >keywords;
	int col, line;

	std::vector<std::shared_ptr<Token> >  _getTokens();



	void skipSpaces();

	std::shared_ptr<Token> nextToken();
	
	std::shared_ptr<Token> scanString();

	std::shared_ptr<Token> scanNumber();

	std::shared_ptr<Token> scanIdentifier();

	std::shared_ptr<Token> scanSymbol();

	bool isNextChar(char c);

	bool isSymbolChar(char ch) const;

	bool isIdentifierChar(char ch) const;

	std::shared_ptr<Token> 
		newWord(const std::string &str, Tag _tag, const Position &st, const Position &ed);

	void keywordsInit();

	Position currentPos() { return Position(line,col); }
};