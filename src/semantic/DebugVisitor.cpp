#include "DebugVisitor.h"
#include <iostream>

using namespace std;

void DebugVisitor::run(Program* p) {
    cout << "\n[DEBUG] Iniciando depuración (CPU virtual sencilla)\n";

    // Reset del trace
    trace.vars.clear();
    trace.steps.clear();
    trace.nextOffset = 0;
    trace.regs = DebugRegisterState{};
    trace.regs.RBP = 0;
    trace.regs.RSP = 0;
    trace.regs.ZF  = 0;
    trace.regs.SF  = 0;
    lastValue = 0;

    // Variables globales
    for (auto vd : p->vdlist) {
        vd->accept(this);
    }

    // Buscar main
    FunDec* mainFun = nullptr;
    for (auto fd : p->fdlist) {
        if (fd->name == "main") {
            mainFun = fd;
            break;
        }
    }

    if (!mainFun) {
        cerr << "[DEBUG] Error: no se encontró función main().\n";
        return;
    }

    step("Antes de ejecutar main()");

    if (mainFun->b)
        mainFun->b->accept(this);

    cout << "\n[DEBUG] Fin de main().\n";
    showEnv();
}

long long DebugVisitor::eval(Exp* e) {
    if (!e) return 0;
    e->accept(this);
    trace.regs.RAX = lastValue;
    updateFlags(lastValue);
    return lastValue;
}

void DebugVisitor::showEnv() {
    cout << "  Estado actual de variables:\n";
    if (trace.vars.empty()) {
        cout << "    (sin variables todavía)\n";
        return;
    }
    for (auto& kv : trace.vars) {
        const string& name = kv.first;
        long long value    = kv.second.second;
        cout << "    " << name << " = " << value << "\n";
    }
}

void DebugVisitor::step(const string& msg) {
    cout << "\n[STEP] " << msg << "\n";
    showEnv();

    // Simulación de punteros de stack
    trace.regs.RBP = 0;
    trace.regs.RSP = trace.nextOffset;

    // modo AST: line = -1
    trace.snapshot(msg);
}

void DebugVisitor::updateFlags(long long val) {
    trace.regs.ZF = (val == 0) ? 1 : 0;
    trace.regs.SF = (val < 0) ? 1 : 0;
}

// ================== Implementaciones Visitor ==================

int DebugVisitor::visit(Program* p) {
    run(p);
    return 0;
}

int DebugVisitor::visit(FunDec* fd) {
    return 0;
}

int DebugVisitor::visit(Body* body) {
    for (auto vd : body->decs)
        vd->accept(this);
    for (auto s : body->stmlist)
        s->accept(this);
    return 0;
}

int DebugVisitor::visit(VarDec* vd) {
    long long init = 0;
    if (vd->e) init = eval(vd->e);
    trace.declareVar(vd->name, init);
    step("Declaración de variable " + vd->name);
    return 0;
}

int DebugVisitor::visit(AssignStm* stm) {
    long long v = eval(stm->e);
    trace.setVar(stm->id, v);
    step("Asignación a " + stm->id);
    return 0;
}

int DebugVisitor::visit(PrintStm* stm) {
    long long v = eval(stm->e);
    cout << "\n[DEBUG println!] valor = " << v << "\n";
    step("Después de println!");
    return 0;
}

int DebugVisitor::visit(IfStm* stm) {
    long long cond = eval(stm->cond);
    trace.regs.RCX = cond;
    step("Evaluado if (cond = " + to_string(cond) + ")");
    if (cond) {
        if (stm->then) stm->then->accept(this);
    } else {
        if (stm->els) stm->els->accept(this);
    }
    return 0;
}

int DebugVisitor::visit(WhileStm* stm) {
    while (true) {
        long long cond = eval(stm->cond);
        trace.regs.RCX = cond;

        if (!cond) break;

        step("Iteración while, cond = " + to_string(cond));
        if (stm->b) stm->b->accept(this);
    }
    trace.regs.RCX = 0;
    updateFlags(0);
    step("Salida del while");
    return 0;
}

int DebugVisitor::visit(ForStm* stm) {
    long long start = eval(stm->start);
    long long end   = eval(stm->end);

    trace.setVar(stm->id, start);

    while (true) {
        long long itVal = trace.getVar(stm->id);
        long long cond  = (itVal < end) ? 1 : 0;

        trace.regs.RCX = itVal;
        updateFlags(cond);

        if (!cond) {
            step("Salida del for");
            break;
        }

        step("Iteración for " + stm->id + " = " + to_string(itVal));
        if (stm->b) stm->b->accept(this);

        trace.setVar(stm->id, itVal + 1);
    }

    return 0;
}

int DebugVisitor::visit(ReturnStm* r) {
    lastValue = eval(r->e);
    step("Return con valor " + to_string(lastValue));
    return 0;
}

// ===== EXPRESIONES =====

int DebugVisitor::visit(NumberExp* exp) {
    lastValue = exp->val;
    return 0;
}

int DebugVisitor::visit(FloatExp* exp) {
    lastValue = static_cast<long long>(exp->val);
    return 0;
}

int DebugVisitor::visit(BoolExp* exp) {
    lastValue = exp->val ? 1 : 0;
    return 0;
}

int DebugVisitor::visit(IdExp* exp) {
    auto it = trace.vars.find(exp->val);
    if (it == trace.vars.end()) {
        cerr << "[DEBUG] Variable no inicializada: " << exp->val << "\n";
        lastValue = 0;
    } else {
        lastValue = it->second.second;
    }
    return 0;
}

int DebugVisitor::visit(BinaryExp* exp) {
    long long l = eval(exp->left);
    long long r = eval(exp->right);

    switch (exp->op) {
        case PLUS_OP:  lastValue = l + r; break;
        case MINUS_OP: lastValue = l - r; break;
        case MUL_OP:   lastValue = l * r; break;
        case DIV_OP:   lastValue = (r != 0 ? l / r : 0); break;
        case GT_OP:    lastValue = (l >  r); break;
        case GE_OP:    lastValue = (l >= r); break;
        case LT_OP:    lastValue = (l <  r); break;
        case LE_OP:    lastValue = (l <= r); break;
        case EQ_OP:    lastValue = (l == r); break;
    }
    return 0;
}

int DebugVisitor::visit(FCallExp* fcall) {
    cerr << "[DEBUG] Llamadas a funciones de usuario aún no soportadas en DebugVisitor.\n";
    lastValue = 0;
    return 0;
}
