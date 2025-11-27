#ifndef RUST_PROJECT_DEBUGVISITOR_H
#define RUST_PROJECT_DEBUGVISITOR_H

#include "visitor.h"
#include "DebugTrace.h"
#include <string>
#include <unordered_map>

class DebugVisitor : public Visitor {
public:
    long long lastValue = 0;
    DebugTrace trace;

    // Tabla de funciones: nombre -> nodo FunDec
    std::unordered_map<std::string, FunDec*> functions;

    // Flag para cortar ejecución de un Body después de un return
    bool stopExecution = false;

    DebugVisitor() = default;

    void run(Program* p);

    long long eval(Exp* e);
    void showEnv();
    void step(const std::string& msg);

    void updateFlags(long long val);

    // Visitor impl
    int visit(Program* p) override;
    int visit(FunDec* fd) override;
    int visit(Body* body) override;
    int visit(VarDec* vd) override;
    int visit(AssignStm* stm) override;
    int visit(PrintStm* stm) override;
    int visit(IfStm* stm) override;
    int visit(WhileStm* stm) override;
    int visit(ForStm* stm) override;
    int visit(ReturnStm* r) override;

    int visit(NumberExp* exp) override;
    int visit(FloatExp* exp) override;
    int visit(BoolExp* exp) override;
    int visit(IdExp* exp) override;
    int visit(BinaryExp* exp) override;
    int visit(FCallExp* fcall) override;
};

#endif // RUST_PROJECT_DEBUGVISITOR_H
