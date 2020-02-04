#pragma once

#include "pch.h"
#include "symbol.h"

class Scope {
public:
	virtual std::string getScopeName() = 0;
	virtual std::shared_ptr<Scope> getEnclosingScope() = 0;
	virtual void define() = 0;
	virtual std::shared_ptr<Symbol> resolve(const std::string &id) = 0;
};

class GlobalScope : public Scope {

};

class LocalScope : public Scope {


};
