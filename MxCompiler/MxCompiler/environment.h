#pragma once
#include "pch.h"
#include "semanticChecker.h"
#include "symbolTable.h"
#include "ClassDeclVisitor.h"
#include "GlobalFuncAndClsDecl.h"

class Environment {
public:
	Environment(std::shared_ptr<ProgramAST> _ast) 
		:ast(_ast){
		global = std::make_shared<GlobalScope>("NMSL");
		intSymbol = std::make_shared<BuiltInTypeSymbol>("int");
		boolSymbol = std::make_shared<BuiltInTypeSymbol>("bool");
		voidSymbol = std::make_shared<BuiltInTypeSymbol>("void");
		stringSymbol = std::make_shared<ClassSymbol>("string", nullptr, global);
		
		builtinTypeInit();
		builtinFuncInit();
		bootstrapFuncInit();
	}

	std::shared_ptr<GlobalScope> globalScope() { return global; }
	void semanticCheck();
private:
	std::shared_ptr<ProgramAST> ast;

	std::shared_ptr<ClassDeclVisitor> clsDeclVistor;
	std::shared_ptr<GlobalFuncAndClsVisitor> glbFuncAndClsVistor;
	std::shared_ptr<SymbolTable> symbolTable;
	std::shared_ptr<SemanticChecker> semanticChecker;
	
	/*Helper functions*/
	void bootstrapFuncInit();
	void builtinTypeInit();
	void builtinFuncInit();
	void buildSymbolTable();
	std::shared_ptr<GlobalScope> global;
	std::shared_ptr<BuiltInTypeSymbol> intSymbol, boolSymbol, voidSymbol;
	std::shared_ptr<ClassSymbol> stringSymbol;
	std::shared_ptr<FunctionSymbol> arraySizeFuncSymbol;
};