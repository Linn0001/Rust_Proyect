#include "ast.h"
#include <iostream>
using namespace std;

//                      Exp
Exp::~Exp() {}

string Exp::binopToChar(BinaryOp op) {
    switch (op) {
        case PLUS_OP:  return "+";
        case MINUS_OP: return "-";
        case MUL_OP:   return "*";
        case DIV_OP:   return "/";
        case GT_OP:    return ">";
        case GE_OP:    return ">=";
        case LT_OP:    return "<";
        case LE_OP:    return "<=";
        case EQ_OP:    return "==";
        default:       return "?";
    }
}

//                     BinaryExp
BinaryExp::BinaryExp(Exp* l, Exp* r, BinaryOp o)
    : left(l), right(r), op(o) {}

BinaryExp::~BinaryExp() {
    delete left;
    delete right;
}

//                     NumberExp
NumberExp::NumberExp(long long v) : val(v) {}   // antes: int v


NumberExp::~NumberExp() {}

//                     FloatExp
FloatExp::FloatExp(double v) : val(v) {}

FloatExp::~FloatExp() {}

//                     BoolExp
BoolExp::BoolExp(bool v) : val(v) {}

BoolExp::~BoolExp() {}

//                     IdExp
IdExp::IdExp(string v) : val(v) {}

IdExp::~IdExp() {}

//                     Stm (base)
Stm::~Stm() {}

//                     PrintStm
PrintStm::PrintStm(Exp* expresion) {
    e = expresion;
}

PrintStm::~PrintStm() {}

//                     AssignStm
AssignStm::AssignStm(string variable, Exp* expresion) {
    id = variable;
    e = expresion;
}

AssignStm::~AssignStm() {}

//                     IfStm
IfStm::IfStm(Exp* c, Body* t, Body* e)
    : cond(c), then(t), els(e) {}

IfStm::~IfStm() {}

//                     WhileStm
WhileStm::WhileStm(Exp* c, Body* t)
    : cond(c), b(t) {}

WhileStm::~WhileStm() {}

//                     ForStm
ForStm::ForStm(string id, Exp* s, Exp* e, Body* b)
        : id(id), start(s), end(e), b(b) {}

ForStm::~ForStm(){}



//                     ExpAsStm
// ExpAsStm::ExpAsStm(Exp *e) : e(e) {}
//
// ExpAsStm::~ExpAsStm() {}


//                     VarDec
VarDec::VarDec() {}

VarDec::~VarDec() {}

//                     Body
Body::Body() {
    decs = list<VarDec*>();
    stmlist = list<Stm*>();
}

Body::~Body() {}
