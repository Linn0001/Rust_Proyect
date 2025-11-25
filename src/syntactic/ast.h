#ifndef RUST_PROJECT_AST_H
#define RUST_PROJECT_AST_H

#include <string>
#include <list>
#include <ostream>
#include <vector>
#include "semantic_types.h"
using namespace std;


// Forward declarations
class Visitor;
class VarDec;
class TypeVisitor;


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
    virtual int accept(Visitor* visitor) = 0;
    virtual ~Exp() = 0;

    static string binopToChar(BinaryOp op);
    virtual Type* accept(TypeVisitor* visitor) = 0;
};


// Expresiones concretas
class BinaryExp : public Exp {
public:
    Exp* left;
    Exp* right;
    BinaryOp op;

    BinaryExp(Exp* l, Exp* r, BinaryOp op);
    int accept(Visitor* visitor);
    Type* accept(TypeVisitor* visitor);
    ~BinaryExp();
};

class NumberExp : public Exp {
public:
    int val;

    NumberExp(int v);
    int accept(Visitor* visitor);
    Type* accept(TypeVisitor* visitor);
    ~NumberExp();
};

class BoolExp : public Exp {
public:
    bool val;

    BoolExp(bool v);
    int accept(Visitor* visitor);
    Type* accept(TypeVisitor* visitor);
    ~BoolExp();
};

class IdExp : public Exp {
public:
    string val;

    IdExp(string v);
    int accept(Visitor* visitor);
    Type* accept(TypeVisitor* visitor);
    ~IdExp();
};

class FCallExp : public Exp {
public:
    string name;
    vector<Exp*> args;

    FCallExp() {}
    int accept(Visitor* visitor);
    Type* accept(TypeVisitor* visitor);
    ~FCallExp() {}
};


// Clase base: Stm (statements)
class Stm {
public:
    virtual int accept(Visitor* visitor) = 0;
    virtual ~Stm() = 0;
    virtual void accept(TypeVisitor* visitor) = 0;
};


// Declaracione de variable
class VarDec {
public:
    string type;
    Type* resolved;
    bool isMutable;
    string name;
    Exp* e;

    int size;
    int align;

    VarDec();
    int accept(Visitor* visitor);
    void accept(TypeVisitor* visitor);
    ~VarDec();

    void computeLayout() {
        Type* t = new Type();
        t->set_basic_type(type);
        auto info = Type::TYPE_TABLE[t->ttype];
        size = info.size;
        align = info.align;
    }
};


// Bloques (Body)
class Body {
public:
    list<Stm*> stmlist;
    list<VarDec*> decs;

    Body();
    int accept(Visitor* visitor);
    void accept(TypeVisitor* visitor);
    ~Body();
};


// Statements concretos
class IfStm : public Stm {
public:
    Exp* cond;
    Body* then;
    Body* els;

    IfStm(Exp* condition, Body* then, Body* els);
    int accept(Visitor* visitor);
    void accept(TypeVisitor* visitor);
    ~IfStm();
};

class WhileStm : public Stm {
public:
    Exp* cond;
    Body* b;

    WhileStm(Exp* condition, Body* b);
    int accept(Visitor* visitor);
    void accept(TypeVisitor* visitor);
    ~WhileStm();
};

class AssignStm : public Stm {
public:
    string id;
    Exp* e;

    AssignStm(string, Exp*);
    int accept(Visitor* visitor);
    void accept(TypeVisitor* visitor);
    ~AssignStm();
};

class PrintStm : public Stm {
public:
    Exp* e;

    PrintStm(Exp*);
    int accept(Visitor* visitor);
    void accept(TypeVisitor* visitor);
    ~PrintStm();
};

class ReturnStm : public Stm {
public:
    Exp* e;

    ReturnStm() {}
    int accept(Visitor* visitor);
    void accept(TypeVisitor* visitor);
    ~ReturnStm() {}
};

// class ExpAsStm : public Stm {
// public:
//     Exp* e;
//
//     ExpAsStm(Exp* e);
//     // int accept(Visitor* visitor);
//     ~ExpAsStm();
// };


// Declaraciones de funciones
class FunDec {
public:
    string name;
    string type;
    Body* b;
    vector<string> ptypes;
    vector<string> pnames;

    FunDec() {}
    int accept(Visitor* visitor);
    void accept(TypeVisitor* visitor);
    ~FunDec() {}
};


// Programa principal
class Program {
public:
    list<VarDec*> vdlist;
    list<FunDec*> fdlist;

    Program() {}
    int accept(Visitor* visitor);
    void accept(TypeVisitor* visitor);
    ~Program() {}
};

#endif // RUST_PROJECT_AST_H
