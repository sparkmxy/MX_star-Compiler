#include "symbol.h"
#include "astnode.h"

std::shared_ptr<SymbolType>
symbolTypeOfNode(Type *node, std::shared_ptr<GlobalScope> globalScope) {
	if (node == nullptr) return globalScope->resolveType("void");
	if (node->isArrayType())
		return std::make_shared<ArraySymbol>(
			symbolTypeOfNode(reinterpret_cast<ArrayType *>(node)->getBaseType().get(),
			globalScope
			));
	return globalScope->resolveType(node->getIdentifier());
}

bool isLeagalName(const std::string & id)
{
	static std::set<std::string> ignore = { "__bootstrap" };
	return ignore.find(id) != ignore.end() || id[0] != '_';
}


std::shared_ptr<Symbol> ClassSymbol::resolveMember(const std::string & id)
{
	auto var = memberVars.find(id);
	if (var != memberVars.end()) return var->second;
	auto func = memberFuncs.find(id);
	return func == memberFuncs.end() ? nullptr : func->second;
}

bool ClassSymbol::compatible(std::shared_ptr<SymbolType> type)
{
	if (this->getTypeName() == "string") {
		if (type->isNull()) return false;
		return type->getTypeName() == "string";
	}
	if (type->isNull()) return true;
	return this->getTypeName() == type->getTypeName();
}

void ClassSymbol::define(std::shared_ptr<Symbol> symbol)
{
	std::string id = symbol->getSymbolName();
	if (!isLeagalName(id))
		throw SemanticError("illegal identifier: " + id, symbol->getDecl()->Where());

	if (!isLeagalName(id))
		throw SemanticError("illegal identifier: " + id,symbol->getDecl()->Where());

	if (memberVars.find(id) != memberVars.end() ||
		memberFuncs.find(id) != memberFuncs.end())
		throw SemanticError("Duplicated identifier", symbol->getDecl()->Where());
	if (symbol->category() == VAR) {
		memberVars[id] = std::static_pointer_cast<VarSymbol>(symbol);
		symbol->setScope(shared_from_this());
		std::static_pointer_cast<VarSymbol>(symbol)->setOffset(size);
		if (symbol->getType()->isUserDefinedType()) size += 4;
		else size += symbol->getType()->getSize();
	}
	else if (symbol->category() == FUNCTION) {
		memberFuncs[id] = std::static_pointer_cast<FunctionSymbol>(symbol);
		symbol->setScope(shared_from_this());
	}
}

std::shared_ptr<Symbol> ClassSymbol::resolve(const std::string & id)
{
	auto member = resolveMember(id);
	return member == nullptr ? getEnclosingScope()->resolve(id) : member;
}

void ClassSymbol::setConstructor(std::shared_ptr<FunctionSymbol> _constructor)
{
	constructor = _constructor;
}

void FunctionSymbol::define(std::shared_ptr<Symbol> symbol)
{
	auto id = symbol->getSymbolName();
	if (!isLeagalName(id))
		throw SemanticError("illegal identifier: " + id, symbol->getDecl()->Where());

	if (symbol->category() != VAR)
		throw SemanticError("argument can only be variable",symbol->getDecl()->Where());
	if (args.find(symbol->getSymbolName()) != args.end())
		throw SemanticError("Duplicated identifier for argument(s).", symbol->getDecl()->Where());

	symbol->setScope(shared_from_this());
	args[id] = std::static_pointer_cast<VarSymbol>(symbol);
	formalArgs.push_back(std::static_pointer_cast<VarSymbol>(symbol));
}

std::shared_ptr<Symbol> FunctionSymbol::resolve(const std::string & id)
{
	auto iter = args.find(id);
	if (iter != args.end()) return iter->second;
	return getEnclosingScope()->resolve(id);
}

bool ArraySymbol::compatible(std::shared_ptr<SymbolType> type)
{
	if (type->isNull()) return true; // nullptr stands for <null>
	if (!type->isArrayType()) return false;
	return std::static_pointer_cast<ArraySymbol>(type)->getBaseType()->compatible(this->baseType);
}

bool BuiltInTypeSymbol::compatible(std::shared_ptr<SymbolType> type)
{
	if (type == nullptr) return false;
	return this->getTypeName() == type->getTypeName();
}

