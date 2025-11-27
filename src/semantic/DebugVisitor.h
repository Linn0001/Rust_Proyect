#ifndef RUST_PROJECT_DEBUGVISITOR_H
#define RUST_PROJECT_DEBUGVISITOR_H

#include "visitor.h"
#include "DebugTrace.h"
#include <string>
#include <unordered_map>

class DebugVisitor : public Visitor {
public:
    // Último valor evaluado en forma entera (para flags, etc.)
    long long lastValue = 0;

    // Soporte para floats en el debugger
    double lastFloatValue = 0.0;
    bool lastIsFloat = false;

    DebugTrace trace;

    // Tabla de funciones: nombre -> nodo FunDec
    std::unordered_map<std::string, FunDec*> functions;

    // Flag para cortar ejecución de un Body después de un return
    bool stopExecution = false;

    // Tracking de tipo y valor real para variables float
    std::unordered_map<std::string, bool>   isFloatVar;
    std::unordered_map<std::string, double> varFloatVal;

    DebugVisitor() = default;

    // Ejecuta todo el programa en modo "depurador AST"
    void run(Program* p);

    // Evalúa una expresión y devuelve el valor entero (para flags / registros)
    long long eval(Exp* e);
    void showEnv();
    void step(const std::string& msg);

    void updateFlags(long long val);

    // Helpers para formatear valores (enteros vs floats)
    std::string formatNumber(double v) const;
    std::string formatValue(long long ival, double fval, bool isFloat) const;

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
    int visit(TernaryExp* e) override;
};

#endif // RUST_PROJECT_DEBUGVISITOR_H
