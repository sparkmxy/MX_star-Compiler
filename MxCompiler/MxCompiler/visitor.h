/*
This file defines the abstract class <Vistor>.
<Vistor> is an interface for iterating an given AST tree.
Usage:
astNode.accept(Visitor);
*/

#pragma once

#include "pch.h"
#include "astnode.h"

class Visitor {
public:
	virtual void visit(Identifier *) {}

	virtual void visit(BasicType *) {}
	virtual void visit(UserDefinedType *) {}
	virtual void visit(ArrayType *) {}


	virtual void visit(StringValue *) {}
	virtual void visit(NumValue *) {}
	virtual void visit(NullValue *) {}
	virtual void visit(BoolValue *) {}
	virtual void visit(IdentifierExpr *) {}
	virtual void visit(FuncCallExpr *) {}
	virtual void visit(MemberFuncCallExpr *) {}
	virtual void visit(NewExpr *) {}
	virtual void visit(UnaryExpr *) {}
	virtual void visit(BinaryExpr *) {}

	virtual void visit(IfStmt *) {}
	virtual void visit(WhileStmt *) {}
	virtual void visit(ForStmt *) {}
	virtual void visit(ExprStmt *) {}
	virtual void visit(VarDeclStmt*) {}
	virtual void visit(ReturnStmt*) {}
	virtual void visit(BreakStmt*) {}
	virtual void visit(ContinueStmt*) {}
	virtual void visit(EmptyStmt *) {}
	virtual void visit(StmtBlock *) {}

	virtual void visit(VarDecl *) {}
	virtual void visit(FunctionDecl *) {}
	virtual void visit(ClassDecl *) {}

	virtual void visit(ProgramAST *) {}
};