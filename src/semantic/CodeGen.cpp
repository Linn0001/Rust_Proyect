#include "CodeGen.h"

CodeGen::CodeGen(const std::string& filename) : out(filename) {}

CodeGen::~CodeGen() {
    if (out.is_open()) out.close();
}

Value CodeGen::visit(Program* program) {
    // TODO: generar prólogo / epílogo del programa
    return Value(Value::UNIT);
}

Value CodeGen::visit(BinaryExp* exp) {
    // TODO: generar código para operaciones aritméticas
    return Value(Value::I32);
}

Value CodeGen::visit(TernaryExp* exp) {
    // TODO: generar código para expresión ternaria
    return Value(Value::I32);
}
