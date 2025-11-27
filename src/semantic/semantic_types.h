#ifndef RUST_PROJECT_SEMANTIC_TYPES_H
#define RUST_PROJECT_SEMANTIC_TYPES_H

#include <iostream>
#include <string>
#include <unordered_map>
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
        I8, I16, I32, I64,   // enteros con signo
        U8, U16, U32, U64,   // enteros sin signo
        F32, F64             // flotantes
    };

    static const char* type_names[13];

    TType ttype;

    Type() : ttype(NOTYPE) {}
    Type(TType tt) : ttype(tt) {}

    struct TypeInfo {
        int size;
        int align;
    };

    static inline unordered_map<Type::TType, TypeInfo> TYPE_TABLE = {
            { Type::BOOL, {1, 1} },
            { Type::I8,   {1, 1} },
            { Type::I16,  {2, 2} },
            { Type::I32,  {4, 4} },
            { Type::I64,  {8, 8} },
            { Type::U8,   {1, 1} },
            { Type::U16,  {2, 2} },
            { Type::U32,  {4, 4} },
            { Type::U64,  {8, 8} },
            { Type::F32,  {4, 4} },
            { Type::F64,  {8, 8} },
            { Type::UNIT, {0, 1} }
    };

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
        if (s == "()" || s == "unit") return UNIT;
        if (s == "bool") return BOOL;

        if (s == "i8")  return I8;
        if (s == "i16") return I16;
        if (s == "i32") return I32;
        if (s == "i64" || s == "long") return I64;

        if (s == "u8")  return U8;
        if (s == "u16") return U16;
        if (s == "u32") return U32;
        if (s == "u64") return U64;

        if (s == "f32") return F32;
        if (s == "f64") return F64;

        return NOTYPE;
    }

    string str() const {
        switch (ttype) {
            case I8:   return "i8";
            case I16:  return "i16";
            case I32:  return "i32";
            case I64:  return "i64";
            case U8:   return "u8";
            case U16:  return "u16";
            case U32:  return "u32";
            case U64:  return "u64";
            case F32:  return "f32";
            case F64:  return "f64";
            case BOOL: return "bool";
            case UNIT: return "unit";
            default:   return "notype";
        }
    }

    static Type* from_string(const string& s) {
        TType tt = string_to_type(s);
        if (tt == NOTYPE) return nullptr;
        return new Type(tt);
    }

    // Devuelve los bytes que ocupa el tipo (útil para CodeGen)
    static int sizeof_type(TType t) {
        auto it = TYPE_TABLE.find(t);
        if (it == TYPE_TABLE.end()) return -1;
        return it->second.size;
    }

    static int alignof_type(TType t) {
        auto it = TYPE_TABLE.find(t);
        if (it == TYPE_TABLE.end()) return -1;
        return it->second.align;
    }
};

// Nombre legible (para debugging)
inline const char* Type::type_names[13] = {
        "notype",
        "unit",
        "bool",
        "i8",
        "i16",
        "i32",
        "i64",
        "u8",
        "u16",
        "u32",
        "u64",
        "f32",
        "f64"
};

#endif // RUST_PROJECT_SEMANTIC_TYPES_H