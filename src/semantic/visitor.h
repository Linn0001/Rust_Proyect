#ifndef RUST_PROJECT_VISITOR_H
#define RUST_PROJECT_VISITOR_H

#include "ast.h"
#include "semantic_types.h"
#include <list>
#include <vector>
#include <unordered_map>
#include <string>
using namespace std;

class BinaryExp;
class NumberExp;
class FloatExp;
class BoolExp;
class Program;
class PrintStm;
class WhileStm;
class IfStm;
class AssignStm;
class Body;
class VarDec;
class FCallExp;
class ReturnStm;
class FunDec;
class OperatorDef;

class Visitor {
public:
    virtual int visit(BinaryExp* exp) = 0;
    virtual int visit(NumberExp* exp) = 0;
    virtual int visit(FloatExp* exp) = 0;
    virtual int visit(BoolExp* exp) = 0;
    virtual int visit(IdExp* exp) = 0;
    virtual int visit(Program* p) = 0;
    virtual int visit(PrintStm* stm) = 0;
    virtual int visit(WhileStm* stm) = 0;
    virtual int visit(IfStm* stm) = 0;
    virtual int visit(AssignStm* stm) = 0;
    virtual int visit(Body* body) = 0;
    virtual int visit(VarDec* vd) = 0;
    virtual int visit(FCallExp* fcall) = 0;
    virtual int visit(ReturnStm* r) = 0;
    virtual int visit(FunDec* fd) = 0;
    virtual int visit(OperatorDef* od) = 0;
};

class GenCodeVisitor : public Visitor {
    std::ostream& out;

    // Funciones auxiliares para cargar/guardar valores según tamaño
    void loadValue(const string& location, int size, Type::TType ttype);
    void storeValue(const string& location, int size, Type::TType ttype);
    unordered_map<string, double> floatConstants;
    int emitFunction(const string& name, const vector<string>& pnames, const vector<string>& ptypes, Body* body);

public:
    GenCodeVisitor(std::ostream& out) : out(out) {}
    int generar(Program* program);

    // Mapa de variables locales: nombre -> offset en el stack frame
    unordered_map<string, int> memory;

    // Mapa de variables globales: nombre -> (esGlobal, Tipo*)
    unordered_map<string, pair<bool, Type*>> globalMemory;

    // Mapa de tipos de variables locales: nombre -> Type*
    unordered_map<string, Type*> localTypes;

    int offset = -8;
    int labelcont = 0;
    bool entornoFuncion = false;
    string nombreFuncion;

    int visit(BinaryExp* exp) override;
    int visit(NumberExp* exp) override;
    int visit(FloatExp* exp) override;
    int visit(BoolExp* exp) override;
    int visit(IdExp* exp) override;
    int visit(Program* p) override ;
    int visit(PrintStm* stm) override;
    int visit(AssignStm* stm) override;
    int visit(WhileStm* stm) override;
    int visit(IfStm* stm) override;
    int visit(Body* body) override;
    int visit(VarDec* vd) override;
    int visit(FCallExp* fcall) override;
    int visit(ReturnStm* r) override;
    int visit(FunDec* fd) override;
    int visit(OperatorDef* od) override;
};

#endif //RUST_PROJECT_VISITOR_H