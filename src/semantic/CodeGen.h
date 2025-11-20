#pragma once
#include <string>
#include <fstream>
#include "Visitor.h"

class CodeGen : public Visitor {
public:
    explicit CodeGen(const std::string& filename);
    ~CodeGen();
    Value visit(Program* program) override;
    Value visit(BinaryExp* exp) override;
    Value visit(TernaryExp* exp) override;
private:
    std::ofstream out;
};
