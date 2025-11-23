#pragma once
#include <string>
#include <vector>

struct Token {
    enum Type {
        END,
        IDENT,
        INT_LITERAL,
        FLOAT_LITERAL,
        KW_FN,
        KW_LET,
        KW_MUT,
        KW_BOOL,
        KW_I32,
        KW_I64,
        KW_U32,
        KW_F32,
        KW_IF,
        KW_ELSE,
        KW_WHILE,
        KW_RETURN,
        OP_PLUS,
        OP_MINUS,
        OP_MUL,
        OP_DIV,
        OP_ASSIGN,
        OP_EQ,
        OP_NEQ,
        OP_LT,
        OP_GT,
        OP_LE,
        OP_GE,
        OP_AND,
        OP_OR,
        QUESTION,
        COLON,
        LPAREN,
        RPAREN,
        LBRACE,
        RBRACE,
        COMMA,
        SEMICOLON
    } type;
    std::string lexeme;
    int line;
    int col;
};

class Scanner {
public:
    explicit Scanner(const std::string& filename);
    Token nextToken();
    Token peekToken();
private:
    std::string source;
    size_t pos {0};
    int line {1};
    int col {1};
    Token current;
    void skipWhitespaceAndComments();
    Token makeToken(Token::Type type, const std::string& lexeme);
};
