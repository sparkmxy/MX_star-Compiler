#pragma once
#include "pch.h"
#include "scope.h"
class Declaration;

/*Symbol type*/
class SymbolType {  //interface class
public:
	virtual std::string getTypeName() const = 0;
	virtual bool compatible(std::shared_ptr<SymbolType> type) = 0;

	virtual bool isBulitInType() { return false; }
	virtual bool isArrayType() { return false; }
	virtual bool isUserDefinedType() { return false; }
	virtual bool isNull() { return false; }
};

class ArraySymbol : public SymbolType {
public:
	ArraySymbol(std::shared_ptr<SymbolType> _base) :baseType(_base) {}

	std::string getTypeName() const override { return baseType->getTypeName() + "_array"; }
	std::shared_ptr<SymbolType> getBaseType() { return baseType; }
	bool compatible(std::shared_ptr<SymbolType> type) override;
	bool isArrayType() override { return true; }
private:
	std::shared_ptr<SymbolType> baseType;
};

/*Symbol*/

class Symbol {
public:
	Symbol(const std::string &_name,std::shared_ptr<SymbolType> _type, Declaration *_decl) 
		: name(_name), type(_type), decl(_decl) {}
	
	enum SymbolCategory
	{
		VAR, FUNCTION, CLASS, BUILTIN,
		FUCK
	};

	// getters and setters 
	void setScope(std::shared_ptr<Scope> _scope) { scope = std::move(_scope); }
	std::string getSymbolName() const { return name; }
	std::shared_ptr<SymbolType> getType() { return type; }
	std::shared_ptr<Scope> getScope() { return scope; }
	Declaration *getDecl() { return decl; }
	
	/*Category : what is this symbol? variable, function, class or builtin type*/


	virtual SymbolCategory category() const = 0;

private:
	std::string name;
	std::shared_ptr<SymbolType> type;
	std::shared_ptr<Scope> scope;
	Declaration * decl;
};


class VarSymbol : public Symbol{
public:
	VarSymbol(const std::string &_name, std::shared_ptr<SymbolType> _type,
		Declaration *_decl) :Symbol(_name, _type, _decl) {}

	SymbolCategory category() const override { return VAR; }
};

class BuiltInTypeSymbol : public Symbol, public SymbolType {
public:
	BuiltInTypeSymbol(const std::string &_name) : Symbol(_name, nullptr, nullptr) {}

	std::string getTypeName() const override { return getSymbolName(); }
	bool compatible(std::shared_ptr<SymbolType> type);

	bool isBulitInType() override{ return true; }

	SymbolCategory category() const override { return BUILTIN; }
};

/*Scoped symbols*/
class ScopedSymbol : public Symbol,public Scope{
public:
	ScopedSymbol(const std::string &_name, std::shared_ptr<SymbolType> _type,
		Declaration *_decl, std::shared_ptr<Scope> _enclosingScope) 
		:Symbol(_name, _type, _decl),enclosingScope(_enclosingScope){}
	
	/*override functions*/
	std::string getScopeName() override { return getSymbolName(); }
	std::shared_ptr<Scope>getEnclosingScope() override { return enclosingScope; }

	virtual void define(std::shared_ptr<Symbol> symbol) = 0;
	virtual std::shared_ptr<Symbol> resolve(const std::string &id) = 0;

	virtual SymbolCategory category() const = 0;
private:
	std::shared_ptr<Scope> enclosingScope;
};

class ClassSymbol : public ScopedSymbol, public SymbolType {
public:
	ClassSymbol(const std::string &_name, 
		Declaration*_decl, std::shared_ptr<Scope> _enclosingScope)
		:ScopedSymbol(_name, nullptr, _decl,_enclosingScope){}

	std::shared_ptr<Symbol> resolveMember(const std::string &id);

	/*override funtions*/
	std::string getTypeName() const override { return getSymbolName(); }

	SymbolCategory category() const override{ return CLASS; }

	bool compatible(std::shared_ptr<SymbolType> type) override;
	bool isUserDefinedType() override { return true; }

	void define(std::shared_ptr<Symbol> symbol) override;
	std::shared_ptr<Symbol> resolve(const std::string &id) override;

	std::shared_ptr<FunctionSymbol> getConstructor() { return constructor; }
	void setConstructor(std::shared_ptr<FunctionSymbol> _constructor) { constructor = _constructor; }
private:
	std::unordered_map<std::string, std::shared_ptr<VarSymbol> >  memberVars;
	std::unordered_map<std::string, std::shared_ptr<FunctionSymbol> >  memberFuncs;
	std::shared_ptr<FunctionSymbol> constructor;
};

class FunctionSymbol : public ScopedSymbol {
public:
	FunctionSymbol(const std::string &_name, std::shared_ptr<SymbolType> _type,
		Declaration *_decl, std::shared_ptr<Scope> _enclosingScope)
		:ScopedSymbol(_name, _type, _decl,_enclosingScope){}

	/*override functions*/
	SymbolCategory category() const override { return FUNCTION; }

	void define(std::shared_ptr<Symbol> symbol) override;
	std::shared_ptr<Symbol> resolve(const std::string &id) override;
private:
	std::unordered_map<std::string, std::shared_ptr<VarSymbol> >  args;
};

class NullTypeSymbol : public SymbolType {
	bool compatible(std::shared_ptr<SymbolType> type) override { return false; }
	bool isNull() override { return true; }
	std::string getTypeName() const override { return "null"; }
};

