
#pragma once

#include "pch.h"
#include "lexer.h"
#include "parser.h"
#include "environment.h"
#include "IR.h"
#include "IR_Generator.h"
#include "globalvarResolver.h"
#include "ir_printer.h"
#include "SSAConstructor.h"
#include "ir_printer.h"
#include "Optimizer.h"
#include "RISCVcodegen.h"

class MxCompiler {
public:
	MxCompiler(std::string _fileName):fileName(_fileName){}
	
	void compile();
	void semantic();
	void printIR(std::ostream &os = std::cerr);
	void emitCode(std::ostream &os = std::cerr);

private:
	void semanticCheck();
	void getCode();
	void parse();
	void buildAST();
	void generateIR();
	void optimize();
	void codegen();

	/*Input file*/
	std::ifstream src;
	std::string fileName;

	/*IR constructors*/
	std::shared_ptr<Lexer> lexer;
	std::shared_ptr<Parser> parser;
	std::shared_ptr<Environment> environment;
	std::shared_ptr<IR_Generator> irGenerator;
	std::shared_ptr<Optimizer> opt;
	std::shared_ptr<RISCVCodeGenerator> codeGenerator;

	/*Data structures for IR*/
	std::vector<std::shared_ptr<Token> > tokens;
	std::shared_ptr<ProgramAST> ast;
	std::shared_ptr<IR> ir;
};