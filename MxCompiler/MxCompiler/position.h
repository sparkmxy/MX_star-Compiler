#pragma once
#include "pch.h"
class Position {
public:
	Position() = default;
	Position(int _line, int _col) :line(_line), col(_col) {}
	
	bool operator == (const Position &rhs) const {
		return line == rhs.line && col == rhs.col;
	}
	bool operator != (const Position &rhs) const {
		return !this->operator==(rhs);
	}

	std::string toString() const {
		return "line: " + std::to_string(line) + " col: " + std::to_string(col);
	}
private:
	int line, col;
};

using PosPair = std::pair<Position, Position>;