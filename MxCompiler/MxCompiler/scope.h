#pragma once
#include "pch.h"

class Symbol;
class BuiltInTypeSymbol;
class ClassSymbol;
class FunctionSymbol;
class SymbolType;

class Scope : public std::enable_shared_from_this<Scope>{   // interface class
public:
	virtual std::string getScopeName() = 0;
	virtual std::shared_ptr<Scope> getEnclosingScope() = 0;
	virtual void define(std::shared_ptr<Symbol> symbol) = 0;
	virtual std::shared_ptr<Symbol> resolve(const std::string &id) = 0;
};

class BaseScope : public Scope{
public:
	BaseScope(const std::string &_name, std::shared_ptr<Scope> _enclosingScope)
		:name(_name), enclosingScope(_enclosingScope) {}

	std::string getScopeName()override { return name; }
	std::shared_ptr<Scope> getEnclosingScope() override { return enclosingScope; }

	virtual void define(std::shared_ptr<Symbol> symbol);
	virtual std::shared_ptr<Symbol> resolve(const std::string &id) = 0;

protected:
	std::unordered_map<std::string, std::shared_ptr<Symbol> > symbols;
	std::string name;
	std::shared_ptr<Scope> enclosingScope;
};

class GlobalScope : public BaseScope {
public:
	GlobalScope(const std::string &_name)
		:BaseScope(_name,nullptr){}

	//getters and setters
	std::shared_ptr<BuiltInTypeSymbol> getIntSymbol() { return intSymbol; }
	void setIntSymbol(std::shared_ptr<BuiltInTypeSymbol> _ints) { intSymbol = _ints; }

	std::shared_ptr<BuiltInTypeSymbol> getBoolSymbol() { return boolSymbol; }
	void setBoolSymbol(std::shared_ptr<BuiltInTypeSymbol> _bools) { boolSymbol = _bools; }

	std::shared_ptr<BuiltInTypeSymbol> getVoidSymbol() { return voidSymbol; }
	void setVoidSymbol(std::shared_ptr<BuiltInTypeSymbol> _voids) { voidSymbol = _voids; }

	std::shared_ptr<ClassSymbol> getStringSymbol() { return stringSymbol; }
	void setStringSymbol(std::shared_ptr<ClassSymbol> _stringSymbol) { stringSymbol = _stringSymbol; }

	std::shared_ptr<FunctionSymbol> getArraySizeFuncSymbol() { return arraySizeFuncSymbol; }
	void setArrayFuncSymbol(std::shared_ptr<FunctionSymbol>
		_arraySizeFuncSymbol) { arraySizeFuncSymbol = _arraySizeFuncSymbol; }

	//override functions
	void define(std::shared_ptr<Symbol> symbol) override;
	std::shared_ptr<Symbol> resolve(const std::string &id) override;

	// Other Functions
	std::shared_ptr<SymbolType> resolveType(const std::string &name);
private:
	std::unordered_map<std::string, std::shared_ptr<SymbolType> > types;
	std::shared_ptr<BuiltInTypeSymbol> intSymbol, boolSymbol, voidSymbol;
	std::shared_ptr<ClassSymbol> stringSymbol;
	std::shared_ptr<FunctionSymbol> arraySizeFuncSymbol;
};

class LocalScope : public BaseScope {
public:
	LocalScope(const std::string &_name, std::shared_ptr<Scope>_enclosingScope)
		:BaseScope(_name, _enclosingScope) {}

	//override functions
	// define() is inherit from <BaseScope>
	std::shared_ptr<Symbol> resolve(const std::string &id) override;
};

/*
<ScopedSymbol> in <symbol.h> is also a derivation class of <scope>, 
and related iplementations are in <symbol.cpp>.
*/
