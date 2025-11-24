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
// CastExp, TernaryExp si los usas:
// int CastExp::accept(Visitor* visitor)    { return visitor->visit(this); }
// int TernaryExp::accept(Visitor* visitor){ return visitor->visit(this); }

///////////////////////////////////////////////////////////////////////////////////
//                               GenCodeVisitor — Helpers
///////////////////////////////////////////////////////////////////////////////////

int GenCodeVisitor::generar(Program* program) {
    program->accept(this);
    return 0;
}

///////////////////////////////////////////////////////////////////////////////////
//                               GenCodeVisitor — Program
///////////////////////////////////////////////////////////////////////////////////

int GenCodeVisitor::visit(Program* program) {
    out << ".data\n";
    out << "print_fmt: .string \"%lld \\n\"\n";

    // Variables globales
    for (auto dec : program->vdlist)
        dec->accept(this);

    for (auto& [var, _] : globalMemory)
        out << var << ": .quad 0\n";

    out << "\n.text\n";

    // Declaraciones de funciones
    for (auto dec : program->fdlist)
        dec->accept(this);

    out << ".section .note.GNU-stack,\"\",@progbits\n";
    return 0;
}

///////////////////////////////////////////////////////////////////////////////////
//                           GenCodeVisitor — VarDec
///////////////////////////////////////////////////////////////////////////////////

int GenCodeVisitor::visit(VarDec* stm) {
    int i = 0;

    auto var = stm->name;

    // Global o local
    if (!entornoFuncion) {
        globalMemory[var] = true;
    }
    else {
        if (!memory.count(var)) {
            memory[var] = offset;
            offset -= 8;
        }
    }

    // Inicialización si viene con expresión
    if (i < (int)stm->exps.size() && stm->exps[i] != nullptr) {
        stm->exps[i]->accept(this);

        if (!entornoFuncion)
            out << " movq %rax, " << var << "(%rip)\n";
        else
            out << " movq %rax, " << memoria[var] << "(%rbp)\n";
    }
    ++i;

    return 0;
}

///////////////////////////////////////////////////////////////////////////////////
//                           GenCodeVisitor — Expresiones
///////////////////////////////////////////////////////////////////////////////////

int GenCodeVisitor::visit(NumberExp* exp) {
    out << " movq $" << exp->value << ", %rax\n";
    return 0;
}

int GenCodeVisitor::visit(IdExp* exp) {
    if (memoriaGlobal.count(exp->value))
        out << " movq " << exp->value << "(%rip), %rax\n";
    else
        out << " movq " << memoria[exp->value] << "(%rbp), %rax\n";
    return 0;
}

int GenCodeVisitor::visit(BinaryExp* exp) {
    exp->left->accept(this);
    out << " pushq %rax\n";

    exp->right->accept(this);
    out << " movq %rax, %rcx\n popq %rax\n";

    switch (exp->op) {
        case PLUS_OP:  out << " addq %rcx, %rax\n"; break;
        case MINUS_OP: out << " subq %rcx, %rax\n"; break;
        case MUL_OP:   out << " imulq %rcx, %rax\n"; break;

        case LT_OP:
            out <<
                " cmpq %rcx, %rax\n"
                " movl $0, %eax\n"
                " setl %al\n"
                " movzbq %al, %rax\n";
            break;

        case LE_OP:
            out <<
                " cmpq %rcx, %rax\n"
                " movl $0, %eax\n"
                " setle %al\n"
                " movzbq %al, %rax\n";
            break;

        case EQ_OP:
            out <<
                " cmpq %rcx, %rax\n"
                " movl $0, %eax\n"
                " sete %al\n"
                " movzbq %al, %rax\n";
            break;
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////////
//                           GenCodeVisitor — Statements
///////////////////////////////////////////////////////////////////////////////////

int GenCodeVisitor::visit(AssignStm* stm) {
    stm->e->accept(this);

    if (memoriaGlobal.count(stm->id))
        out << " movq %rax, " << stm->id << "(%rip)\n";
    else
        out << " movq %rax, " << memoria[stm->id] << "(%rbp)\n";
    return 0;
}

int GenCodeVisitor::visit(PrintStm* stm) {
    stm->e->accept(this);

    out <<
        " subq $32, %rsp\n"
        " movq %rax, %rdx\n"
        " leaq print_fmt(%rip), %rcx\n"
        " movl $0, %eax\n"
        " call printf\n"
        " addq $32, %rsp\n";

    return 0;
}

int GenCodeVisitor::visit(Body* b) {
    for (auto dec : b->declarations)
        dec->accept(this);

    for (auto s : b->StmList)
        s->accept(this);

    return 0;
}

int GenCodeVisitor::visit(IfStm* stm) {
    int label = labelcont++;

    stm->condition->accept(this);
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

    stm->condition->accept(this);
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

///////////////////////////////////////////////////////////////////////////////////
//                           GenCodeVisitor — Funciones y Calls
///////////////////////////////////////////////////////////////////////////////////

int GenCodeVisitor::visit(FunDec* f) {
    entornoFuncion = true;
    memoria.clear();
    offset = -8;
    nombreFuncion = f->nombre;

    vector<string> argRegs = {"%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"};

    // Setup
    out << ".globl " << f->nombre << "\n";
    out << f->nombre << ":\n";
    out << " pushq %rbp\n";
    out << " movq %rsp, %rbp\n";

    // Argumentos
    for (int i = 0; i < (int)f->Pnombres.size(); ++i) {
        memoria[f->Pnombres[i]] = offset;
        out << " movq " << argRegs[i] << ", " << offset << "(%rbp)\n";
        offset -= 8;
    }

    // Reservar espacio local
    long totalLocals = 0;
    for (auto dec : f->cuerpo->declarations)
        totalLocals += dec->vars.size();

    if (totalLocals > 0)
        out << " subq $" << (totalLocals * 8) << ", %rsp\n";

    // Inicializar locales
    for (auto dec : f->cuerpo->declarations)
        dec->accept(this);

    // Ejecutar cuerpo
    for (auto s : f->cuerpo->StmList)
        s->accept(this);

    out << ".end_" << f->nombre << ":\n";
    out << " leave\n";
    out << " ret\n";

    entornoFuncion = false;
    return 0;
}

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
//                           GenCodeVisitor — Otros (Ternario / Cast)
///////////////////////////////////////////////////////////////////////////////////

int GenCodeVisitor::visit(CastExp* exp) {
    exp->expr->accept(this);
    return 0;
}

int GenCodeVisitor::visit(TernaryExp* exp) {
    int label = labelcont++;
    int endlabel = labelcont++;

    // Condición
    exp->cond->accept(this);
    out << " cmpq $0, %rax\n";
    out << " je ternary_else_" << label << "\n";

    // then
    exp->thenExp->accept(this);
    out << " jmp ternary_end_" << label << "\n";

    // else
    out << "ternary_else_" << label << ":\n";
    exp->elseExp->accept(this);

    out << "ternary_end_" << label << ":\n";
    return 0;
}
