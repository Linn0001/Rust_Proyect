#include "TypeChecker.h"
#include <iostream>
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

bool TypeChecker::isInt(Type* t) const {
    if (!t) return false;
    switch (t->ttype) {
        case Type::I8:
        case Type::I16:
        case Type::I32:
        case Type::I64:
        case Type::U8:
        case Type::U16:
        case Type::U32:
        case Type::U64:
            return true;
        default:
            return false;
    }
}

bool TypeChecker::isFloat(Type* t) const {
    if (!t) return false;
    return t->ttype == Type::F32 || t->ttype == Type::F64;
}

bool TypeChecker::isNumeric(Type* t) const {
    return isInt(t) || isFloat(t);
}

bool TypeChecker::promoteLiteralTo(Exp* expr, Type* target) {
    if (!target) return false;
    if (auto num = dynamic_cast<NumberExp*>(expr)) {
        if (isNumeric(target)) {
            num->type = target;
            return true;
        }
    } else if (auto fl = dynamic_cast<FloatExp*>(expr)) {
        if (isFloat(target)) {
            fl->type = target;
            return true;
        }
    }
    return false;
}

// ===========================================================
//   Registrar funciones globales
// ===========================================================

void TypeChecker::add_function(FunDec* fd) {
    if (functions.find(fd->name) != functions.end()) {
        cerr << "Error: función '" << fd->name << "' ya fue declarada." << endl;
        exit(0);
    }

    Type* returnType = Type::from_string(fd->type);

    if (!returnType) {
        cerr << "Error: tipo de retorno no válido en función '"
             << fd->name << "' (" << fd->type << ")." << endl;
        exit(0);
    }

    functions[fd->name] = returnType;
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

    for (size_t i = 0; i < f->pnames.size(); ++i) {
        Type* pt = new Type();
        if (!pt->set_basic_type(f->ptypes[i])) {
            cerr << "Error: tipo de parámetro inválido en función '" << f->name << "'." << endl;
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

    if (!varType->match(expType)) {
        if (promoteLiteralTo(stm->e, varType)) {
            expType = stm->e->type;
        }

        if (!varType->match(expType)) {
            cerr << "Error: tipos incompatibles en asignación a '" << stm->id << "'." << endl;
            exit(0);
        }
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
            if (!left->match(right)) {
                bool adjusted = false;
                if (promoteLiteralTo(e->left, right)) {
                    left = e->left->type;
                    adjusted = left->match(right);
                }
                if (!adjusted && promoteLiteralTo(e->right, left)) {
                    right = e->right->type;
                    adjusted = left->match(right);
                }
                if (!adjusted) {
                    cerr << "Error: los operandos deben ser del mismo tipo en la operación aritmética.\n";
                    exit(0);
                }
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
                bool adjusted = false;
                if (promoteLiteralTo(e->left, right)) {
                    left = e->left->type;
                    adjusted = left->match(right);
                }
                if (!adjusted && promoteLiteralTo(e->right, left)) {
                    right = e->right->type;
                    adjusted = left->match(right);
                }
                if (!adjusted) {
                    cerr << "Error: comparación entre tipos distintos: "
                         << left->str() << " y " << right->str() << "\n";
                    exit(0);
                }
            }
            resultType = t_bool;
            break;

        case EQ_OP:
            if (!left->match(right)) {
                bool adjusted = false;
                if (promoteLiteralTo(e->left, right)) {
                    left = e->left->type;
                    adjusted = left->match(right);
                }
                if (!adjusted && promoteLiteralTo(e->right, left)) {
                    right = e->right->type;
                    adjusted = left->match(right);
                }
                if (!adjusted) {
                    cerr << "Error: comparación de igualdad requiere operandos del mismo tipo.\n";
                    exit(0);
                }
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
    return env.lookup(e->val);
    return t;
}

Type* TypeChecker::visit(FCallExp* e) {
    auto it = functions.find(e->name);
    if (it == functions.end()) {
        cerr << "Error: llamada a función no declarada '" << e->name << "'." << endl;
        exit(0);
    }
    e->type = it->second;
    return it->second;
}
