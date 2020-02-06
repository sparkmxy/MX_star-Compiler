#pragma once

#include "pch.h"
#include "visitor.h"

class SemanticChecker : public Visitor {
public:
	SemanticChecker() {

	}
	bool check();
private:

	//override functions
	void visit(ProgramAST *program) override;
};

