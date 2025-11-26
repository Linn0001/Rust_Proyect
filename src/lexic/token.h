#ifndef TOKEN_H
#define TOKEN_H

#include <string>
#include <ostream>

using namespace std;

class Token {
public:

    enum Type {
        END,       // EOF
        ERR,       // Error léxico

        // Palabras clave
        FN,
        LET,
        MUT,
        RETURN,
        IF,
        ELSE,
        FOR,
        IN,
        RANGE,
        WHILE,
        PRINTLN,
        TRUE,
        FALSE,
        OPERATOR,

        // Tipos de lenguaje
        TYPE_ID,

        // Identificadores y literales
        ID,
        NUM,
        FLOAT,
        STRING,

        // Operadores
        ASSIGN,   // =
        PLUS,     // +
        MINUS,    // -
        MUL,      // *
        DIV,      // /
        POW,      // **

        GT,       // >
        GE,       // >=
        LT,       // <
        LE,       // <=
        EQ,       // ==
        AND,
        OR,

        // Símbolos
        LPAREN,   // (
        RPAREN,   // )
        LBRACE,   // {
        RBRACE,   // }
        COMMA,    // ,
        SEMICOL,  // ;
        COL,     // :
        ARROW     // ->
    };


    // Atributos
    Type type;
    string text;

    // Constructores
    Token(Type type);
    Token(Type type, char c);
    Token(Type type, const string& source, int first, int last);

    // Sobrecarga de operadores de salida
    friend ostream& operator<<(ostream& outs, const Token& tok);
    friend ostream& operator<<(ostream& outs, const Token* tok);
};

#endif // TOKEN_H