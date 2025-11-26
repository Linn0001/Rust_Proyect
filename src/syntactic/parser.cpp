#include<iostream>
#include "token.h"
#include "scanner.h"
#include "ast.h"
#include "parser.h"

using namespace std;

// =============================
// Métodos de la clase Parser
// =============================

Parser::Parser(Scanner* sc) : scanner(sc) {
    previous = nullptr;
    current = scanner->nextToken();
    if (current->type == Token::ERR) {
        throw runtime_error("Error léxico");
    }
}

bool Parser::match(Token::Type ttype) {
    if (check(ttype)) {
        advance();
        return true;
    }
    return false;
}

bool Parser::check(Token::Type ttype) {
    if (isAtEnd()) return false;
    return current->type == ttype;
}

bool Parser::advance() {
    if (!isAtEnd()) {
        Token* temp = current;
        if (previous) delete previous;
        current = scanner->nextToken();
        previous = temp;

        if (check(Token::ERR)) {
            throw runtime_error("Error lexico");
        }
        return true;
    }
    return false;
}

bool Parser::isAtEnd() {
    return (current->type == Token::END);
}

// =============================
// Reglas gramaticales
// =============================

Program* Parser::parseProgram() {
    Program* p = new Program();

    if (check(Token::LET)) {
        p->vdlist.push_back(parseVarDec());
        while (match(Token::SEMICOL)) {
            if (check(Token::LET)) {
                p->vdlist.push_back(parseVarDec());
            }
        }
    }
    if (check(Token::FN)) {
        p->fdlist.push_back(parseFunDec());
        while (check(Token::FN)) {
                p->fdlist.push_back(parseFunDec());
            }
        }
    cout << "Parser exitoso" << endl;

    return p;
}

VarDec* Parser::parseVarDec() {
    VarDec* vd = new VarDec();

    match(Token::LET);

    if (match(Token::MUT)) {
        vd->isMutable = true;
    }
    else {
        vd->isMutable = false;
    }

    match(Token::ID);
    vd->name = previous->text;

    match(Token::COL);

    match(Token::ID);
    vd->type = previous->text;

    if (match(Token::ASSIGN)) {
        vd->e = parseCE();
    }

    return vd;
}

FunDec *Parser::parseFunDec() {
    FunDec* fd = new FunDec();

    match(Token::FN);

    match(Token::ID);
    fd->name = previous->text;

    match(Token::LPAREN);

    if (check(Token::ID)) {
        while (match(Token::ID)) {
            fd->pnames.push_back(previous->text);
            match(Token::COL);
            match(Token::ID);
            fd->ptypes.push_back(previous->text);
            match(Token::COMMA);
        }
    }
    match(Token::RPAREN);

    if (match(Token::ARROW)) {
        match(Token::ID);
        fd->type = previous->text;
    }
    else {
        fd->type = "i32";
    }

    match(Token::LBRACE);

    fd->b = parseBody();

    match(Token::RBRACE);

    return fd;
}


Body* Parser::parseBody() {
    Body* b = new Body();

    // Declaraciones de variables
    if (check(Token::LET)) {
        b->decs.push_back(parseVarDec());
        while (match(Token::SEMICOL)) {
            if (check(Token::LET)) {
                b->decs.push_back(parseVarDec());
            }
        }
    }

    // Statements: repetir hasta encontrar '}'
    while (!check(Token::RBRACE) && !isAtEnd()) {
        b->stmlist.push_back(parseStm());
        // Consumir ';' si está, pero no lo exigimos después de bloques
        match(Token::SEMICOL);
    }

    return b;
}

Stm* Parser::parseStm() {
    Stm* a;
    Exp* e;
    string var;
    Body* tb = nullptr;
    Body* fb = nullptr;

    if (match(Token::ID)) {
        var = previous->text;
        match(Token::ASSIGN);
        e = parseCE();

        return new AssignStm(var, e);
    }
    else if (match(Token::PRINTLN)) {
        match(Token::LPAREN);
        match(Token::STRING);
        match(Token::COMMA);
        e = parseCE();
        match(Token::RPAREN);

        return new PrintStm(e);
    }
    else if (match(Token::RETURN)) {
        ReturnStm* r  = new ReturnStm();

        match(Token::LPAREN);
        r->e = parseCE();
        match(Token::RPAREN);

        return r;
    }
    else if (match(Token::IF)) {
        e = parseCE();

        if (!match(Token::LBRACE)) {
            cout << "Error: se esperaba '{' después de la expresión." << endl;
            exit(1);
        }

        tb = parseBody();

        if (!match(Token::RBRACE)) {
            cout << "Error: se esperaba '}' después del bloque then." << endl;
            exit(1);
        }

        if (match(Token::ELSE)) {
            if (!match(Token::LBRACE)) {
                cout << "Error: se esperaba '{' después de else." << endl;
                exit(1);
            }
            fb = parseBody();

            if (!match(Token::RBRACE)) {
                cout << "Error: se esperaba '}' al final del bloque else." << endl;
                exit(1);
            }
        }

        a = new IfStm(e, tb, fb);
    }
    else if (match(Token::WHILE)) {
        e = parseCE();

        if (!match(Token::LBRACE)) {
            cout << "Error: se esperaba '{' después de la expresión." << endl;
            exit(1);
        }

        tb = parseBody();

        if (!match(Token::RBRACE)) {
            cout << "Error: se esperaba '}' al final de la declaración while." << endl;
            exit(1);
        }

        a = new WhileStm(e, tb);
    }
    else if (match(Token::FOR)) {
        // Sintaxis esperada:
        // for i in 1..11 {
        //     ...
        // }

        // Identificador del iterador
        if (!match(Token::ID)) {
            cout << "Error: se esperaba identificador después de 'for'." << endl;
            exit(1);
        }
        string iter = previous->text;

        // Palabra clave 'in'
        if (!match(Token::IN)) {
            cout << "Error: se esperaba 'in' después del iterador en el for." << endl;
            exit(1);
        }

        // Expresión de inicio
        Exp* start = parseCE();

        // Operador de rango '..'
        if (!match(Token::RANGE)) {
            cout << "Error: se esperaba '..' en el rango del for." << endl;
            exit(1);
        }

        // Expresión de fin
        Exp* end = parseCE();

        // Cuerpo entre llaves
        if (!match(Token::LBRACE)) {
            cout << "Error: se esperaba '{' después del rango en el for." << endl;
            exit(1);
        }

        Body* body = parseBody();

        if (!match(Token::RBRACE)) {
            cout << "Error: se esperaba '}' al final del for." << endl;
            exit(1);
        }

        return new ForStm(iter, start, end, body);
    }
    else {
        throw runtime_error("Error sintáctico");
    }

    return a;
}


    Exp* Parser::parseCE() {
        Exp* l = parseBE();

        if (match(Token::GT)) {
            BinaryOp op = GT_OP;
            Exp* r = parseBE();

            l = new BinaryExp(l, r, op);
        }
        else if (match(Token::GE)) {
            BinaryOp op = GE_OP;
            Exp* r = parseBE();

            l = new BinaryExp(l, r, op);
        }
        else if (match(Token::LT)) {
            BinaryOp op = LT_OP;
            Exp* r = parseBE();

            l = new BinaryExp(l, r, op);
        }
        else if (match(Token::LE)) {
            BinaryOp op = LE_OP;
            Exp* r = parseBE();

            l = new BinaryExp(l, r, op);
        }
        else if (match(Token::EQ)) {
            BinaryOp op = EQ_OP;
            Exp* r = parseBE();

            l = new BinaryExp(l, r, op);
        }

        return l;
    }


    Exp* Parser::parseBE() {
        Exp* l = parseE();

        while (match(Token::PLUS) || match(Token::MINUS)) {
            BinaryOp op;

            if (previous->type == Token::PLUS){
                op = PLUS_OP;
            }
            else {
                op = MINUS_OP;
            }

            Exp* r = parseE();

            l = new BinaryExp(l, r, op);
        }

        return l;
    }


    Exp* Parser::parseE() {
        Exp* l = parseF();

        while (match(Token::MUL) || match(Token::DIV)) {
            BinaryOp op;

            if (previous->type == Token::MUL){
                op = MUL_OP;
            }
            else {
                op = DIV_OP;
            }

            Exp* r = parseF();

            l = new BinaryExp(l, r, op);
        }

        return l;
    }


    // Exp* Parser::parseT() {
    //     Exp* l = parseF();
    //
    //     if (match(Token::POW)) {
    //         BinaryOp op = POW_OP;
    //         Exp* r = parseF();
    //         l = new BinaryExp(l, r, op);
    //     }
    //     return l;
    // }

    Exp* Parser::parseF() {
        Exp* e;
        string nom;

        if (match(Token::NUM)) {
            return new NumberExp(stoll(previous->text));
        }
        else if (match(Token::FLOAT)) {
            return new FloatExp(stod(previous->text));
        }
        else if (match(Token::TRUE)) {
            return new BoolExp(true);
        }
        else if (match(Token::FALSE)) {
            return new BoolExp(false);
        }
        else if (match(Token::LPAREN)) {
            e = parseCE();
            match(Token::RPAREN);

            return e;
        }
        else if (match(Token::ID)) {
            nom = previous->text;

            if (check(Token::LPAREN)) {
                match(Token::LPAREN);

                FCallExp* fcall = new FCallExp();

                fcall->name = nom;

                if (!check(Token::RPAREN)) {
                    fcall->args.push_back(parseCE());
                    while(match(Token::COMMA)) {
                        fcall->args.push_back(parseCE());
                    }
                }
                match(Token::RPAREN);
                return fcall;
            }
            else {
                return new IdExp(nom);
            }
        }
        else {
            throw runtime_error("Error sintáctico");
        }
}
