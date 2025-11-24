#ifndef RUST_PROJECT_AST_H
#define RUST_PROJECT_AST_H

#include <string>
#include <list>
#include <ostream>
#include <vector>
using namespace std;


// Forward declarations
class Visitor;
class VarDec;


// Operadores binarios soportados
enum BinaryOp {
    PLUS_OP,
    MINUS_OP,
    MUL_OP,
    DIV_OP,
    GT_OP,
    GE_OP,
    LT_OP,
    LE_OP,
    EQ_OP
};


// Clase base: Exp (expresiones)
class Exp {
public:
    // virtual int accept(Visitor* visitor) = 0;
    virtual ~Exp() = 0;

    static string binopToChar(BinaryOp op);
};


// Expresiones concretas
class BinaryExp : public Exp {
public:
    Exp* left;
    Exp* right;
    BinaryOp op;

    BinaryExp(Exp* l, Exp* r, BinaryOp op);
    // int accept(Visitor* visitor);
    ~BinaryExp();
};

class NumberExp : public Exp {
public:
    int val;

    NumberExp(int v);
    // int accept(Visitor* visitor);
    ~NumberExp();
};

class BoolExp : public Exp {
public:
    bool val;

    BoolExp(bool v);
    // int accept(Visitor* visitor);
    ~BoolExp();
};

class IdExp : public Exp {
public:
    string val;

    IdExp(string v);
    // int accept(Visitor* visitor);
    ~IdExp();
};

class FCallExp : public Exp {
public:
    string name;
    vector<Exp*> args;

    FCallExp() {}
    // int accept(Visitor* visitor);
    ~FCallExp() {}
};


// Clase base: Stm (statements)
class Stm {
public:
    // virtual int accept(Visitor* visitor) = 0;
    virtual ~Stm() = 0;
};


// Declaracione de variable
class VarDec {
public:
    string type;
    bool is_mutable;
    string name;

    VarDec();
    // int accept(Visitor* visitor);
    ~VarDec();
};


// Bloques (Body)
class Body {
public:
    list<Stm*> stmlist;
    list<VarDec*> decs;

    Body();
    // int accept(Visitor* visitor);
    ~Body();
};


// Statements concretos
class IfStm : public Stm {
public:
    Exp* cond;
    Body* then;
    Body* els;

    IfStm(Exp* condition, Body* then, Body* els);
    // int accept(Visitor* visitor);
    ~IfStm() {};
};

class WhileStm : public Stm {
public:
    Exp* cond;
    Body* b;

    WhileStm(Exp* condition, Body* b);
    // int accept(Visitor* visitor);
    ~WhileStm() {};
};

class AssignStm : public Stm {
public:
    string id;
    Exp* e;

    AssignStm(string, Exp*);
    // int accept(Visitor* visitor);
    ~AssignStm();
};

class PrintStm : public Stm {
public:
    Exp* e;

    PrintStm(Exp*);
    // int accept(Visitor* visitor);
    ~PrintStm();
};

class ReturnStm : public Stm {
public:
    Exp* e;

    ReturnStm() {}
    // int accept(Visitor* visitor);
    ~ReturnStm() {}
};


// Declaraciones de funciones
class FunDec {
public:
    string name;
    string type;
    Body* b;
    vector<string> ptypes;
    vector<string> pnames;

    FunDec() {}
    // int accept(Visitor* visitor);
    ~FunDec() {}
};


// Programa principal
class Program {
public:
    list<VarDec*> vdlist;
    list<FunDec*> fdlist;

    Program() {}
    // int accept(Visitor* visitor);
    ~Program() {}
};

#endif // RUST_PROJECT_AST_H
