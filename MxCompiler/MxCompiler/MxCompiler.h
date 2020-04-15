
#pragma once

#include "pch.h"
#include "lexer.h"
#include "parser.h"
#include "environment.h"
#include "IR.h"
#include "IR_Generator.h"
#include "ir_printer.h"

class MxCompiler {
public:
	MxCompiler(std::string _fileName):fileName(_fileName){}
	
	void complie();
	void semantic();
	void printIR(std::ostream &os = std::cerr);

private:
	void semanticCheck();
	void getCode();
	void parse();
	void buildAST();
	void generateIR();

	/*Input file*/
	std::ifstream src;
	std::string fileName;

	/*IR constructors*/
	std::shared_ptr<Lexer> lexer;
	std::shared_ptr<Parser> parser;
	std::shared_ptr<Environment> environment;
	std::shared_ptr<IR_Generator> irGenerator;

	/*Data structures for IR*/
	std::vector<std::shared_ptr<Token> > tokens;
	std::shared_ptr<ProgramAST> ast;
	std::shared_ptr<IR> ir;
};