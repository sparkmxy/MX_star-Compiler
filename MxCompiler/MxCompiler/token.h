#pragma once

#include "pch.h"
#include "position.h"

enum Tag
{
	Bool, Int, String, Null, // basic types
	Void,
	True, False,
	If, Else,
	For, While, Break, Continue,
	Ret,
	New,
	Class, This,					// class 
	/* operators */
	Add, Minus, Multiply, Divide, Mod,
	Less, Greater, Leq, Geq, Neq, Equal,
	And, Or, Not,
	RShift, LShift, Inverse, BitOr, BitAnd, Xor,
	Assign, Inc, Dec, Domain,
	LeftIndex, RightIndex,
	LeftBracket, RightBracket, LeftBrace, RightBrace,
	/*Others*/
	ID, Comma, Semicolon,
	ConstString, Num,
	FINISH
};

class Token
{
public:
	Token(Tag t,const Position &st, const Position &ed) 
		:_tag(t),_pos(std::make_pair(st,ed)){}
	Token(Tag t, const PosPair &__pos)
		:_tag(t), _pos(__pos) {}
	virtual ~Token() = default;
	Tag tag() { return _tag; }
	virtual std::string toString() = 0;
	virtual std::string getLexeme() = 0;
	PosPair &pos() { return _pos; }
private:
	Tag _tag;
	PosPair _pos;
};

class Number: public Token
{
public:
	Number(int _v, const Position &st, const Position &ed) :Token(Num,st,ed),val(_v) {};
	Number(int _v, const PosPair &p) :Token(Num, p), val(_v) {};
	int value() { return val; }
	std::string getLexeme() override { return std::to_string(val); }
	std::string toString() override{
		return "NUM("+std::to_string(val)+")" + " @ " + pos().first.toString(); 
	}
private:
	int val;
};

class Word: public Token
{
public:
	Word(const std::string &str,Tag _tag, const Position &st, const Position &ed)
		:Token(_tag,st,ed),lexeme(str) {}
	Word(const std::string &str, Tag _tag, const PosPair &p)
		:Token(_tag,p), lexeme(str) {}
	std::string toString() override { 
		return lexeme + " @ " + pos().first.toString();
	}
	std::string getLexeme()override {
		return lexeme;
	}

private:
	std::string lexeme;
};