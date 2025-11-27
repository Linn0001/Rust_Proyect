#include <iostream>
#include "token.h"

using namespace std;

// Constructores

Token::Token(Type type)
        : type(type), text("") { }

Token::Token(Type type, char c)
        : type(type), text(string(1, c)) { }

Token::Token(Type type, const string& source, int first, int last)
        : type(type), text(source.substr(first, last)) { }

// Sobrecarga de operador <<

// Para Token por referencia
ostream& operator<<(ostream& outs, const Token& tok) {
    switch (tok.type) {

        // Control
        case Token::END:     outs << "TOKEN(END)"; break;
        case Token::ERR:     outs << "TOKEN(ERR, \"" << tok.text << "\")"; break;

            // Palabras clave
        case Token::FN:       outs << "TOKEN(FN, \"" << tok.text << "\")"; break;
        case Token::LET:      outs << "TOKEN(LET, \"" << tok.text << "\")"; break;
        case Token::MUT:      outs << "TOKEN(MUT, \"" << tok.text << "\")"; break;
        case Token::RETURN:   outs << "TOKEN(RETURN, \"" << tok.text << "\")"; break;
        case Token::IF:       outs << "TOKEN(IF, \"" << tok.text << "\")"; break;
        case Token::ELSE:     outs << "TOKEN(ELSE, \"" << tok.text << "\")"; break;
        case Token::FOR:      outs << "TOKEN(FOR, \"" << tok.text << "\")"; break;
        case Token::IN:       outs << "TOKEN(IN, \"" << tok.text << "\")"; break;
        case Token::RANGE:    outs << "TOKEN(RANGE, \"" << tok.text << "\")"; break;
        case Token::WHILE:    outs << "TOKEN(WHILE, \"" << tok.text << "\")"; break;
        case Token::PRINTLN:  outs << "TOKEN(PRINTLN, \"" << tok.text << "\")"; break;
        case Token::TRUE:     outs << "TOKEN(TRUE, \"" << tok.text << "\")"; break;
        case Token::FALSE:    outs << "TOKEN(FALSE, \"" << tok.text << "\")"; break;
        case Token::QMARK:   outs << "TOKEN(QMARK, \"" << tok.text << "\")"; break;

            // Identificadores y literales
        case Token::ID:       outs << "TOKEN(ID, \"" << tok.text << "\")"; break;
        case Token::NUM:   outs << "TOKEN(NUMBER, \"" << tok.text << "\")"; break;
        case Token::FLOAT:    outs << "TOKEN(FLOAT, \"" << tok.text << "\")"; break;
        case Token::STRING:   outs << "TOKEN(STRING, \"" << tok.text << "\")"; break;

            // Operadores
        case Token::ASSIGN:   outs << "TOKEN(ASSIGN, \"" << tok.text << "\")"; break;
        case Token::PLUS:     outs << "TOKEN(PLUS, \"" << tok.text << "\")"; break;
        case Token::MINUS:    outs << "TOKEN(MINUS, \"" << tok.text << "\")"; break;
        case Token::MUL:      outs << "TOKEN(MUL, \"" << tok.text << "\")"; break;
        case Token::DIV:      outs << "TOKEN(DIV, \"" << tok.text << "\")"; break;
        case Token::POW:      outs << "TOKEN(POW, \"" << tok.text << "\")"; break;

        case Token::GT:       outs << "TOKEN(GT, \"" << tok.text << "\")"; break;
        case Token::GE:       outs << "TOKEN(GE, \"" << tok.text << "\")"; break;
        case Token::LT:       outs << "TOKEN(LT, \"" << tok.text << "\")"; break;
        case Token::LE:       outs << "TOKEN(LE, \"" << tok.text << "\")"; break;
        case Token::EQ:       outs << "TOKEN(EQ, \"" << tok.text << "\")"; break;

            // Símbolos
        case Token::LPAREN:   outs << "TOKEN(LPAREN, \"" << tok.text << "\")"; break;
        case Token::RPAREN:   outs << "TOKEN(RPAREN, \"" << tok.text << "\")"; break;
        case Token::LBRACE:   outs << "TOKEN(LBRACE, \"" << tok.text << "\")"; break;
        case Token::RBRACE:   outs << "TOKEN(RBRACE, \"" << tok.text << "\")"; break;
        case Token::COMMA:    outs << "TOKEN(COMMA, \"" << tok.text << "\")"; break;
        case Token::SEMICOL:  outs << "TOKEN(SEMICOL, \"" << tok.text << "\")"; break;
        case Token::COL:     outs << "TOKEN(COL, \"" << tok.text << "\")"; break;
        case Token::ARROW:    outs << "TOKEN(ARROW, \"" << tok.text << "\")"; break;
    }
    return outs;
}

ostream& operator<<(ostream& outs, const Token* tok) {
    if (!tok) return outs << "TOKEN(NULL)";
    return outs << *tok;
}