#pragma once

#include "pch.h"
#include "astnode.h"
#include "scope.h"

class SymbolType {
public:
	virtual std::string getTypeName() const = 0;
};

class Symbol {
public:
	Symbol(const std::string &_name,
		std::shared_ptr<SymbolType> _type,
		std::shared_ptr<Declaration> _decl) 
		: name(std::move(_name)), type(std::move(_type)), decl(std::move(_decl)) {}
	
	// getters and setters 
	void setScope(std::shared_ptr<Scope> _scope) { scope = std::move(_scope); }
	std::string getSymbolName() const { return name; }
	std::shared_ptr<SymbolType> getType() { return type; }
	std::shared_ptr<Scope> getScope() { return scope; }
	std::shared_ptr<Declaration> getDecl() { return decl; }
	
	enum SymbolCategory
	{
		VAR, FUNCTION, CLASS, BUILTIN
	};

	virtual SymbolCategory category() const = 0;

private:
	std::string name;
	std::shared_ptr<SymbolType> type;
	std::shared_ptr<Scope> scope;
	std::shared_ptr<Declaration> decl;
};


class VarSymbol : public Symbol{
public:
	VarSymbol(const std::string &_name, std::shared_ptr<SymbolType> _type,
		std::shared_ptr<VarDecl> _decl) :Symbol(_name, _type, _decl) {}

	SymbolCategory category() const override { return VAR; }
};

class BuiltInTypeSymbol : public Symbol, public SymbolType {
public:
	BuiltInTypeSymbol(const std::string &_name) : Symbol(_name, nullptr, nullptr) {}

	std::string getTypeName() const override { return getSymbolName(); }

	SymbolCategory category() const override { return BUILTIN; }

	bool compatible() const {
	}
};

class ArraySymbol : public Symbol, public SymbolType {
public:
private:
};

/*Scoped symbols*/
class ScopedSymbol : public Symbol {};

class ClassSymbol : public ScopedSymbol, public SymbolType, public Scope {

};

class FunctionSymbol : public Symbol {

};

 /*Helper function*/
std::shared_ptr<SymbolType> 
SymbolTypeOfNode(Type &node,std::shared_ptr<GlobalScope> globalScope);