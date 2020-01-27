#pragma once
/*
This file implements abstract class <Vistor>.
*/
#include "pch.h"
#include "astnode.h"

class Visitor {

public:
	virtual void visit(Identifier &) const {}

	virtual void visit(BasicType &) const {}
	virtual void visit(UserDefinedType &) const {}
	virtual void visit(ArrayType &) const {}


	virtual void visit(StringValue &) const {}
	virtual void visit(NumValue &) const {}
	virtual void visit(NullValue &) const {}
	virtual void visit(BoolValue &) const {}
	virtual void visit(IdentifierExpr &) const {}
	virtual void visit(FuncCallExpr &) const {}
	virtual void visit(MemberFuncCallExpr &) const {}
	virtual void visit(NewExpr &) const {}
	virtual void visit(UnaryExpr &) const {}
	virtual void visit(BinaryExpr &) const {}

	virtual void visit(IfStmt &) const {}
	virtual void visit(WhileStmt &) const {}
	virtual void visit(ForStmt &) const {}
	virtual void visit(ExprStmt &) const {}
	virtual void visit(VarDeclStmt&) const {}
	virtual void visit(ReturnStmt&) const {}
	virtual void visit(BreakStmt&) const {}
	virtual void visit(ContinueStmt&) const {}
	virtual void visit(EmptyStmt &) const {}
	virtual void visit(StmtBlock &) const {}

	virtual void visit(GlobalVarDecl &) const {}
	virtual void visit(FunctionDecl &) const {}
	virtual void visit(ClassDecl &) const {}
};