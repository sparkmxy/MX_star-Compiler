#pragma once

#include "pch.h"
#include "visitor.h"

class SemanticChecker : public Visitor {
public:
	SemanticChecker() {

	}
	bool check();
private:
	void visit(ProgramAST *program) override;
};

