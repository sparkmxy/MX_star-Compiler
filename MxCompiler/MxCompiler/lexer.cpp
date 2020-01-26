#include "lexer.h"

const std::string Lexer::SYMBOLS = "+-*/.<>&{}[]()=!|%,;";
const PosPair Lexer::defaultPos = std::make_pair(Position(0, 0),Position(0,0));

bool Lexer::isSymbolChar(char ch) const {
	return SYMBOLS.find(ch) != std::string::npos;
};

bool Lexer::isIdentifierChar(char ch) const {
	return isalnum(ch) || ch == '_';
}

std::vector<Token*> Lexer::_getTokens() {
	std::vector<Token*> ret;
	while (true) {
		auto nxt = nextToken();
		if (nxt != nullptr) {
		//	std::cout << nxt->toString() << '\n';
			ret.push_back(nxt);
		}
		else break;
	}
	ret.push_back(&finish);
	return ret;
}


Token *Lexer::nextToken() {
	skipSpaces();
	char ch = is.get();
	if(ch == '/') { 
		// ignore comment
		ch = is.get();
		if (ch == '/') {  // comment in a line
			col += 2;
			while (true){
				ch = is.get();
				if (ch == '\n')
					col = 1, line++;
				else col++;
				if (ch == '\n' || ch == '\r' || ch == EOF) break;
			}
			return nextToken();
		}
		else if (ch == '*') { // long comment
			char prev = EOF;
			col += 2;
			while (true) {
				ch = is.get();
				if (ch == '\n')
					col = 1, line++;
				else col++;
				if (ch == EOF || (prev == '*' && ch == '/')) 
					break;
				prev = ch;
			}
			return nextToken();
		}
		if (ch != EOF) is.unget();
		ch = '/';
	}
	if(ch == EOF) return nullptr;
	if (ch == '"' || ch == '\'') {
		is.unget();
		return scanString();
	}
	if (isdigit(ch)) {
		is.unget();
		return scanNumber();
	}
	if (isSymbolChar(ch)) {
		is.unget();
		return scanSymbol();
	}
	if (isalpha(ch) || ch == '_') {
		is.unget();
		return scanIdentifier();
	}
    throw Error("illegal character: " + ch);
}

void Lexer::skipSpaces() {
	while (true) {
		char ch = is.get();
		if (ch == EOF) return;
		if (!isspace(ch)) {
			is.unget();
			return;
		}
		if (ch == '\n')
			col = 1, line++;
		else col++;
	}
}

Token *Lexer::scanString() {
	char delim = is.get();
	auto st = currentPos();
	col++;
	std::string lexeme = "";
	bool escape = false;
	while (true) {
		char ch = is.get();
		if (ch == EOF) throw Error("Undetermined string.");
		if (ch == delim && !escape) break;
		escape = (ch == '\\') && !escape;
		lexeme += ch;  
		col++;
	}
	return newWord(delim + lexeme + delim,ConstString,st,currentPos());
}

Token *Lexer::scanNumber() {  // for int
	std::string lexeme;
	char ch = is.get();
	auto st = currentPos();
	int val = 0;
	do {
		val = val * 10 + ch - '0';
		col++;
		ch = is.get();
	} while (isdigit(ch));
	is.unget();
	return new Number(val,st,currentPos());
}

Token *Lexer::scanIdentifier() {
	std::string lexeme;
	auto st = currentPos();
	while (true) {
		int ch = is.get();
		if (ch == EOF) break;
		if (!isIdentifierChar(ch)) {
			is.unget();
			break;
		}
		lexeme += ch;
	}
	return newWord(lexeme,ID,st,currentPos());
}

Token *Lexer::scanSymbol() { 
	char ch = is.get();
	auto st = currentPos();
	col++;
	if (ch == '&') {
		ch = is.get();
		if (ch == '&') {
			col++;
			return newWord("&&",And,st,currentPos());
		}
		else {
			is.unget();
			return newWord("&", BitAnd, st, currentPos());
		}
	}
	else if (ch == '|') {
		ch = is.get();
		if (ch == '|') {
			col++;
			return newWord("||", Or, st, currentPos());
		}
		else {
			is.unget();
			return newWord("|", BitOr, st, currentPos());
		}
	}
	else if (ch == '=') {
		ch = is.get();
		if (ch == '=') {
			col++;
			return newWord("==", Equal, st, currentPos());
		}
		else {
			is.unget();
			return newWord("=", Assign, st, currentPos());
		}
	}
	else if (ch == '<') {
		ch = is.get();
		if (ch == '<') {
			col++;
			return newWord("<<", LShift, st, currentPos());
		}
		else if (ch == '=') {
			return newWord("<=", Leq, st, currentPos());
			col++;
		}
		else {
			is.unget();
			return newWord("<",Less, st, currentPos());
		}
	}
	else if (ch == '>'){
		ch = is.get();
		if (ch == '>') {
			col++;
			return newWord(">>", RShift, st, currentPos());
		}
		else if (ch == '=') {
			col++;
			return newWord(">=", Geq, st, currentPos());
		}
		else {
			is.unget();
			return newWord(">", Greater, st, currentPos());
		}
	}
	else if (ch == '!') {
		ch = is.get();
		if (ch == '=') {
			col++;
			return newWord("!=", Neq, st, currentPos());
		}
		is.unget();
		return newWord("!", Not, st, currentPos());
	}
	else if (ch == '-') {
		ch = is.get();
		if (ch == '-') {
			col++;
			return newWord("--", Dec, st, currentPos());
		}
		is.unget();
		return newWord("-",Minus, st, currentPos());
	}
	else if (ch == '+') {
		ch = is.get();
		if (ch == '+') {
			col++;
			return newWord("++", Inc, st, currentPos());
		}
		is.unget();
		return newWord("+", Add, st, currentPos());
	}
	else {
		switch (ch)
		{
		case '~': return newWord("~", Inverse, st, currentPos());
		case '^': return newWord("^", Xor, st, currentPos());
		case ',': return newWord(",", Comma, st, currentPos());
		case ';': return newWord(";", Semicolon, st, currentPos());
		case '.': return newWord(".", Domain, st, currentPos());
		case '(': return newWord("(", LeftBracket, st, currentPos());
		case ')': return newWord(")", RightBracket, st, currentPos());
		case '[': return newWord("[", LeftIndex, st, currentPos());
		case ']': return newWord("]", RightIndex, st, currentPos());
		case '*': return newWord("*", Multiply, st, currentPos());
		case '/': return newWord("/", Divide, st, currentPos());
		case '%': return newWord("%", Mod, st, currentPos());
		case '{': return newWord("{", LeftBrace, st, currentPos());
		case '}': return newWord("}", RightBrace, st, currentPos());
		default:
			throw Error("Lexer error: unmatched symbol.");
		}
	}
	return nullptr;
}

bool Lexer::isNextChar(char c) {
	if (is.get() == c) return true;
	is.unget();
	return false;
}

Token *Lexer::newWord(const std::string &str, Tag _tag, const Position &st, const Position &ed) {
	if (words.find(str) != words.end()) return words[str];
	auto ret = new Word(str, _tag,st,ed);
	words[str] = ret;
	return ret;
}

void Lexer::keywordsInit() {
	words["bool"] = new Word("bool", Bool, defaultPos);
	words["int"] = new Word("num", Int, defaultPos);
	words["void"] = new Word("num", Void, defaultPos);
	words["string"] = new Word("string", String, defaultPos);
	words["null"] = new Word("null", Null, defaultPos);
	words["true"] = new Word("true", True, defaultPos);
	words["false"] = new Word("false", False, defaultPos);
	words["if"] = new Word("if", If, defaultPos);
	words["else"] = new Word("else", Else, defaultPos);
	words["for"] = new Word("for", For, defaultPos);
	words["while"] = new Word("while", While, defaultPos);
	words["break"] = new Word("break", Break, defaultPos);
	words["continue"] = new Word("continue", Continue, defaultPos);
	words["return"] = new Word("return", Return, defaultPos);
	words["new"] = new Word("new", New, defaultPos);
	words["class"] = new Word("class", Class, defaultPos);
	words["this"] = new Word("this", This, defaultPos);
}