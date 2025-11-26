#ifndef RUST_PROJECT_TYPECHECKER_H
#define RUST_PROJECT_TYPECHECKER_H

#include <unordered_map>
#include <string>
#include "ast.h"
#include "environment.h"
#include "semantic_types.h"

using namespace std;

// ----------------------------------------------
//     Adelantos de clases del AST
// ----------------------------------------------
class BinaryExp;
class NumberExp;
class FloatExp;
class BoolExp;
class Program;
class IfStm;
class WhileStm;
class PrintStm;
class AssignStm;
class FunDec;
class ReturnStm;
class Body;
class VarDec;
class FCallExp;
class IdExp;
class OperatorDef;

// ----------------------------------------------
// VISITOR DE TIPOS (interfaz)
// ----------------------------------------------
class TypeVisitor {
public:
    // Nodos de alto nivel
    virtual void visit(Program* p) = 0;
    virtual void visit(Body* b) = 0;
    virtual void visit(VarDec* v) = 0;
    virtual void visit(FunDec* f) = 0;
    virtual void visit(OperatorDef* f) = 0;

    // Sentencias
    virtual void visit(IfStm* stm) = 0;
    virtual void visit(WhileStm* stm) = 0;
    virtual void visit(PrintStm* stm) = 0;
    virtual void visit(AssignStm* stm) = 0;
    virtual void visit(ReturnStm* stm) = 0;

    // Expresiones
    virtual Type* visit(BinaryExp* e) = 0;
    virtual Type* visit(NumberExp* e) = 0;
    virtual Type* visit(FloatExp* e) = 0;
    virtual Type* visit(BoolExp* e) = 0;
    virtual Type* visit(IdExp* e) = 0;
    virtual Type* visit(FCallExp* e) = 0;
};

// ===========================================================
//                   TYPECHECKER
// ===========================================================
class TypeChecker : public TypeVisitor {

private:

    Environment<Type*> env;                  // Entorno de variables
    struct FunctionSignature {
        string mangledName;
        Type* returnType;
        vector<Type*> args;
    };
    unordered_map<string, vector<FunctionSignature>> functions; // nombre -> firmas
    struct OperatorSignature {
        string functionName;
        Type* returnType;
        vector<Type*> args;
    };
    unordered_map<string, OperatorSignature> operatorTable; // (<op>, <left>, <right>) -> firma
    Type* currentFunctionReturnType;

    // Tipos básicos de Rust
    Type* t_bool;
    Type* t_i8;
    Type* t_i16;
    Type* t_i32;
    Type* t_i64;
    Type* t_u8;
    Type* t_u16;
    Type* t_u32;
    Type* t_u64;
    Type* t_f32;
    Type* t_f64;
    Type* t_unit;   // equivalente a ()

    // Registro de función (nombre → tipo retorno)
    void add_function(FunDec* fd);
    void add_operator(OperatorDef* op);
    string makeOperatorKey(BinaryOp op, Type* left, Type* right) const;
    string typeToString(Type* t) const;
    string mangleOperatorName(BinaryOp op, const vector<Type*>& args) const;
    FunctionSignature makeFunctionSignature(const string& name, Type* returnType, const vector<Type*>& args) const;
    const FunctionSignature* findFunctionOverload(const string& name, const vector<Type*>& args) const;

public:
    TypeChecker();

    // Punto de entrada
    void typecheck(Program* program);

    // --- Visitas de alto nivel ---
    void visit(Program* p) override;
    void visit(Body* b) override;
    void visit(VarDec* v) override;
    void visit(FunDec* f) override;
    void visit(OperatorDef* f) override;

    // --- Sentencias ---
    void visit(IfStm* stm) override;
    void visit(WhileStm* stm) override;
    void visit(PrintStm* stm) override;
    void visit(AssignStm* stm) override;
    void visit(ReturnStm* stm) override;

    // --- Expresiones ---
    Type* visit(BinaryExp* e) override;
    Type* visit(NumberExp* e) override;
    Type* visit(FloatExp* e) override;
    Type* visit(BoolExp *e) override;
    Type* visit(IdExp* e) override;
    Type* visit(FCallExp* e) override;
};

#endif //RUST_PROJECT_TYPECHECKER_H
