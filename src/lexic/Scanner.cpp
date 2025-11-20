#include "Scanner.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cctype>

Scanner::Scanner(const std::string& filename) {
    std::ifstream in(filename);
    if (!in.is_open()) {
        throw std::runtime_error("No se pudo abrir el archivo: " + filename);
    }
    std::ostringstream ss;
    ss << in.rdbuf();
    source = ss.str();
}

void Scanner::skipWhitespaceAndComments() {
    while (pos < source.size()) {
        char c = source[pos];
        if (c == ' ' || c == '\t' || c == '\r') {
            ++pos; ++col;
        } else if (c == '\n') {
            ++pos; ++line; col = 1;
        } else {
            break;
        }
    }
}

Token Scanner::makeToken(Token::Type type, const std::string& lexeme) {
    Token t;
    t.type = type;
    t.lexeme = lexeme;
    t.line = line;
    t.col = col;
    return t;
}

Token Scanner::nextToken() {
    skipWhitespaceAndComments();
    if (pos >= source.size()) {
        return makeToken(Token::END, "");
    }
    char c = source[pos];

    // TODO: implementar bien el scanner (identificadores, números, keywords, etc.)
    // De momento solo soporta fin de archivo.
    ++pos;
    ++col;
    return makeToken(Token::END, "");
}

Token Scanner::peekToken() {
    size_t oldPos = pos;
    int oldLine = line;
    int oldCol = col;
    Token t = nextToken();
    pos = oldPos;
    line = oldLine;
    col = oldCol;
    return t;
}
