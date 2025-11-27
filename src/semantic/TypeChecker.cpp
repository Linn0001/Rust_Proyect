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

    auto isInt = [&](Type* t) {
        return t->match(t_i8)  || t->match(t_i16) || t->match(t_i32) || t->match(t_i64) ||
               t->match(t_u8)  || t->match(t_u16) || t->match(t_u32) || t->match(t_u64);
    };

    bool ok = varType->match(expType);

    // 🔧 Flexibilizar literales numéricos: permitir que un literal entero
    // se asigne a cualquier tipo entero (i32, i64, u32, etc.)
    if (!ok) {
        if (stm->e->isNumberLiteral() && isInt(varType) && isInt(expType)) {
            ok = true;
            // Opcional: fijar el tipo concreto del literal al de la variable
            stm->e->type = varType;
        }
    }

    if (!ok) {
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

    // ----- pequeños helpers de tipos -----
    auto isInt = [&](Type* t) {
        return t->match(t_i8)  || t->match(t_i16) || t->match(t_i32) || t->match(t_i64) ||
               t->match(t_u8)  || t->match(t_u16) || t->match(t_u32) || t->match(t_u64);
    };

    auto isSignedInt = [&](Type* t) {
        return t->match(t_i8) || t->match(t_i16) || t->match(t_i32) || t->match(t_i64);
    };

    auto isUnsignedInt = [&](Type* t) {
        return t->match(t_u8) || t->match(t_u16) || t->match(t_u32) || t->match(t_u64);
    };

    auto isFloat = [&](Type* t) {
        return t->match(t_f32) || t->match(t_f64);
    };

    auto isNumeric = [&](Type* t) {
        return isInt(t) || isFloat(t);
    };

    auto isBool = [&](Type* t) {
        return t->match(t_bool);
    };

    // Dado dos enteros, escoger el tipo entero "común"
    auto commonIntType = [&](Type* a, Type* b) -> Type* {
        if (!isInt(a) || !isInt(b)) return nullptr;

        int sizeA = Type::sizeof_type(a->ttype);  // 1,2,4,8 bytes
        int sizeB = Type::sizeof_type(b->ttype);
        int size  = max(sizeA, sizeB);

        bool signedA = isSignedInt(a);
        bool signedB = isSignedInt(b);
        bool resultSigned = signedA || signedB;   // si alguno es signed, el resultado lo tratamos como signed

        if (resultSigned) {
            if (size <= 1) return t_i8;
            if (size <= 2) return t_i16;
            if (size <= 4) return t_i32;
            return t_i64;
        } else {
            if (size <= 1) return t_u8;
            if (size <= 2) return t_u16;
            if (size <= 4) return t_u32;
            return t_u64;
        }
    };

    Type* resultType = nullptr;

    switch (e->op) {
        // -----------------------------
        // Operaciones aritméticas
        // -----------------------------
        case PLUS_OP:
        case MINUS_OP:
        case MUL_OP:
        case DIV_OP:
            if (!isNumeric(left) || !isNumeric(right)) {
                cerr << "Error: operación aritmética requiere operandos numéricos.\n";
                exit(0);
            }

            if (isFloat(left) || isFloat(right)) {
                // No mezclamos de momento int con float
                if (!(isFloat(left) && isFloat(right))) {
                    cerr << "Error: no se permite operar entre enteros y flotantes.\n";
                    exit(0);
                }

                // Si alguno es f64, resultado f64; si no, f32
                if (left->match(t_f64) || right->match(t_f64)) {
                    resultType = t_f64;
                } else {
                    resultType = t_f32;
                }
            } else {
                // Ambos son enteros, posiblemente de distinto tamaño/signo
                resultType = commonIntType(left, right);
            }
            break;

        // -----------------------------
        // Comparaciones >, >=, <, <=
        // -----------------------------
        case GT_OP:
        case GE_OP:
        case LT_OP:
        case LE_OP:
            if (!isNumeric(left) || !isNumeric(right)) {
                cerr << "Error: comparación requiere operandos numéricos.\n";
                exit(0);
            }

            // Permitimos int-int o float-float, pero no mezclados
            if ((isInt(left) && isInt(right)) ||
                (isFloat(left) && isFloat(right))) {
                resultType = t_bool;
            } else {
                cerr << "Error: comparación entre tipos incompatibles: "
                     << left->str() << " y " << right->str() << "\n";
                exit(0);
            }
            break;

        // -----------------------------
        // Igualdad ==
        // -----------------------------
        case EQ_OP:
            // Igualdad numérica: int-int o float-float
            if ((isInt(left) && isInt(right)) ||
                (isFloat(left) && isFloat(right))) {
                resultType = t_bool;
            }
            // Igualdad booleana
            else if (isBool(left) && isBool(right)) {
                resultType = t_bool;
            } else {
                cerr << "Error: comparación de igualdad requiere tipos compatibles.\n";
                exit(0);
            }
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

void TypeChecker::visit(ForStm* stm) {
    // variable de iteración debe existir
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

    // Cuerpo del for
    if (stm->b)
        stm->b->accept(this);
}
