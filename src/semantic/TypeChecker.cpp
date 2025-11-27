#include "Typechecker.h"
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
Type* TernaryExp::accept(TypeVisitor* v) { return v->visit(this); }


void IfStm::accept(TypeVisitor* v) { v->visit(this); }
void WhileStm::accept(TypeVisitor* v) { v->visit(this); }
void AssignStm::accept(TypeVisitor* v) { v->visit(this); }
void PrintStm::accept(TypeVisitor* v) { v->visit(this); }
void ReturnStm::accept(TypeVisitor* v) { v->visit(this); }
void ForStm::accept(TypeVisitor* v) { v->visit(this); }

void VarDec::accept(TypeVisitor* v) { v->visit(this); }
void FunDec::accept(TypeVisitor* v) { v->visit(this); }
void Body::accept(TypeVisitor* v) { v->visit(this); }
void Program::accept(TypeVisitor* v) { v->visit(this); }

// ===========================================================
//   Constructor del TypeChecker
// ===========================================================

TypeChecker::TypeChecker() {
    t_bool = new Type(Type::BOOL);
    t_i8   = new Type(Type::I8);
    t_i16  = new Type(Type::I16);
    t_i32  = new Type(Type::I32);
    t_i64  = new Type(Type::I64);
    t_u8   = new Type(Type::U8);
    t_u16  = new Type(Type::U16);
    t_u32  = new Type(Type::U32);
    t_u64  = new Type(Type::U64);
    t_f32  = new Type(Type::F32);
    t_f64  = new Type(Type::F64);
    t_unit = new Type(Type::UNIT);

    currentFunctionReturnType = t_unit;
}

// ===========================================================
//   Registrar funciones globales (tipo de retorno + args)
// ===========================================================

void TypeChecker::add_function(FunDec* fd) {
    if (functions.find(fd->name) != functions.end()) {
        cerr << "Error: función '" << fd->name << "' ya fue declarada." << endl;
        exit(0);
    }

    // Tipo de retorno
    Type* returnType = Type::from_string(fd->type);
    if (!returnType) {
        cerr << "Error: tipo de retorno no válido en función '"
             << fd->name << "' (" << fd->type << ")." << endl;
        exit(0);
    }
    functions[fd->name] = returnType;

    // Tipos de parámetros
    vector<Type*> args;
    for (size_t i = 0; i < fd->ptypes.size(); ++i) {
        Type* pt = Type::from_string(fd->ptypes[i]);
        if (!pt) {
            cerr << "Error: tipo de parámetro inválido en función '"
                 << fd->name << "' (" << fd->ptypes[i] << ")." << endl;
            exit(0);
        }
        args.push_back(pt);
    }
    functionArgs[fd->name] = args;
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
    // Primero registrar funciones (tipos)
    for (auto f : p->fdlist)
        add_function(f);

    env.add_level();
    for (auto v : p->vdlist)
        v->accept(this);
    for (auto f : p->fdlist)
        f->accept(this);
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
}

void TypeChecker::visit(FunDec* f) {
    Type* oldReturn = currentFunctionReturnType;

    Type* ret = new Type();
    if (!ret->set_basic_type(f->type)) {
        cerr << "Error: tipo de retorno inválido en función '" << f->name << "'." << endl;
        exit(0);
    }

    currentFunctionReturnType = ret;

    env.add_level();

    // Parámetros como variables en el entorno
    for (size_t i = 0; i < f->pnames.size(); ++i) {
        Type* pt = new Type();
        if (!pt->set_basic_type(f->ptypes[i])) {
            cerr << "Error: tipo de parámetro inválido en función '"
                 << f->name << "'." << endl;
            exit(0);
        }
        env.add_var(f->pnames[i], pt);
    }

    f->b->accept(this);

    env.remove_level();

    currentFunctionReturnType = oldReturn;
}


// ===========================================================
//   Sentencias
// ===========================================================

void TypeChecker::visit(IfStm* stm) {
    Type* condType = stm->cond->accept(this);

    if (!condType->match(t_bool)) {
        cerr << "Error: la condición del if debe ser bool." << endl;
        exit(0);
    }

    if (stm->then)
        stm->then->accept(this);

    if (stm->els)
        stm->els->accept(this);
}


void TypeChecker::visit(WhileStm* stm) {
    Type* condType = stm->cond->accept(this);

    if (!condType->match(t_bool)) {
        cerr << "Error: la condición del while debe ser bool." << endl;
        exit(0);
    }

    if (stm->b)
        stm->b->accept(this);
}


void TypeChecker::visit(PrintStm* stm) {
    Type* t = stm->e->accept(this);

    bool ok =
            t->match(t_bool) ||
            t->match(t_i8)  ||
            t->match(t_i16) ||
            t->match(t_i32) ||
            t->match(t_i64) ||
            t->match(t_u8)  ||
            t->match(t_u16) ||
            t->match(t_u32) ||
            t->match(t_u64) ||
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

    if (varType->match(expType)) {
        return;
    }

    // Permitir i32 <-> i64
    if ((varType->ttype == Type::I64 && expType->ttype == Type::I32) ||
        (varType->ttype == Type::I32 && expType->ttype == Type::I64)) {
        return;
    }

    cerr << "Error: tipos incompatibles en asignación a '"
         << stm->id << "'." << endl;
    exit(0);
}


void TypeChecker::visit(ReturnStm* stm) {
    Type* retType = (stm->e ? stm->e->accept(this) : t_unit);

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
                cerr << "Error: operación aritmética requiere operandos numéricos.\n";
                exit(0);
            }
            resultType = left;  // Simplificación
            break;

        case GT_OP:
        case GE_OP:
        case LT_OP:
        case LE_OP:
            if (!isNumeric(left) || !isNumeric(right)) {
                cerr << "Error: comparación requiere operandos numéricos.\n";
                exit(0);
            }
            if (!(isInt(left) && isInt(right)) &&
                !(isFloat(left) && isFloat(right))) {
                cerr << "Error: comparación entre tipos incompatibles: "
                     << left->str() << " y " << right->str() << "\n";
                exit(0);
            }
            resultType = t_bool;
            break;

        case EQ_OP:
            if (!(isInt(left) && isInt(right)) &&
                !(isFloat(left) && isFloat(right))) {
                cerr << "Error: comparación de igualdad requiere tipos compatibles.\n";
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

// ====== LLAMADAS A FUNCIÓN ======.

Type* TypeChecker::visit(FCallExp* e) {
    auto itRet = functions.find(e->name);
    if (itRet == functions.end()) {
        cerr << "Error: llamada a función no declarada '" << e->name << "'." << endl;
        exit(0);
    }

    auto itArgs = functionArgs.find(e->name);
    vector<Type*> params;
    if (itArgs != functionArgs.end()) {
        params = itArgs->second;
    }

    if (params.size() != e->args.size()) {
        cerr << "Error: número de argumentos inválido en llamada a '"
             << e->name << "'. Se esperaban " << params.size()
             << " pero se recibieron " << e->args.size() << ".\n";
        exit(0);
    }

    for (size_t i = 0; i < e->args.size(); ++i) {
        Type* argT = e->args[i]->accept(this);
        Type* parT = params[i];

        if (argT->match(parT)) continue;

        // permitimos i32 <-> i64 como antes
        if ((argT->ttype == Type::I32 && parT->ttype == Type::I64) ||
            (argT->ttype == Type::I64 && parT->ttype == Type::I32)) {
            continue;
        }

        cerr << "Error: tipo de argumento " << (i+1) << " inválido en llamada a '"
             << e->name << "'. Se esperaba " << parT->str()
             << " pero se obtuvo " << argT->str() << ".\n";
        exit(0);
    }

    e->type = itRet->second;
    return itRet->second;
}

Type* TypeChecker::visit(TernaryExp* e) {
    // 1) La condición debe ser bool
    Type* condType = e->cond->accept(this);
    if (!condType->match(t_bool)) {
        cerr << "Error: la condición del operador ternario debe ser bool.\n";
        exit(0);
    }

    // 2) Tipos de las dos ramas
    Type* thenType = e->thenExp->accept(this);
    Type* elseType = e->elseExp->accept(this);

    auto isInt = [&](Type* t) {
        return t->match(t_i8) || t->match(t_i16) || t->match(t_i32) || t->match(t_i64) ||
               t->match(t_u8) || t->match(t_u16) || t->match(t_u32) || t->match(t_u64);
    };

    auto isFloat = [&](Type* t) {
        return t->match(t_f32) || t->match(t_f64);
    };

    Type* resultType = nullptr;

    // 3) Si son exactamente iguales → ok
    if (thenType->match(elseType)) {
        resultType = thenType;
    }
        // 4) Permitimos promoción entre enteros (por simplicidad, i32 <-> i64, y cualquier par entero)
    else if (isInt(thenType) && isInt(elseType)) {
        // Reutilizamos la misma relajación que usas en asignaciones (i32<->i64)
        // Nos quedamos con el tipo de la rama "then" como referencia
        resultType = thenType;
    }
        // 5) Permitimos mezclar floats (f32, f64)
    else if (isFloat(thenType) && isFloat(elseType)) {
        // Si uno es f64, devolvemos f64
        if (thenType->ttype == Type::F64 || elseType->ttype == Type::F64) {
            resultType = t_f64;
        } else {
            resultType = t_f32;
        }
    }
    else {
        cerr << "Error: las ramas del operador ternario deben tener tipos compatibles. "
             << "then: " << thenType->str()
             << ", else: " << elseType->str() << "\n";
        exit(0);
    }

    e->type = resultType;
    return resultType;
}


void TypeChecker::visit(ForStm* stm) {
    if (!env.check(stm->id)) {
        cerr << "Error: variable de iteración '" << stm->id
             << "' no declarada." << endl;
        exit(0);
    }

    Type* varType   = env.lookup(stm->id);
    Type* startType = stm->start->accept(this);
    Type* endType   = stm->end->accept(this);

    auto isInt = [&](Type* t) {
        return t->match(t_i8) || t->match(t_i16) ||
               t->match(t_i32) || t->match(t_i64);
    };

    if (!isInt(varType) || !isInt(startType) || !isInt(endType)) {
        cerr << "Error: for requiere tipos enteros en iterador y rango." << endl;
        exit(0);
    }

    if (stm->b)
        stm->b->accept(this);
}