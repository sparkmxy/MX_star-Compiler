#include "symbol.h"

std::shared_ptr<SymbolType>
symbolTypeOfNode(Type *node, std::shared_ptr<GlobalScope> globalScope) {
	if (node == nullptr) return globalScope->resolveType("void");
	std::shared_ptr<SymbolType> tp = globalScope->resolveType(node->getIdentifier());
	if (node->isArrayType())
		return std::shared_ptr<ArraySymbol>(new ArraySymbol(tp));
	return tp;
}

std::shared_ptr<Symbol> ClassSymbol::resolveMember(const std::string & id)
{
	auto var = memberVars.find(id);
	if (var != memberVars.end()) return var->second;
	auto func = memberFuncs.find(id);
	return func == memberFuncs.end() ? nullptr : func->second;
}

void ClassSymbol::define(std::shared_ptr<Symbol> symbol)
{
	std::string id = symbol->getSymbolName();
	if (memberVars.find(id) != memberVars.end() ||
		memberFuncs.find(id) != memberFuncs.end())
		throw SemanticError("Duplicated identifier", symbol->getDecl()->Where());
	if (symbol->category() == VAR) {
		memberVars[id] = symbol;
		symbol->setScope(shared_from_this());
	}
	else if (symbol->category() == FUNCTION) {
		memberFuncs[id] = symbol;
		symbol->setScope(shared_from_this());
	}
}

std::shared_ptr<Symbol> ClassSymbol::resolve(const std::string & id)
{
	auto member = resolveMember(id);
	return member == nullptr ? getEnclosingScope()->resolve(id) : member;
}

void FunctionSymbol::define(std::shared_ptr<Symbol> symbol)
{
	if (args.find(symbol->getSymbolName()) != args.end())
		throw SemanticError("Duplicated identifier for argument(s).", symbol->getDecl()->Where());
	args[symbol->getSymbolName()] = symbol;
	symbol->setScope(shared_from_this());
}

std::shared_ptr<Symbol> FunctionSymbol::resolve(const std::string & id)
{
	auto iter = args.find(id);
	if (iter != args.end()) return iter->second;
	return getEnclosingScope()->resolve(id);
}
