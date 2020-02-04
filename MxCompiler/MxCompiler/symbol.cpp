#include "symbol.h"

std::shared_ptr<SymbolType>
SymbolTypeOfNode(Type &node, std::shared_ptr<GlobalScope> globalScope) {
	std::shared_ptr<SymbolType> tp = globalScope.resolveType();
	if(node.isArrayType()) 
		return std::shared_ptr<ArraySymbol>(new ArraySymbol())
}