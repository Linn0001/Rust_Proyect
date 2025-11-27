#include <iostream>
#include <cstring>
#include <fstream>
#include "token.h"
#include "scanner.h"

using namespace std;

// Constructor
Scanner::Scanner(const char* s) : input(s), first(0), current(0) {}

// Función auxiliar
bool is_white_space(char c) {
    return c == ' ' || c == '\n' || c == '\r' || c == '\t';
}

// nextToken: obtiene el siguiente token
Token* Scanner::nextToken() {
    Token* token;

    // Saltar espacios
    while (current < input.length() && is_white_space(input[current]))
        current++;

    // Fin de entrada
    if (current >= input.length())
        return new Token(Token::END);

    char c = input[current];
    first = current;

    // ======================
    // NÚMEROS
    // ======================
    if (isdigit(c)) {
        current++;
        bool isFloat = false;

        // Parte entera
        while (current < input.length() && isdigit(input[current]))
            current++;

        // Verificar si hay punto decimal
        if (current < input.length() && input[current] == '.') {
            // Asegurarse de que hay un dígito después del punto
            if (current + 1 < input.length() && isdigit(input[current + 1])) {
                isFloat = true;
                current++; // consumir el punto

                // Parte decimal
                while (current < input.length() && isdigit(input[current]))
                    current++;
            }
        }

        if (current < input.length() && (input[current] == 'e' || input[current] == 'E')) {
            isFloat = true;
            current++;

            if (current < input.length() && (input[current] == '+' || input[current] == '-'))
                current++;

            while (current < input.length() && isdigit(input[current]))
                current++;
        }

        if (isFloat)
            return new Token(Token::FLOAT, input, first, current - first);
        else
            return new Token(Token::NUM, input, first, current - first);
    }

    // STRINGS
    if (c == '"') {
        current++;
        while (current < input.length() && input[current] != '"')
            current++;

        if (current >= input.length())
            return new Token(Token::ERR, input, first, current - first);

        current++; // cerrar comilla

        return new Token(Token::STRING, input, first, current - first);
    }

    // IDENTIFICADORES / PALABRAS CLAVE
    if (isalpha(c) || c == '_') {
        current++;

        while (current < input.length() &&
               (isalnum(input[current]) || input[current] == '_' || input[current] == '!'))
        {
            current++;
        }

        string lexema = input.substr(first, current - first);

        // Palabras clave
        if (lexema == "fn")     return new Token(Token::FN, input, first, current - first);
        if (lexema == "let")    return new Token(Token::LET, input, first, current - first);
        if (lexema == "mut")    return new Token(Token::MUT, input, first, current - first);
        if (lexema == "return") return new Token(Token::RETURN, input, first, current - first);
        if (lexema == "if")     return new Token(Token::IF, input, first, current - first);
        if (lexema == "else")   return new Token(Token::ELSE, input, first, current - first);
        if (lexema == "for")    return new Token(Token::FOR, input, first, current - first);
        if (lexema == "in")     return new Token(Token::IN, input, first, current - first);
        if (lexema == "while")  return new Token(Token::WHILE, input, first, current - first);

        if (lexema == "println!") return new Token(Token::PRINTLN, input, first, current - first);

        if (lexema == "true")  return new Token(Token::TRUE, input, first, current - first);
        if (lexema == "false") return new Token(Token::FALSE, input, first, current - first);

        return new Token(Token::ID, input, first, current - first);
    }

    // OPERADORES Y SÍMBOLOS
    // Primero operadores DOBLES
    if (strchr("+-*/(){},;:><=!.?", c)) {

        // ====== OPERADORES DOBLES ======

        if (c == '-' && input[current+1] == '>') {
            current += 2;
            return new Token(Token::ARROW, input, first, 2);
        }

        if (c == '=' && input[current+1] == '=') {
            current += 2;
            return new Token(Token::EQ, input, first, 2);
        }

        if (c == '!' && input[current+1] == '=') {
            current += 2;
            return new Token(Token::ERR, input, first, 2);
        }

        if (c == '<' && input[current+1] == '=') {
            current += 2;
            return new Token(Token::LE, input, first, 2);
        }

        if (c == '>' && input[current+1] == '=') {
            current += 2;
            return new Token(Token::GE, input, first, 2);
        }

        if (c == '*' && input[current+1] == '*') {
            current += 2;
            return new Token(Token::POW, input, first, 2);
        }

        if (c == '.' && input[current+1] == '.') {
            current += 2;
            return new Token(Token::RANGE, input, first, 2);
        }

        // ====== OPERADORES / SÍMBOLOS 1 CARACTER ======
        switch (c) {
            case '+': token = new Token(Token::PLUS, c); break;
            case '-': token = new Token(Token::MINUS, c); break;
            case '*': token = new Token(Token::MUL,  c); break;
            case '/': token = new Token(Token::DIV,  c); break;

            case '=': token = new Token(Token::ASSIGN, c); break;
            case '<': token = new Token(Token::LT, c); break;
            case '>': token = new Token(Token::GT, c); break;

            case '(': token = new Token(Token::LPAREN, c); break;
            case ')': token = new Token(Token::RPAREN, c); break;
            case '{': token = new Token(Token::LBRACE, c); break;
            case '}': token = new Token(Token::RBRACE, c); break;

            case ',': token = new Token(Token::COMMA, c); break;
            case ';': token = new Token(Token::SEMICOL, c); break;
            case '?': token = new Token(Token::QMARK, c); break;

            case ':': token = new Token(Token::COL, c); break;
        }

        current++;
        return token;
    }
}

// Destructor
Scanner::~Scanner() {}

// Función de prueba

int ejecutar_scanner(Scanner* scanner, const string& InputFile) {
    Token* tok;

    // Crear nombre para archivo de salida
    string OutputFileName = InputFile;
    size_t pos = OutputFileName.find_last_of(".");
    if (pos != string::npos) {
        OutputFileName = OutputFileName.substr(0, pos);
    }
    OutputFileName += "_tokens.txt";

    ofstream outFile(OutputFileName);
    if (!outFile.is_open()) {
        cerr << "Error: no se pudo abrir el archivo " << OutputFileName << endl;
        return 0;
    }

    outFile << "Scanner\n" << endl;

    while (true) {
        tok = scanner->nextToken();

        if (tok->type == Token::END) {
            outFile << *tok << endl;
            delete tok;
            outFile << "\nScanner exitoso" << endl << endl;
            outFile.close();
            return 0;
        }

        if (tok->type == Token::ERR) {
            outFile << *tok << endl;
            delete tok;
            outFile << "Caracter invalido" << endl << endl;
            outFile << "Scanner no exitoso" << endl << endl;
            outFile.close();
            return 0;
        }

        outFile << *tok << endl;
        delete tok;
    }
}
