#ifndef RUST_PROJECT_SEMANTIC_TYPES_H
#define RUST_PROJECT_SEMANTIC_TYPES_H

#include <iostream>
#include <string>
using namespace std;

// ===========================================================
//      REPRESENTACIÓN DE TIPOS DEL LENGUAJE (ESTILO RUST)
// ===========================================================

class Type {
public:
    // Tipos primitivos soportados
    enum TType {
        NOTYPE,   // Error o desconocido
        UNIT,     // () en Rust
        BOOL,     // bool
        I8, I16, I32, I64,   // enteros
        F32, F64             // flotantes
    };

    static const char* type_names[10];

    TType ttype;

    Type() : ttype(NOTYPE) {}
    Type(TType tt) : ttype(tt) {}

    // Comparación por igualdad
    bool match(Type* t) const {
        return this->ttype == t->ttype;
    }

    // Setter desde string (proveniente del parser)
    bool set_basic_type(const string& s) {
        TType tt = string_to_type(s);
        if (tt == NOTYPE) return false;
        ttype = tt;
        return true;
    }

    // Conversión string → enum
    static TType string_to_type(const string& s) {
        if (s == "()")    return UNIT;

        if (s == "bool")  return BOOL;

        if (s == "i8")    return I8;
        if (s == "i16")   return I16;
        if (s == "i32")   return I32;
        if (s == "i64")   return I64;

        if (s == "f32")   return F32;
        if (s == "f64")   return F64;

        return NOTYPE;
    }

    string str() const {
        switch (ttype) {
            case I8:   return "i8";
            case I16:  return "i16";
            case I32:  return "i32";
            case I64:  return "i64";
            case F32:  return "f32";
            case F64:  return "f64";
            case BOOL: return "bool";
            case UNIT: return "unit";
            default:   return "notype";
        }
    }

    static Type* from_string(const string& s) {
        if (s == "bool") return new Type(BOOL);
        if (s == "i8")   return new Type(I8);
        if (s == "i16")  return new Type(I16);
        if (s == "i32")  return new Type(I32);
        if (s == "i64")  return new Type(I64);
        if (s == "f32")  return new Type(F32);
        if (s == "f64")  return new Type(F64);
        if (s == "unit") return new Type(UNIT);
        return nullptr;
    }

    // Devuelve los bytes que ocupa el tipo (útil para CodeGen)
    static int sizeof_type(TType t) {
        switch (t) {
            case BOOL:  return 1;
            case I8:    return 1;
            case I16:   return 2;
            case I32:   return 4;
            case I64:   return 8;
            case F32:   return 4;
            case F64:   return 8;
            case UNIT:  return 0; // () no ocupa nada
            default:    return -1;
        }
    }
};

// Nombre legible (para debugging)
inline const char* Type::type_names[10] = {
    "notype",
    "unit",
    "bool",
    "i8",
    "i16",
    "i32",
    "i64",
    "f32",
    "f64"
};

#endif // RUST_PROJECT_SEMANTIC_TYPES_H
