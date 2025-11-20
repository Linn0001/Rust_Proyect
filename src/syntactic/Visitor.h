#pragma once
#include "Exp.h"

class BinaryExp;
class TernaryExp;

class Visitor {
public:
    virtual ~Visitor() = default;
    virtual Value visit(Program* program) = 0;
    virtual Value visit(BinaryExp* exp) = 0;
    virtual Value visit(TernaryExp* exp) = 0;
};
