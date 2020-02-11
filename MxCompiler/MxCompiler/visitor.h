/*
This file defines the abstract class <Vistor>.
<Vistor> is an interface for iterating an given AST tree.
Usage:
astNode.accept(Visitor);
*/

#pragma once

#include "pch.h"

class Identifier;
class BasicType;
class UserDefinedType;
class ArrayType;

class StringValue;
class NumValue;
class NullValue;
class BoolValue;
class IdentifierExpr;
class FuncCallExpr;
class NewExpr;
class UnaryExpr;
class BinaryExpr;
class MemberFuncCallExpr;
class ClassMemberExpr;
class ThisExpr;

class IfStmt;
class WhileStmt;
class ForStmt;
class ExprStmt;
class VarDeclStmt;
class ReturnStmt;
class BreakStmt;
class ContinueStmt;
class EmptyStmt;
class StmtBlock;

class FunctionDecl;
class ClassDecl;
class MultiVarDecl;

class ProgramAST;

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
	virtual void visit(ClassMemberExpr *) {}
	virtual void visit(ThisExpr *) {}

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

	virtual void visit(FunctionDecl *) {}
	virtual void visit(ClassDecl *) {}
	virtual void visit(MultiVarDecl *) {}

	virtual void visit(ProgramAST *) {}
};