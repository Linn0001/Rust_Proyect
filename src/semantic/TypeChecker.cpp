#include "TypeChecker.h"
#include <iostream>
#include <algorithm>
#include <stdexcept>
using namespace std;


Type* NumberExp::accept(TypeVisitor* v) { return v->visit(this); }
Type* FloatExp::accept(TypeVisitor* v) { return v->visit(this); }
Type* BoolExp::accept(TypeVisitor* v) { return v->visit(this); }
Type* IdExp::accept(TypeVisitor* v) { return v->visit(this); }
Type* BinaryExp::accept(TypeVisitor* v) { return v->visit(this); }
Type* FCallExp::accept(TypeVisitor* v) { return v->visit(this); }

void IfStm::accept(TypeVisitor* v) { v->visit(this); }
void WhileStm::accept(TypeVisitor* v) { v->visit(this); }
void AssignStm::accept(TypeVisitor* v) { v->visit(this); }
void PrintStm::accept(TypeVisitor* v) { v->visit(this); }
void ReturnStm::accept(TypeVisitor* v) { v->visit(this); }

void VarDec::accept(TypeVisitor* v) { v->visit(this); }
void FunDec::accept(TypeVisitor* v) { v->visit(this); }
void Body::accept(TypeVisitor* v) { v->visit(this); }
void Program::accept(TypeVisitor* v) { v->visit(this); }
void OperatorDef::accept(TypeVisitor* v) { v->visit(this); }

// ===========================================================
//   Constructor del TypeChecker
// ===========================================================

TypeChecker::TypeChecker() {
    t_bool = new Type(Type::BOOL);
    t_i8 = new Type(Type::I8);
    t_i16 = new Type(Type::I16);
    t_i32 = new Type(Type::I32);
    t_i64 = new Type(Type::I64);
    t_u8 = new Type(Type::U8);
    t_u16 = new Type(Type::U16);
    t_u32 = new Type(Type::U32);
    t_u64 = new Type(Type::U64);
    t_f32 = new Type(Type::F32);
    t_f64 = new Type(Type::F64);
    t_unit = new Type(Type::UNIT);
}

// ===========================================================
//   Registrar funciones globales
// ===========================================================

TypeChecker::FunctionSignature TypeChecker::makeFunctionSignature(const string& name, Type* returnType, const vector<Type*>& args) const {
    return {name, returnType, args};
}

const TypeChecker::FunctionSignature* TypeChecker::findFunctionOverload(const string& name, const vector<Type*>& args) const {
    auto it = functions.find(name);
    if (it == functions.end()) return nullptr;

    for (const auto& sig : it->second) {
        if (sig.args.size() != args.size()) continue;
        bool match = true;
        for (size_t i = 0; i < args.size(); ++i) {
            if (!sig.args[i]->match(args[i])) {
                match = false;
                break;
            }
        }
        if (match) return &sig;
    }
    return nullptr;
}
void TypeChecker::add_function(FunDec* fd) {
    Type* returnType = Type::from_string(fd->type);

    if (!returnType) {
        cerr << "Error: tipo de retorno no válido en función '"
             << fd->name << "' (" << fd->type << ")." << endl;
        exit(0);
    }

    vector<Type*> args;
    for (const auto& ptype : fd->ptypes) {
        Type* param = Type::from_string(ptype);
        if (!param) {
            cerr << "Error: tipo de parámetro no válido en función '" << fd->name << "'." << endl;
            exit(0);
        }
        args.push_back(param);
    }

    auto sig = makeFunctionSignature(fd->name, returnType, args);

    auto& overloads = functions[fd->name];
    for (const auto& existing : overloads) {
        if (existing.args.size() == sig.args.size()) {
            bool same = true;
            for (size_t i = 0; i < sig.args.size(); ++i) {
                if (!existing.args[i]->match(sig.args[i])) {
                    same = false;
                    break;
                }
            }
            if (same) {
                cerr << "Error: función '" << fd->name << "' ya fue declarada con la misma firma." << endl;
                exit(0);
            }
        }
    }

    sig.mangledName = fd->name;
    fd->mangledName = sig.mangledName;
    overloads.push_back(sig);
}

string TypeChecker::mangleOperatorName(BinaryOp op, const vector<Type*>& args) const {
    string name = "__op_" + Exp::binopToName(op);
    for (auto* a : args) {
        name += "_" + a->str();
    }
    return name;
}

void TypeChecker::add_operator(OperatorDef* od) {
    Type* returnType = Type::from_string(od->type);
    if (!returnType) {
        cerr << "Error: tipo de retorno no válido en operador '" << Exp::binopToChar(od->operatorKind) << "'." << endl;
        exit(0);
    }

    vector<Type*> args;
    for (const auto& ptype : od->ptypes) {
        Type* param = Type::from_string(ptype);
        if (!param) {
            cerr << "Error: tipo de parámetro no válido en operador '" << Exp::binopToChar(od->operatorKind) << "'." << endl;
            exit(0);
        }
        args.push_back(param);
    }

    if (args.empty() || args.size() > 2) {
        cerr << "Error: las sobrecargas de operador requieren uno o dos parámetros." << endl;
        exit(0);
    }

    string mangled = mangleOperatorName(od->operatorKind, args);
    od->mangledName = mangled;

    auto sig = makeFunctionSignature(mangled, returnType, args);

    auto& overloads = functions[mangled];
    for (const auto& existing : overloads) {
        if (existing.args.size() == sig.args.size()) {
            bool same = true;
            for (size_t i = 0; i < sig.args.size(); ++i) {
                if (!existing.args[i]->match(sig.args[i])) {
                    same = false;
                    break;
                }
            }
            if (same) {
                cerr << "Error: sobrecarga duplicada para operador '" << Exp::binopToChar(od->operatorKind)
                     << "' con tipos ya registrados." << endl;
                exit(0);
            }
        }
    }
    overloads.push_back(sig);

    Type* right = args.size() > 1 ? args[1] : nullptr;
    string key = makeOperatorKey(od->operatorKind, args[0], right);
    if (operatorTable.find(key) != operatorTable.end()) {
        cerr << "Error: sobrecarga duplicada para operador '" << Exp::binopToChar(od->operatorKind)
             << "' con la combinación de tipos dada." << endl;
        exit(0);
    }
    operatorTable[key] = {mangled, returnType, args};
}

string TypeChecker::makeOperatorKey(BinaryOp op, Type* left, Type* right) const {
    string key = Exp::binopToName(op) + ":" + to_string(left->ttype) + ":";
    key += (right ? to_string(right->ttype) : string("none"));
    return key;
}

string TypeChecker::typeToString(Type* t) const {
    return t ? t->str() : "notype";
}

// ===========================================================
//   Método principal de verificación
// ===========================================================

void TypeChecker::typecheck(Program* program) {
    if (program) program->accept(this);
    cout << "Revisión exitosa" << endl;
}

// ===========================================================
//   Nivel superior: Programa y Bloque
// ===========================================================

void TypeChecker::visit(Program* p) {
    // Primero registrar funciones
    for (auto f : p->fdlist)
        add_function(f);
    for (auto op : p->opList)
        add_operator(op);

    env.add_level();
    for (auto v : p->vdlist)
        v->accept(this);
    for (auto f : p->fdlist)
        f->accept(this);
    for (auto op : p->opList)
        op->accept(this);
    env.remove_level();
}

void TypeChecker::visit(Body* b) {
    env.add_level();
    for (auto v : b->decs)
        v->accept(this);
    for (auto s : b->stmlist)
        s->accept(this);
    env.remove_level();
}

// ===========================================================
//   Declaraciones
// ===========================================================

void TypeChecker::visit(VarDec* v) {
    Type* t = new Type();
    if (!t->set_basic_type(v->type)) {
        cerr << "Error: tipo de variable no válido." << endl;
        exit(0);
    }

    if (env.check(v->name)) {
        cerr << "Error: variable '" << v->name << "' ya declarada." << endl;
        exit(0);
    }
    env.add_var(v->name, t);
    v->resolved = t;

    if (v->e) {
        Type* initType = v->e->accept(this);
        if (!t->match(initType)) {
            cerr << "Error: tipos incompatibles en la inicialización de '" << v->name << "'." << endl;
            exit(0);
        }
    }
}

void TypeChecker::visit(FunDec* f) {
    Type* oldReturn = currentFunctionReturnType;

    vector<Type*> args;
    for (const auto& t : f->ptypes) {
        Type* resolved = Type::from_string(t);
        if (!resolved) {
            cerr << "Error: tipo de parámetro inválido en función '" << f->name << "'." << endl;
            exit(0);
        }
        args.push_back(resolved);
    }

    const FunctionSignature* sig = findFunctionOverload(f->name, args);
    if (!sig) {
        cerr << "Error interno: firma de función no registrada para '" << f->name << "'." << endl;
        exit(0);
    }

    f->mangledName = sig->mangledName;
    currentFunctionReturnType = sig->returnType;

    env.add_level();

    for (size_t i = 0; i < f->pnames.size(); ++i) {
        env.add_var(f->pnames[i], args[i]);
    }

    f->b->accept(this);

    env.remove_level();

    currentFunctionReturnType = oldReturn;
}

void TypeChecker::visit(OperatorDef* f) {
    Type* oldReturn = currentFunctionReturnType;

    vector<Type*> args;
    for (const auto& t : f->ptypes) {
        Type* resolved = Type::from_string(t);
        if (!resolved) {
            cerr << "Error: tipo de parámetro inválido en operador '" << Exp::binopToChar(f->operatorKind) << "'." << endl;
            exit(0);
        }
        args.push_back(resolved);
    }

    const FunctionSignature* sig = findFunctionOverload(f->mangledName, args);
    if (!sig) {
        cerr << "Error interno: firma no registrada para operador '" << Exp::binopToChar(f->operatorKind) << "'." << endl;
        exit(0);
    }

    currentFunctionReturnType = sig->returnType;

    env.add_level();
    for (size_t i = 0; i < f->pnames.size(); ++i) {
        env.add_var(f->pnames[i], args[i]);
    }

    f->b->accept(this);
    env.remove_level();

    currentFunctionReturnType = oldReturn;
}


// ===========================================================
//   Sentencias
// ===========================================================

void TypeChecker::visit(IfStm* stm) {
    // Verificar que la condición sea bool
    Type* condType = stm->cond->accept(this);

    if (!condType->match(t_bool)) {
        cerr << "Error: la condición del if debe ser bool." << endl;
        exit(0);
    }

    // Verificar bloque 'then'
    if (stm->then)
        stm->then->accept(this);

    // Verificar bloque else si existe
    if (stm->els)
        stm->els->accept(this);
}


void TypeChecker::visit(WhileStm* stm) {
    // Verificar que la condición sea bool
    Type* condType = stm->cond->accept(this);

    if (!condType->match(t_bool)) {
        cerr << "Error: la condición del while debe ser bool." << endl;
        exit(0);
    }

    // Verificar el cuerpo del while
    if (stm->b)
        stm->b->accept(this);
}


void TypeChecker::visit(PrintStm* stm) {
    Type* t = stm->e->accept(this);

    // Tipos permitidos en println (Rust simplificado)
    bool ok =
        t->match(t_bool) ||
        t->match(t_i8)  ||
        t->match(t_i16) ||
        t->match(t_i32) ||
        t->match(t_i64) ||
        t ->match(t_u8) ||
        t ->match(t_u16)||
        t ->match(t_u32)||
        t ->match(t_u64)||
        t->match(t_f32) ||
        t->match(t_f64);

    if (!ok) {
        cerr << "Error: tipo inválido en println! (solo bool, enteros o floats)" << endl;
        exit(0);
    }
}

void TypeChecker::visit(AssignStm* stm) {
    if (!env.check(stm->id)) {
        cerr << "Error: variable '" << stm->id << "' no declarada." << endl;
        exit(0);
    }

    Type* varType = env.lookup(stm->id);
    Type* expType = stm->e->accept(this);

    if (!varType->match(expType)) {
        cerr << "Error: tipos incompatibles en asignación a '" << stm->id << "'." << endl;
        exit(0);
    }
}

void TypeChecker::visit(ReturnStm* stm) {
    // Si no hay expresión, el return es equivalente a retornar unit
    Type* retType = (stm->e ? stm->e->accept(this) : t_unit);

    // El tipo que la función actual debería devolver
    Type* expected = currentFunctionReturnType;

    if (!retType->match(expected)) {
        cerr << "Error: tipo de retorno inválido. "
             << "Se esperaba " << expected->str()
             << " pero se obtuvo " << retType->str() << endl;
        exit(0);
    }
}


// ===========================================================
//   Expresiones
// ===========================================================

Type* TypeChecker::visit(BinaryExp* e) {
    Type* left  = e->left->accept(this);
    Type* right = e->right->accept(this);

    string key = makeOperatorKey(e->op, left, right);
    auto overload = operatorTable.find(key);
    if (overload != operatorTable.end()) {
        e->hasOverloadedOperator = true;
        e->overloadTarget = overload->second.functionName;
        e->type = overload->second.returnType;
        return overload->second.returnType;
    }

    e->hasOverloadedOperator = false;
    e->overloadTarget.clear();

    auto isInt = [&](Type* t) {
        return t->match(t_i8) || t->match(t_i16) || t->match(t_i32) || t->match(t_i64) ||
               t->match(t_u8) || t->match(t_u16) || t->match(t_u32) || t->match(t_u64);
    };

    auto isFloat = [&](Type* t) {
        return t->match(t_f32) || t->match(t_f64);
    };

    auto isNumeric = [&](Type* t) {
        return isInt(t) || isFloat(t);
    };

    Type* resultType = nullptr;

    switch (e->op) {
        case PLUS_OP:
        case MINUS_OP:
        case MUL_OP:
        case DIV_OP:
            if (!isNumeric(left) || !isNumeric(right)) {
                cerr << "Error: no existe sobrecarga para '" << Exp::binopToChar(e->op)
                     << "' con tipos " << typeToString(left) << " y " << typeToString(right) << ".\n";
                exit(0);
            }
            if (!left->match(right)) {
                cerr << "Error: las operaciones aritméticas requieren operandos del mismo tipo.\n";
                exit(0);
            }
            resultType = left;  // Simplificación: usar tipo izquierdo
            break;

        case GT_OP:
        case GE_OP:
        case LT_OP:
        case LE_OP:
            if (!isNumeric(left) || !isNumeric(right)) {
                cerr << "Error: comparación requiere operandos numéricos.\n";
                exit(0);
            }
            if (!left->match(right)) {
                cerr << "Error: comparación entre tipos distintos: "
                     << left->str() << " y " << right->str() << "\n";
                exit(0);
            }
            resultType = t_bool;
            break;

        case EQ_OP:
            if (!left->match(right)) {
                cerr << "Error: comparación de igualdad requiere operandos del mismo tipo.\n";
                exit(0);
            }
            resultType = t_bool;
            break;

        default:
            cerr << "Error: operador binario no soportado.\n";
            exit(0);
    }

    e->type = resultType;
    return resultType;
}


Type* TypeChecker::visit(NumberExp* e) {
    e->type = t_i32;
    return t_i32;
}

Type* TypeChecker::visit(FloatExp* e) {
    e->type = t_f64;
    return t_f64;
}

Type* TypeChecker::visit(BoolExp* e) {
    e->type = t_bool;
    return t_bool;
}

Type* TypeChecker::visit(IdExp* e) {
    if (!env.check(e->val)) {
        cerr << "Error: variable '" << e->val << "' no declarada." << endl;
        exit(0);
    }
    Type* t = env.lookup(e->val);
    e->type = t;
    return t;
}

Type* TypeChecker::visit(FCallExp* e) {
    vector<Type*> argTypes;
    for (auto* arg : e->args) {
        argTypes.push_back(arg->accept(this));
    }
    const FunctionSignature* sig = findFunctionOverload(e->name, argTypes);
    if (!sig) {
        cerr << "Error: no existe una función " << e->name << " con la firma solicitada." << endl;
        exit(0);
    }
    e->type = sig->returnType;
    e->name = sig->mangledName;
    return sig->returnType;
}
