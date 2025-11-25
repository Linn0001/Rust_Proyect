// #ifndef RUST_PROJECT_VISITOR_H
// #define RUST_PROJECT_VISITOR_H
//
// #include "ast.h"
// #include <list>
// #include <vector>
// #include <unordered_map>
// #include <string>
// using namespace std;
//
// class BinaryExp;
// class NumberExp;
// class BoolExp;
// class Program;
// class PrintStm;
// class WhileStm;
// class IfStm;
// class AssignStm;
// class Body;
// class VarDec;
// class FCallExp;
// class ReturnStm;
// class FunDec;
// // class CastExp;
//
// class Visitor {
// public:
//
//     virtual int visit(BinaryExp* exp) = 0;
//     virtual int visit(NumberExp* exp) = 0;
//     virtual int visit(BoolExp* exp) = 0;
//     virtual int visit(IdExp* exp) = 0;
//     virtual int visit(Program* p) = 0;
//     virtual int visit(PrintStm* stm) = 0;
//     virtual int visit(WhileStm* stm) = 0;
//     virtual int visit(IfStm* stm) = 0;
//     virtual int visit(AssignStm* stm) = 0;
//     virtual int visit(Body* body) = 0;
//     virtual int visit(VarDec* vd) = 0;
//     virtual int visit(FCallExp* fcall) = 0;
//     virtual int visit(ReturnStm* r) = 0;
//     virtual int visit(FunDec* fd) = 0;
//     // virtual int visit(CastExp* exp) = 0;
//     // virtual int visit(TernaryExp* exp) = 0;
// };
//
// class GenCodeVisitor : public Visitor {
//     std::ostream& out;
// public:
//     GenCodeVisitor(std::ostream& out) : out(out) {}
//     int generar(Program* program);
//     unordered_map<string, int> memory;
//     unordered_map<string, bool> globalMemory;
//     int offset = -8;
//     int labelcont = 0;
//     bool entornoFuncion = false;
//     string nombreFuncion;
//
//     int visit(BinaryExp* exp) override;
//     int visit(NumberExp* exp) override;
//     int visit(BoolExp* exp) override;
//     int visit(IdExp* exp) override;
//     int visit(Program* p) override ;
//     int visit(PrintStm* stm) override;
//     int visit(AssignStm* stm) override;
//     int visit(WhileStm* stm) override;
//     int visit(IfStm* stm) override;
//     int visit(Body* body) override;
//     int visit(VarDec* vd) override;
//     int visit(FCallExp* fcall) override;
//     int visit(ReturnStm* r) override;
//     int visit(FunDec* fd) override;
//     // int visit(CastExp* exp) override;
//     // int visit(TernaryExp* exp) override;
// };
//
// #endif //RUST_PROJECT_VISITOR_H