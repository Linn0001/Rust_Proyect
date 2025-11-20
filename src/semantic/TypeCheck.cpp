#include "TypeCheck.h"
#include <stdexcept>

class SymbolTable {};

TypeCheck::TypeCheck(SymbolTable* table) : table(table) {}

bool TypeCheck::isNumeric(Value::Type t) const {
    return t == Value::I32 || t == Value::I64 ||
           t == Value::U32 || t == Value::F32;
}

Value::Type TypeCheck::promoteNumeric(Value::Type a, Value::Type b) const {
    if (a == Value::F32 || b == Value::F32) return Value::F32;
    if (a == Value::I64 || b == Value::I64) return Value::I64;
    if (a == Value::U32 || b == Value::U32) return Value::U32;
    return Value::I32;
}

Value TypeCheck::visit(Program* program) {
    // TODO: recorrer el programa y chequear tipos
    return Value(Value::UNIT);
}

Value TypeCheck::visit(BinaryExp* exp) {
    // TODO: ejemplo para operadores numéricos
    // Value lhs = exp->left->accept(this);
    // Value rhs = exp->right->accept(this);
    // if (!isNumeric(lhs.type) || !isNumeric(rhs.type)) error
    return Value(Value::I32);
}

Value TypeCheck::visit(TernaryExp* exp) {
    // TODO: chequeo de tipos para expresión condicional (ternaria)
    return Value(Value::I32);
}
