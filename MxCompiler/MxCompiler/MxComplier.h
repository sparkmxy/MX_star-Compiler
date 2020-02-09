
#pragma once

#include "pch.h"
#include "lexer.h"
#include "parser.h"
#include "environment.h"

class MxComplier {
public:
	MxComplier(std::string _fileName):fileName(_fileName){}
	
	void getCode();
	void parse();
	void buildAST();
	void semanticCheck();
private:

	std::ifstream src;
	std::string fileName;

	std::shared_ptr<Lexer> lexer;
	std::shared_ptr<Parser> parser;
	std::shared_ptr<Environment> environment;

	std::vector<std::shared_ptr<Token> > tokens;
	std::shared_ptr<ProgramAST> ast;
};