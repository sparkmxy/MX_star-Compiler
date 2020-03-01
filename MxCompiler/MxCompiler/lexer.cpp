#include "lexer.h"

const std::string Lexer::SYMBOLS = "+-*/.<>&{}[]()=!|%,;^~";
const PosPair Lexer::defaultPos = std::make_pair(Position(0, 0),Position(0,0));

bool Lexer::isSymbolChar(char ch) const {
	return SYMBOLS.find(ch) != std::string::npos;
};

bool Lexer::isIdentifierChar(char ch) const {
	return isalnum(ch) || ch == '_';
}

std::vector<std::shared_ptr<Token> > Lexer::_getTokens() {
	std::vector<std::shared_ptr<Token> > ret;
	while (true) {
		auto nxt = nextToken();
		if (nxt != nullptr) {
		//	std::cout << nxt->toString() << '\n';
			ret.push_back(nxt);
		}
		else break;
	}
	ret.push_back(finish);
	return ret;
}


std::shared_ptr<Token> Lexer::nextToken() {
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
				if (ch == '\n' || ch == '\r' || ch == EOF) 
					break;
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
    throw SyntaxError(std::string("illeagal character") + ch, currentPos());
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

std::shared_ptr<Token> Lexer::scanString() {
	char delim = is.get();
	auto st = currentPos();
	col++;
	std::string lexeme = "";
	bool escape = false;
	while (true) {
		char ch = is.get();
		if (ch == EOF) throw SyntaxError("Undetermined string.",currentPos());
		if (ch == delim && !escape) break;
		escape = (ch == '\\') && !escape;
		lexeme += ch;  
		col++;
	}
	return newWord(delim + lexeme + delim,ConstString,st,currentPos());
}

std::shared_ptr<Token> Lexer::scanNumber() {  // for int
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
	return std::make_shared<Number>(val,st,currentPos());
}

std::shared_ptr<Token> Lexer::scanIdentifier() {
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
		col++;
	}
	return newWord(lexeme,ID,st,currentPos());
}

std::shared_ptr<Token> Lexer::scanSymbol() {
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
		col++;
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
			throw SyntaxError("Lexer error: unmatched symbol.",currentPos());
		}
	}
	return nullptr;
}

bool Lexer::isNextChar(char c) {
	if (is.get() == c) return true;
	is.unget();
	return false;
}

std::shared_ptr<Token> Lexer::newWord(const std::string &str, Tag _tag, const Position &st, const Position &ed) {
	auto iter = keywords.find(str);
	if (iter != keywords.end())   // this is a keyword
		return std::make_shared<Word>(iter->second->toString(),iter->second->tag(), st, ed);
	return std::make_shared<Word>(str, _tag,st,ed);
}

void Lexer::keywordsInit() {
	keywords["bool"] = std::make_shared<Word>("bool",Bool, defaultPos);
	keywords["int"] = std::make_shared<Word>("int",Int, defaultPos);
	keywords["void"] = std::make_shared<Word>("void",Void, defaultPos);
	keywords["string"] = std::make_shared<Word>("string",String, defaultPos);
	keywords["null"] = std::make_shared<Word>("null",Null, defaultPos);
	keywords["true"] = std::make_shared<Word>("true",True, defaultPos);
	keywords["false"] = std::make_shared<Word>("false",False, defaultPos);
	keywords["if"] = std::make_shared<Word>("if",If, defaultPos);
	keywords["else"] = std::make_shared<Word>("else",Else, defaultPos);
	keywords["for"] = std::make_shared<Word>("for",For, defaultPos);
	keywords["while"] = std::make_shared<Word>("while",While, defaultPos);
	keywords["break"] = std::make_shared<Word>("break",Break, defaultPos);
	keywords["continue"] = std::make_shared<Word>("continue",Continue, defaultPos);
	keywords["return"] = std::make_shared<Word>("return",Ret, defaultPos);
	keywords["new"] = std::make_shared<Word>("new",New, defaultPos);
	keywords["class"] = std::make_shared<Word>("class",Class, defaultPos);
	keywords["this"] = std::make_shared<Word>("this",This, defaultPos);
}