#include "scope.h"
#include "symbol.h"
#include "astnode.h"

void BaseScope::define(std::shared_ptr<Symbol> symbol)
{
	if (symbols.find(symbol->getSymbolName()) != symbols.end())
		throw SemanticError("duplicated identifier", symbol->getDecl()->Where());
	symbols[symbol->getSymbolName()] = symbol;
	symbol->setScope(shared_from_this());
}

void GlobalScope::define(std::shared_ptr<Symbol> symbol)
{
	if (symbol->category() == Symbol::BUILTIN) {   // definition of builtin symbol
		types[symbol->getSymbolName()] = std::static_pointer_cast<BuiltInTypeSymbol>(symbol);
		symbol->setScope(shared_from_this());
	}
	else {
		if (symbol->category() == Symbol::CLASS) { // Class definitions need to register in <types> in particular.
			if (types.find(symbol->getSymbolName()) != types.end())
				throw SemanticError("duplicated identifier", symbol->getDecl()->Where());
			types[symbol->getSymbolName()] = std::static_pointer_cast<ClassSymbol>(symbol);
		}
		BaseScope::define(symbol);
	}
}

std::shared_ptr<Symbol> GlobalScope::resolve(const std::string &id)
{
	auto iter = symbols.find(id);
	return iter == symbols.end() ? nullptr : iter->second;
}

std::shared_ptr<SymbolType> GlobalScope::resolveType(const std::string &id)
{
	auto iter = types.find(id);
	return iter == types.end() ? nullptr : iter->second;
}

std::shared_ptr<Symbol> LocalScope::resolve(const std::string & id)
{
	auto iter = symbols.find(id);
	if (iter != symbols.end()) return iter->second;
	return enclosingScope->resolve(id);
}
