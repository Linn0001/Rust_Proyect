#pragma once
#include "Visitor.h"

class SymbolTable;

class TypeCheck : public Visitor {
public:
    explicit TypeCheck(SymbolTable* table);
    Value visit(Program* program) override;
    Value visit(BinaryExp* exp) override;
    Value visit(TernaryExp* exp) override;
private:
    SymbolTable* table;
    bool isNumeric(Value::Type t) const;
    Value::Type promoteNumeric(Value::Type a, Value::Type b) const;
};
