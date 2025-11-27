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
int FloatExp::accept(Visitor* visitor)      { return visitor->visit(this); }
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
int TernaryExp::accept(Visitor* visitor)    { return visitor->visit(this); }

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

    // Variables globales
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

    // Funciones
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

    if (!stm->resolved) {
        stm->resolved = Type::from_string(stm->type);
        if (!stm->resolved) {
            cerr << "Error: tipo inválido para variable '" << var << "'" << endl;
            exit(1);
        }
    }

    Type* t = stm->resolved;

    if (!entornoFuncion) {
        // Global
        globalMemory[var] = {true, t};
    } else {
        // Local
        if (localTypes.find(var) == localTypes.end()) {
            localTypes[var] = t;
        }
    }

    // Inicialización
    if (!stm->name.empty() && stm->e != nullptr) {
        stm->e->accept(this);   // valor en %rax (o bits del double)

        if (!entornoFuncion) {
            int size = Type::sizeof_type(t->ttype);
            storeValue(var + "(%rip)", size, t->ttype);
        } else {
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
    union {
        double d;
        unsigned long long u;
    } conv;
    conv.d = exp->val;

    out << " movabsq $" << conv.u << ", %rax\n";
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
//                           GenCodeVisitor – Assign
///////////////////////////////////////////////////////////////////////////////////

int GenCodeVisitor::visit(AssignStm* stm) {
    stm->e->accept(this);

    Type* t = nullptr;
    int size = 8;

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

///////////////////////////////////////////////////////////////////////////////////
//                   GenCodeVisitor – BinaryExp (CORREGIDO FLOAT)
///////////////////////////////////////////////////////////////////////////////////

int GenCodeVisitor::visit(BinaryExp* exp) {
    auto isFloatType = [](Type* t) {
        return t && (t->ttype == Type::F32 || t->ttype == Type::F64);
    };

    bool opIsArith =
            (exp->op == PLUS_OP ||
             exp->op == MINUS_OP ||
             exp->op == MUL_OP  ||
             exp->op == DIV_OP);

    bool opIsCmp =
            (exp->op == GT_OP ||
             exp->op == GE_OP ||
             exp->op == LT_OP ||
             exp->op == LE_OP ||
             exp->op == EQ_OP);

    bool leftIsFloat  = false;
    bool rightIsFloat = false;

    // 1) Si son literales float
    if (dynamic_cast<FloatExp*>(exp->left))  leftIsFloat  = true;
    if (dynamic_cast<FloatExp*>(exp->right)) rightIsFloat = true;

    // 2) Si son IdExp, consulta localTypes/globalMemory para ver si son f32/f64
    if (auto idL = dynamic_cast<IdExp*>(exp->left)) {
        auto itL = localTypes.find(idL->val);
        if (itL != localTypes.end() && isFloatType(itL->second))
            leftIsFloat = true;

        auto itLg = globalMemory.find(idL->val);
        if (itLg != globalMemory.end() && isFloatType(itLg->second.second))
            leftIsFloat = true;
    }

    if (auto idR = dynamic_cast<IdExp*>(exp->right)) {
        auto itR = localTypes.find(idR->val);
        if (itR != localTypes.end() && isFloatType(itR->second))
            rightIsFloat = true;

        auto itRg = globalMemory.find(idR->val);
        if (itRg != globalMemory.end() && isFloatType(itRg->second.second))
            rightIsFloat = true;
    }

    bool isFloatOp      = (leftIsFloat || rightIsFloat);
    bool isFloatArith   = isFloatOp && opIsArith;
    bool isFloatCompare = isFloatOp && opIsCmp;

    // ============================================================
    // 1) ARITMÉTICA DE FLOTANTES (+ - * /)  – ya venías usando x87
    // ============================================================
    if (isFloatArith) {
        // left → bits double en %rax
        exp->left->accept(this);
        out << " subq $16, %rsp\n";
        out << " movq %rax, 8(%rsp)\n";   // left

        // right → bits double en %rax
        exp->right->accept(this);
        out << " movq %rax, 0(%rsp)\n";   // right

        switch (exp->op) {
            case PLUS_OP:
                out << " fldl 8(%rsp)\n";
                out << " fldl 0(%rsp)\n";
                out << " faddp %st, %st(1)\n";
                out << " fstpl 0(%rsp)\n";
                break;
            case MINUS_OP:
                out << " fldl 8(%rsp)\n";
                out << " fldl 0(%rsp)\n";
                out << " fsubp %st, %st(1)\n";
                out << " fstpl 0(%rsp)\n";
                break;
            case MUL_OP:
                out << " fldl 8(%rsp)\n";
                out << " fldl 0(%rsp)\n";
                out << " fmulp %st, %st(1)\n";
                out << " fstpl 0(%rsp)\n";
                break;
            case DIV_OP:
                out << " fldl 8(%rsp)\n";
                out << " fldl 0(%rsp)\n";
                out << " fdivp %st, %st(1)\n";
                out << " fstpl 0(%rsp)\n";
                break;
            default:
                cerr << "Error: operador flotante no soportado en aritmética.\n";
                exit(1);
        }

        // Resultado en %rax (bits del double) y en %xmm0 (para printf)
        out << " movq 0(%rsp), %rax\n";
        out << " movq %rax, %xmm0\n";
        out << " addq $16, %rsp\n";
        return 0;
    }

    // ============================================================
    // 2) COMPARACIONES DE FLOTANTES (>, >=, <, <=, ==)
    //    Devuelve bool en %rax (0 o 1)
    // ============================================================
    if (isFloatCompare) {
        // Evaluamos left → bits en %rax, lo movemos a %xmm0
        exp->left->accept(this);
        out << " movq %rax, %xmm0\n";

        // Evaluamos right → bits en %rax, lo movemos a %xmm1
        exp->right->accept(this);
        out << " movq %rax, %xmm1\n";

        // ucomisd xmm1, xmm0  ; compara xmm0 (left) con xmm1 (right)
        out << " ucomisd %xmm1, %xmm0\n";
        out << " movl $0, %eax\n";

        switch (exp->op) {
            case GT_OP:  // left > right
                out << " seta %al\n";   // CF=0 && ZF=0
                break;
            case GE_OP:  // left >= right
                out << " setae %al\n";  // CF=0
                break;
            case LT_OP:  // left < right
                out << " setb %al\n";   // CF=1
                break;
            case LE_OP:  // left <= right
                out << " setbe %al\n";  // CF=1 || ZF=1
                break;
            case EQ_OP:  // left == right
                out << " sete %al\n";   // ZF=1
                break;
            default:
                cerr << "Error: operador de comparación flotante no soportado.\n";
                exit(1);
        }

        out << " movzbq %al, %rax\n";   // bool 0/1 en %rax
        return 0;
    }

    // ============================================================
    // 3) CAMINO ENTERO / BOOL ORIGINAL (lo dejamos tal cual)
    // ============================================================
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

    return 0;
}


///////////////////////////////////////////////////////////////////////////////////
//                           Ternary, Print, Body, If, While, Return, For, FunDec, FCall
//      (igual que tenías, solo los dejo como estaban)
///////////////////////////////////////////////////////////////////////////////////

int GenCodeVisitor::visit(TernaryExp* e) {
    int label = labelcont++;
    string elseLabel = "tern_else_" + to_string(label);
    string endLabel  = "tern_end_" + to_string(label);

    e->cond->accept(this);
    out << " cmpq $0, %rax\n";
    out << " je " << elseLabel << "\n";

    e->thenExp->accept(this);
    out << " jmp " << endLabel << "\n";

    out << elseLabel << ":\n";
    e->elseExp->accept(this);

    out << endLabel << ":\n";

    return 0;
}

int GenCodeVisitor::visit(PrintStm* stm) {
    stm->e->accept(this);

    if (stm->e->type) {
        if (stm->e->type->ttype == Type::F32 || stm->e->type->ttype == Type::F64) {
            out << " movq %rax, %xmm0\n";
            out << " leaq print_fmt_float(%rip), %rdi\n";
            out << " movl $1, %eax\n";
            out << " call printf\n";
        } else if (stm->e->type->ttype == Type::BOOL) {
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
            out << " movq %rax, %rsi\n";
            out << " leaq print_fmt_uint(%rip), %rdi\n";
            out << " movl $0, %eax\n";
            out << " call printf\n";
        } else {
            out << " movq %rax, %rsi\n";
            out << " leaq print_fmt_int(%rip), %rdi\n";
            out << " movl $0, %eax\n";
            out << " call printf\n";
        }
    } else {
        out << " movq %rax, %rsi\n";
        out << " leaq print_fmt_int(%rip), %rdi\n";
        out << " movl $0, %eax\n";
        out << " call printf\n";
    }

    return 0;
}

// Body
int GenCodeVisitor::visit(Body* b) {
    for (auto dec : b->decs)
        dec->accept(this);

    for (auto s : b->stmlist)
        s->accept(this);

    return 0;
}

// If
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

// While
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

// Return
int GenCodeVisitor::visit(ReturnStm* stm) {
    stm->e->accept(this);
    out << " jmp .end_" << nombreFuncion << "\n";
    return 0;
}

// For
int GenCodeVisitor::visit(ForStm* stm) {
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
    stm->start->accept(this);
    if (isGlobal) {
        storeValue(var + "(%rip)", size, t->ttype);
    } else {
        storeValue(to_string(offsetVar) + "(%rbp)", size, t->ttype);
    }

    int label = labelcont++;

    out << "for_" << label << ":\n";

    // i → %rcx
    if (isGlobal) {
        loadValue(var + "(%rip)", size, t->ttype);
    } else {
        loadValue(to_string(offsetVar) + "(%rbp)", size, t->ttype);
    }
    out << " movq %rax, %rcx\n";

    // end → %rax
    stm->end->accept(this);
    out << " cmpq %rax, %rcx\n";
    out << " jge endfor_" << label << "\n";

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

// FunDec
int GenCodeVisitor::visit(FunDec* f) {
    entornoFuncion = true;
    memory.clear();
    localTypes.clear();
    offset = -8;
    nombreFuncion = f->name;

    vector<string> argRegs = {"%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"};

    out << ".globl " << f->name << "\n";
    out << f->name << ":\n";
    out << " pushq %rbp\n";
    out << " movq %rsp, %rbp\n";

    // =========================
    // PARÁMETROS
    // =========================
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

    // =========================
    // LOCALES (RESERVA OFFSET)
    // =========================
    for (auto dec : f->b->decs) {
        if (!dec->resolved) {
            dec->resolved = Type::from_string(dec->type);
        }
        if (!dec->resolved) {
            cerr << "Error: tipo inválido para variable local '" << dec->name << "'" << endl;
            exit(1);
        }

        Type* t = dec->resolved;
        localTypes[dec->name] = t;

        int size  = Type::sizeof_type(t->ttype);
        int align = Type::alignof_type(t->ttype);

        if (size <= 0 || align <= 0) {
            cerr << "Error: tipo inválido para variable '" << dec->name << "'" << endl;
            exit(1);
        }

        int alignMask = align - 1;
        offset = (offset - size) & ~alignMask;

        memory[dec->name] = offset;
    }

    // Tamaño total de stack para esta función
    int stackSpace = (-offset);
    if (stackSpace % 16 != 0) {
        stackSpace = ((stackSpace + 15) / 16) * 16;
    }

    if (stackSpace > 0) {
        out << " subq $" << stackSpace << ", %rsp\n";
    }

    // =========================
    // TABLA DE DEBUG PARA VARIABLES LOCALES
    // =========================
    // Formato:  # DEBUG_VAR <nombre> <offset>
    // El AsmInterpreter leerá estas líneas y sabrá que offset -> nombre.
    out << " # DEBUG_VARS_BEGIN\n";
    for (const auto& [name, off] : memory) {
        out << " # DEBUG_VAR " << name << " " << off << "\n";
    }
    out << " # DEBUG_VARS_END\n";

    // =========================
    // INICIALIZAR LOCALES
    // =========================
    for (auto dec : f->b->decs) {
        dec->accept(this);
    }

    // =========================
    // CUERPO
    // =========================
    for (auto s : f->b->stmlist) {
        s->accept(this);
    }

    out << ".end_" << f->name << ":\n";
    out << " leave\n";
    out << " ret\n";

    entornoFuncion = false;
    return 0;
}


// FCallExp
int GenCodeVisitor::visit(FCallExp* exp) {
    vector<string> argRegs = {"%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"};

    for (int i = 0; i < (int)exp->args.size(); ++i) {
        exp->args[i]->accept(this);
        out << " movq %rax, " << argRegs[i] << "\n";
    }

    out << " call " << exp->name << "\n";
    return 0;
}

///////////////////////////////////////////////////////////////////////////////////
//                           FUNCIONES AUXILIARES load/store
///////////////////////////////////////////////////////////////////////////////////

void GenCodeVisitor::loadValue(const string& location, int size, Type::TType ttype) {
    switch(size) {
        case 1:
            if (ttype == Type::I8) {
                out << " movsbq " << location << ", %rax\n";
            } else {
                out << " movzbq " << location << ", %rax\n";
            }
            break;
        case 2:
            if (ttype == Type::I16) {
                out << " movswq " << location << ", %rax\n";
            } else {
                out << " movzwq " << location << ", %rax\n";
            }
            break;
        case 4:
            if (ttype == Type::I32) {
                out << " movslq " << location << ", %rax\n";
            } else {
                out << " movl " << location << ", %eax\n";
            }
            break;
        case 8:
            out << " movq " << location << ", %rax\n";
            break;
        default:
            cerr << "Error: tamaño no soportado para carga: " << size << endl;
            exit(1);
    }
}

void GenCodeVisitor::storeValue(const string& location, int size, Type::TType) {
    switch(size) {
        case 1:
            out << " movb %al, " << location << "\n";
            break;
        case 2:
            out << " movw %ax, " << location << "\n";
            break;
        case 4:
            out << " movl %eax, " << location << "\n";
            break;
        case 8:
            out << " movq %rax, " << location << "\n";
            break;
        default:
            cerr << "Error: tamaño no soportado para almacenamiento: " << size << endl;
            exit(1);
    }
}