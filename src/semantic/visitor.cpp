#include <iostream>
#include "../syntactic/ast.h"
#include "visitor.h"
#include <unordered_map>

using namespace std;

///////////////////////////////////////////////////////////////////////////////////
//                          MÉTODOS accept() DE CADA NODO
///////////////////////////////////////////////////////////////////////////////////

int BinaryExp::accept(Visitor* visitor)     { return visitor->visit(this); }
int NumberExp::accept(Visitor* visitor)     { return visitor->visit(this); }
int FloatExp::accept(Visitor* visitor)     { return visitor->visit(this); }
int BoolExp::accept(Visitor* visitor)       { return visitor->visit(this); }
int IdExp::accept(Visitor* visitor)         { return visitor->visit(this); }
int PrintStm::accept(Visitor* visitor)      { return visitor->visit(this); }
int AssignStm::accept(Visitor* visitor)     { return visitor->visit(this); }
int IfStm::accept(Visitor* visitor)         { return visitor->visit(this); }
int WhileStm::accept(Visitor* visitor)      { return visitor->visit(this); }
int Body::accept(Visitor* visitor)          { return visitor->visit(this); }
int VarDec::accept(Visitor* visitor)        { return visitor->visit(this); }
int FCallExp::accept(Visitor* visitor)      { return visitor->visit(this); }
int FunDec::accept(Visitor* visitor)        { return visitor->visit(this); }
int Program::accept(Visitor* visitor)       { return visitor->visit(this); }
int ReturnStm::accept(Visitor* visitor)     { return visitor->visit(this); }
int ForStm::accept(Visitor *visitor)        { return visitor->visit(this); }

///////////////////////////////////////////////////////////////////////////////////
//                               GenCodeVisitor – Helpers
///////////////////////////////////////////////////////////////////////////////////

int GenCodeVisitor::generar(Program* program) {
    program->accept(this);
    return 0;
}

///////////////////////////////////////////////////////////////////////////////////
//                               GenCodeVisitor – Program
///////////////////////////////////////////////////////////////////////////////////

int GenCodeVisitor::visit(Program* program) {
    out << ".data\n";
    out << "print_fmt_int: .string \"%lld\\n\"\n";
    out << "print_fmt_uint: .string \"%llu\\n\"\n";
    out << "print_fmt_float: .string \"%.6f\\n\"\n";
    out << "print_fmt_bool_true: .string \"true\\n\"\n";
    out << "print_fmt_bool_false: .string \"false\\n\"\n";

    // Variables globales con tamaño correcto
    for (auto dec : program->vdlist) {
        dec->accept(this);
    }

    for (auto& [var, info] : globalMemory) {
        Type* t = info.second;
        int size = Type::sizeof_type(t->ttype);

        switch(size) {
            case 1: out << var << ": .byte 0\n"; break;
            case 2: out << var << ": .word 0\n"; break;
            case 4: out << var << ": .long 0\n"; break;
            case 8: out << var << ": .quad 0\n"; break;
            default:
                cerr << "Error: tamaño no soportado para global: " << size << endl;
                exit(1);
        }
    }

    out << "\n.text\n";

    // Declaraciones de funciones
    for (auto dec : program->fdlist)
        dec->accept(this);

    out << ".section .note.GNU-stack,\"\",@progbits\n";
    return 0;
}

///////////////////////////////////////////////////////////////////////////////////
//                           GenCodeVisitor – VarDec
///////////////////////////////////////////////////////////////////////////////////

int GenCodeVisitor::visit(VarDec* stm) {
    string var = stm->name;

    // Asegurar que el tipo está resuelto
    if (!stm->resolved) {
        stm->resolved = Type::from_string(stm->type);
        if (!stm->resolved) {
            cerr << "Error: tipo inválido para variable '" << var << "'" << endl;
            exit(1);
        }
    }

    Type* t = stm->resolved;

    // Global o local
    if (!entornoFuncion) {
        globalMemory[var] = {true, t};
    }
    else {
        // Las variables locales ya tienen espacio reservado en visit(FunDec)
        // Solo las registramos en localTypes si no están ya
        if (localTypes.find(var) == localTypes.end()) {
            localTypes[var] = t;
        }
    }

    // Inicializar la variable si tiene expresión de inicialización
    if (!stm->name.empty() && stm->e != nullptr) {
        stm->e->accept(this);   // valor queda en %rax

        if (!entornoFuncion) {
            // Variable global
            int size = Type::sizeof_type(t->ttype);
            storeValue(var + "(%rip)", size, t->ttype);
        }
        else {
            // Variable local
            int size = Type::sizeof_type(t->ttype);
            storeValue(to_string(memory[var]) + "(%rbp)", size, t->ttype);
        }
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////////
//                           GenCodeVisitor – Expresiones
///////////////////////////////////////////////////////////////////////////////////

int GenCodeVisitor::visit(NumberExp* exp) {
    out << " movq $" << exp->val << ", %rax\n";
    return 0;
}

int GenCodeVisitor::visit(FloatExp* exp) {
    // Cargar valor flotante a registro XMM
    // Necesitamos ponerlo en memoria primero
    out << " movq $" << *(long long*)&exp->val << ", %rax\n";
    out << " movq %rax, %xmm0\n";
    return 0;
}

int GenCodeVisitor::visit(BoolExp* exp) {
    out << " movq $" << (exp->val ? 1 : 0) << ", %rax\n";
    return 0;
}

int GenCodeVisitor::visit(IdExp* exp) {
    Type* t = nullptr;
    int size = 8;

    // Buscar el tipo de la variable
    if (globalMemory.count(exp->val)) {
        t = globalMemory[exp->val].second;
        size = Type::sizeof_type(t->ttype);
        loadValue(exp->val + "(%rip)", size, t->ttype);
    }
    else if (localTypes.count(exp->val)) {
        t = localTypes[exp->val];
        size = Type::sizeof_type(t->ttype);
        loadValue(to_string(memory[exp->val]) + "(%rbp)", size, t->ttype);
    }
    else {
        cerr << "Error: variable no encontrada: " << exp->val << endl;
        exit(1);
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////////
//                           GenCodeVisitor – Statements
///////////////////////////////////////////////////////////////////////////////////

int GenCodeVisitor::visit(AssignStm* stm) {
    stm->e->accept(this);

    Type* t = nullptr;
    int size = 8;

    // Buscar el tipo de la variable
    if (globalMemory.count(stm->id)) {
        t = globalMemory[stm->id].second;
        size = Type::sizeof_type(t->ttype);
        storeValue(stm->id + "(%rip)", size, t->ttype);
    }
    else if (localTypes.count(stm->id)) {
        t = localTypes[stm->id];
        size = Type::sizeof_type(t->ttype);
        storeValue(to_string(memory[stm->id]) + "(%rbp)", size, t->ttype);
    }
    else {
        cerr << "Error: variable no encontrada en asignación: " << stm->id << endl;
        exit(1);
    }

    return 0;
}

int GenCodeVisitor::visit(BinaryExp* exp) {
    // Verificar si es operación con flotantes
    bool isFloat = (exp->left->type &&
                    (exp->left->type->ttype == Type::F32 ||
                     exp->left->type->ttype == Type::F64));

    if (isFloat) {
        // Código para flotantes
        exp->left->accept(this);
        out << " movq %rax, %xmm0\n";
        out << " pushq %rax\n";

        exp->right->accept(this);
        out << " movq %rax, %xmm1\n";
        out << " popq %rax\n";

        switch (exp->op) {
            // -------------------------
            // Aritmética con flotantes
            // -------------------------
            case PLUS_OP:
                out << " addsd %xmm1, %xmm0\n";
                out << " movq %xmm0, %rax\n";
                break;

            case MINUS_OP:
                out << " subsd %xmm1, %xmm0\n";
                out << " movq %xmm0, %rax\n";
                break;

            case MUL_OP:
                out << " mulsd %xmm1, %xmm0\n";
                out << " movq %xmm0, %rax\n";
                break;

            case DIV_OP:
                out << " divsd %xmm1, %xmm0\n";
                out << " movq %xmm0, %rax\n";
                break;

            // -------------------------
            // Comparaciones con flotantes usando ucomisd
            // -------------------------
            // ucomisd compara dos doubles y setea flags:
            // - ZF (Zero Flag): 1 si son iguales
            // - PF (Parity Flag): 1 si alguno es NaN
            // - CF (Carry Flag): 1 si %xmm0 < %xmm1

            case GT_OP:  // %xmm0 > %xmm1
                out << " ucomisd %xmm1, %xmm0\n";
                out << " seta %al\n";           // Set if above (CF=0 and ZF=0)
                out << " movzbq %al, %rax\n";   // Zero-extend al a rax
                break;

            case GE_OP:  // %xmm0 >= %xmm1
                out << " ucomisd %xmm1, %xmm0\n";
                out << " setae %al\n";          // Set if above or equal (CF=0)
                out << " movzbq %al, %rax\n";
                break;

            case LT_OP:  // %xmm0 < %xmm1
                // Para A < B, comparamos B con A y usamos seta (above)
                out << " ucomisd %xmm0, %xmm1\n";  // Compara %xmm1 con %xmm0
                out << " seta %al\n";              // Set if %xmm1 > %xmm0 (es decir, %xmm0 < %xmm1)
                out << " movzbq %al, %rax\n";
                break;

            case LE_OP:  // %xmm0 <= %xmm1
                // Para A <= B, comparamos B con A y usamos setae (above or equal)
                out << " ucomisd %xmm0, %xmm1\n";  // Compara %xmm1 con %xmm0
                out << " setae %al\n";             // Set if %xmm1 >= %xmm0 (es decir, %xmm0 <= %xmm1)
                out << " movzbq %al, %rax\n";
                break;

            case EQ_OP:  // %xmm0 == %xmm1
                out << " ucomisd %xmm1, %xmm0\n";
                out << " sete %al\n";           // Set if equal (ZF=1)
                out << " setnp %cl\n";          // Set if not parity (not NaN)
                out << " andb %cl, %al\n";      // AND ambos (igual Y no-NaN)
                out << " movzbq %al, %rax\n";
                break;

            default:
                cerr << "Error: operador no soportado para flotantes.\n";
                exit(1);
        }
    } else {
        // Código para enteros (el que ya tienes)
        exp->left->accept(this);
        out << " pushq %rax\n";
        exp->right->accept(this);
        out << " movq %rax, %rcx\n";
        out << " popq %rax\n";

        switch (exp->op) {
            case PLUS_OP:
                out << " addq %rcx, %rax\n";
                break;
            case MINUS_OP:
                out << " subq %rcx, %rax\n";
                break;
            case MUL_OP:
                out << " imulq %rcx, %rax\n";
                break;
            case DIV_OP:
                out << " cqto\n";
                out << " idivq %rcx\n";
                break;

            case GT_OP:
                out << " cmpq %rcx, %rax\n";
                out << " movl $0, %eax\n";
                out << " setg %al\n";
                out << " movzbq %al, %rax\n";
                break;

            case GE_OP:
                out << " cmpq %rcx, %rax\n";
                out << " movl $0, %eax\n";
                out << " setge %al\n";
                out << " movzbq %al, %rax\n";
                break;

            case LT_OP:
                out << " cmpq %rcx, %rax\n";
                out << " movl $0, %eax\n";
                out << " setl %al\n";
                out << " movzbq %al, %rax\n";
                break;

            case LE_OP:
                out << " cmpq %rcx, %rax\n";
                out << " movl $0, %eax\n";
                out << " setle %al\n";
                out << " movzbq %al, %rax\n";
                break;

            case EQ_OP:
                out << " cmpq %rcx, %rax\n";
                out << " movl $0, %eax\n";
                out << " sete %al\n";
                out << " movzbq %al, %rax\n";
                break;
        }
    }

    return 0;
}

int GenCodeVisitor::visit(PrintStm* stm) {
    stm->e->accept(this);

    // Detectar el tipo de la expresión
    if (stm->e->type) {
        if (stm->e->type->ttype == Type::F32 || stm->e->type->ttype == Type::F64) {
            // Imprimir flotante
            out << " movq %rax, %xmm0\n";
            out << " leaq print_fmt_float(%rip), %rdi\n";
            out << " movl $1, %eax\n";  // 1 registro XMM usado
            out << " call printf\n";
        } else if (stm->e->type->ttype == Type::BOOL) {
            // Imprimir booleano
            int label = labelcont++;
            out << " cmpq $0, %rax\n";
            out << " je .print_false_" << label << "\n";
            out << " leaq print_fmt_bool_true(%rip), %rdi\n";
            out << " jmp .print_bool_" << label << "\n";
            out << ".print_false_" << label << ":\n";
            out << " leaq print_fmt_bool_false(%rip), %rdi\n";
            out << ".print_bool_" << label << ":\n";
            out << " movl $0, %eax\n";
            out << " call printf\n";
        } else if (stm->e->type->ttype == Type::U8 || stm->e->type->ttype == Type::U16 ||
               stm->e->type->ttype == Type::U32 || stm->e->type->ttype == Type::U64) {
            // Imprimir entero sin signo
            out << " movq %rax, %rsi\n";
            out << " leaq print_fmt_uint(%rip), %rdi\n";
            out << " movl $0, %eax\n";
            out << " call printf\n";
        } else {
            // Imprimir entero
            out << " movq %rax, %rsi\n";
            out << " leaq print_fmt_int(%rip), %rdi\n";
            out << " movl $0, %eax\n";
            out << " call printf\n";
        }
    } else {
        // Fallback: asumir entero
        out << " movq %rax, %rsi\n";
        out << " leaq print_fmt_int(%rip), %rdi\n";
        out << " movl $0, %eax\n";
        out << " call printf\n";
    }

    return 0;
}

int GenCodeVisitor::visit(Body* b) {
    for (auto dec : b->decs)
        dec->accept(this);

    for (auto s : b->stmlist)
        s->accept(this);

    return 0;
}

int GenCodeVisitor::visit(IfStm* stm) {
    int label = labelcont++;

    stm->cond->accept(this);
    out << " cmpq $0, %rax\n";
    out << " je else_" << label << "\n";

    stm->then->accept(this);
    out << " jmp endif_" << label << "\n";

    out << "else_" << label << ":\n";
    if (stm->els) stm->els->accept(this);

    out << "endif_" << label << ":\n";
    return 0;
}

int GenCodeVisitor::visit(WhileStm* stm) {
    int label = labelcont++;

    out << "while_" << label << ":\n";

    stm->cond->accept(this);
    out << " cmpq $0, %rax\n";
    out << " je endwhile_" << label << "\n";

    stm->b->accept(this);
    out << " jmp while_" << label << "\n";

    out << "endwhile_" << label << ":\n";
    return 0;
}

int GenCodeVisitor::visit(ReturnStm* stm) {
    stm->e->accept(this);
    out << " jmp .end_" << nombreFuncion << "\n";
    return 0;
}

int GenCodeVisitor::visit(ForStm* stm) {
    // for i in start..end { body }
    // Requiere que la variable 'i' ya exista como global o local.

    Type* t = nullptr;
    int size = 8;
    string var = stm->id;

    bool isGlobal = false;
    int offsetVar = 0;

    if (globalMemory.count(var)) {
        t = globalMemory[var].second;
        size = Type::sizeof_type(t->ttype);
        isGlobal = true;
    } else if (localTypes.count(var)) {
        t = localTypes[var];
        size = Type::sizeof_type(t->ttype);
        offsetVar = memory[var];
    } else {
        cerr << "Error: variable de iteración no declarada: " << var << endl;
        exit(1);
    }

    // i = start;
    stm->start->accept(this);  // deja inicio en %rax
    if (isGlobal) {
        storeValue(var + "(%rip)", size, t->ttype);
    } else {
        storeValue(to_string(offsetVar) + "(%rbp)", size, t->ttype);
    }

    int label = labelcont++;

    out << "for_" << label << ":\n";

    // Cargar i en %rcx
    if (isGlobal) {
        loadValue(var + "(%rip)", size, t->ttype); // -> %rax
    } else {
        loadValue(to_string(offsetVar) + "(%rbp)", size, t->ttype);
    }
    out << " movq %rax, %rcx\n";

    // Evaluar end en %rax
    stm->end->accept(this);   // %rax = end

    // while (i < end)
    out << " cmpq %rax, %rcx\n";
    out << " jge endfor_" << label << "\n";

    // Cuerpo del for
    stm->b->accept(this);

    // i = i + 1
    if (isGlobal) {
        loadValue(var + "(%rip)", size, t->ttype);
    } else {
        loadValue(to_string(offsetVar) + "(%rbp)", size, t->ttype);
    }
    out << " addq $1, %rax\n";

    if (isGlobal) {
        storeValue(var + "(%rip)", size, t->ttype);
    } else {
        storeValue(to_string(offsetVar) + "(%rbp)", size, t->ttype);
    }

    out << " jmp for_" << label << "\n";
    out << "endfor_" << label << ":\n";

    return 0;
}

///////////////////////////////////////////////////////////////////////////////////
//                           GenCodeVisitor – Funciones y Calls
///////////////////////////////////////////////////////////////////////////////////

int GenCodeVisitor::visit(FunDec* f) {
    entornoFuncion = true;
    memory.clear();
    localTypes.clear();
    offset = -8;
    nombreFuncion = f->name;

    // Registros de parámetros según System V ABI (Linux/Unix)
    vector<string> argRegs = {"%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"};

    // Setup
    out << ".globl " << f->name << "\n";
    out << f->name << ":\n";
    out << " pushq %rbp\n";
    out << " movq %rsp, %rbp\n";

    // ===== ARGUMENTOS =====
    // Los argumentos se guardan primero con 8 bytes cada uno
    for (int i = 0; i < (int)f->pnames.size(); ++i) {
        Type* paramType = Type::from_string(f->ptypes[i]);
        if (!paramType) {
            cerr << "Error: tipo de parámetro inválido: " << f->ptypes[i] << endl;
            exit(1);
        }

        memory[f->pnames[i]] = offset;
        localTypes[f->pnames[i]] = paramType;
        out << " movq " << argRegs[i] << ", " << offset << "(%rbp)\n";
        offset -= 8;
    }

    // ===== VARIABLES LOCALES CON TAMAÑO DINÁMICO =====
    for (auto dec : f->b->decs) {
        // Asegurarse de que el tipo está resuelto
        if (!dec->resolved) {
            dec->resolved = Type::from_string(dec->type);
        }

        if (!dec->resolved) {
            cerr << "Error: tipo inválido para variable local '" << dec->name << "'" << endl;
            exit(1);
        }

        Type* t = dec->resolved;
        localTypes[dec->name] = t;

        // Obtener tamaño y alineación del tipo
        int size = Type::sizeof_type(t->ttype);
        int align = Type::alignof_type(t->ttype);

        // Validar que el tipo sea válido
        if (size <= 0 || align <= 0) {
            cerr << "Error: tipo inválido para variable '" << dec->name << "'" << endl;
            exit(1);
        }

        // Alinear el offset según el requerimiento del tipo
        int alignMask = align - 1;
        offset = (offset - size) & ~alignMask;

        // Guardar la posición de esta variable
        memory[dec->name] = offset;
    }

    // Calcular espacio total necesario (debe ser múltiplo de 16 para ABI x86-64)
    int stackSpace = (-offset);
    if (stackSpace % 16 != 0) {
        stackSpace = ((stackSpace + 15) / 16) * 16;
    }

    // Reservar espacio en el stack para las variables locales
    if (stackSpace > 0) {
        out << " subq $" << stackSpace << ", %rsp\n";
    }

    // ===== INICIALIZAR VARIABLES LOCALES =====
    for (auto dec : f->b->decs) {
        dec->accept(this);
    }

    // ===== EJECUTAR CUERPO DE LA FUNCIÓN =====
    for (auto s : f->b->stmlist) {
        s->accept(this);
    }

    // ===== EPÍLOGO =====
    out << ".end_" << f->name << ":\n";
    out << " leave\n";
    out << " ret\n";

    entornoFuncion = false;
    return 0;
}

int GenCodeVisitor::visit(FCallExp* exp) {
    // Registros de argumentos según System V ABI (Linux/Unix)
    vector<string> argRegs = {"%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"};

    for (int i = 0; i < (int)exp->args.size(); ++i) {
        exp->args[i]->accept(this);
        out << " movq %rax, " << argRegs[i] << "\n";
    }

    out << " call " << exp->name << "\n";
    return 0;
}

///////////////////////////////////////////////////////////////////////////////////
//                           FUNCIONES AUXILIARES
///////////////////////////////////////////////////////////////////////////////////

void GenCodeVisitor::loadValue(const string& location, int size, Type::TType ttype) {
    switch(size) {
        case 1:  // i8, u8, bool
            if (ttype == Type::I8) {
                out << " movsbq " << location << ", %rax\n"; // extensión con signo
            } else {
                out << " movzbq " << location << ", %rax\n"; // extensión cero para bool/u8
            }
            break;
        case 2:  // i16, u16
            if (ttype == Type::I16) {
                out << " movswq " << location << ", %rax\n";
            } else {
                out << " movzwq " << location << ", %rax\n";
            }
            break;
        case 4:  // i32, f32, u32
            // Para enteros con signo usar movslq
            if (ttype == Type::I32) {
                out << " movslq " << location << ", %rax\n";
            } else {
                out << " movl " << location << ", %eax\n";
            }
            break;
        case 8:  // i64, f64, u64
            out << " movq " << location << ", %rax\n";
            break;
        default:
            cerr << "Error: tamaño no soportado para carga: " << size << endl;
            exit(1);
    }
}

void GenCodeVisitor::storeValue(const string& location, int size, Type::TType ttype) {
    switch(size) {
        case 1:  // i8, bool
            out << " movb %al, " << location << "\n";
            break;
        case 2:  // i16
            out << " movw %ax, " << location << "\n";
            break;
        case 4:  // i32, f32
            out << " movl %eax, " << location << "\n";
            break;
        case 8:  // i64, f64
            out << " movq %rax, " << location << "\n";
            break;
        default:
            cerr << "Error: tamaño no soportado para almacenamiento: " << size << endl;
            exit(1);
    }
}