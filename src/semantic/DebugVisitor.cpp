#include "DebugVisitor.h"
#include <iostream>
#include <sstream>
#include <iomanip>

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

    lastValue       = 0;
    lastFloatValue  = 0.0;
    lastIsFloat     = false;
    stopExecution   = false;
    functions.clear();
    isFloatVar.clear();
    varFloatVal.clear();

    // Registrar todas las funciones
    for (auto fd : p->fdlist) {
        functions[fd->name] = fd;
    }

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
    if (!e) {
        lastValue      = 0;
        lastFloatValue = 0.0;
        lastIsFloat    = false;
        return 0;
    }

    e->accept(this);

    // RAX siempre se queda con la versión entera
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
        const string& name     = kv.first;
        const DebugVarInfo& vi = kv.second;
        cout << "    " << name << " = " << vi.value_str << "\n";
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

// ===== Helpers de formato =====

std::string DebugVisitor::formatNumber(double v) const {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(6) << v;
    std::string s = oss.str();

    // Quitar ceros finales y punto si sobra
    while (!s.empty() && s.back() == '0') s.pop_back();
    if (!s.empty() && s.back() == '.') s.pop_back();
    if (s.empty()) s = "0";
    return s;
}

std::string DebugVisitor::formatValue(long long ival, double fval, bool isFloat) const {
    if (isFloat) {
        return formatNumber(fval);
    }
    return std::to_string(ival);
}

// ================== Implementaciones Visitor ==================

int DebugVisitor::visit(Program* p) {
    run(p);
    return 0;
}

int DebugVisitor::visit(FunDec* fd) {
    // No ejecutamos funciones aquí; las llamamos desde FCallExp
    return 0;
}

int DebugVisitor::visit(Body* body) {
    for (auto vd : body->decs) {
        vd->accept(this);
        if (stopExecution) return 0;
    }
    for (auto s : body->stmlist) {
        s->accept(this);
        if (stopExecution) return 0;
    }
    return 0;
}

int DebugVisitor::visit(VarDec* vd) {
    long long initInt     = 0;
    double    initFloat   = 0.0;
    bool      isFloatInit = false;

    if (vd->e) {
        eval(vd->e);
        initInt     = lastValue;
        initFloat   = lastFloatValue;
        isFloatInit = lastIsFloat;
    }

    trace.declareVar(vd->name, initInt);

    // Marcar tipo y valor real si es float
    isFloatVar[vd->name] = isFloatInit;
    if (isFloatInit) {
        varFloatVal[vd->name] = initFloat;
        auto it = trace.vars.find(vd->name);
        if (it != trace.vars.end()) {
            it->second.value_str = formatValue(initInt, initFloat, true);
        }
    }

    step("Declaración de variable " + vd->name);
    return 0;
}

int DebugVisitor::visit(AssignStm* stm) {
    eval(stm->e);
    trace.setVar(stm->id, lastValue);

    bool isFloatAssign = lastIsFloat;
    isFloatVar[stm->id] = isFloatAssign;
    if (isFloatAssign) {
        varFloatVal[stm->id] = lastFloatValue;
    } else {
        varFloatVal.erase(stm->id);
    }

    auto it = trace.vars.find(stm->id);
    if (it != trace.vars.end()) {
        double usedFloat = isFloatAssign ? lastFloatValue : static_cast<double>(lastValue);
        it->second.value_str = formatValue(lastValue, usedFloat, isFloatAssign);
    }

    step("Asignación a " + stm->id);
    return 0;
}

int DebugVisitor::visit(PrintStm* stm) {
    eval(stm->e);

    std::string repr = formatValue(
            lastValue,
            lastIsFloat ? lastFloatValue : static_cast<double>(lastValue),
            lastIsFloat
    );

    cout << "\n[DEBUG println!] valor = " << repr << "\n";
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
        if (stopExecution) break;
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
    isFloatVar[stm->id] = false;
    varFloatVal.erase(stm->id);

    while (true) {
        long long itVal = trace.getVar(stm->id);
        long long cond  = (itVal < end) ? 1 : 0;

        // RCX: iterador, RAX: condición (bool) → coherente con otros casos
        trace.regs.RCX = itVal;
        lastValue      = cond;
        lastFloatValue = static_cast<double>(cond);
        lastIsFloat    = false;
        trace.regs.RAX = cond;
        updateFlags(cond);

        if (!cond) {
            step("Salida del for");
            break;
        }

        step("Iteración for " + stm->id + " = " + to_string(itVal));
        if (stm->b) stm->b->accept(this);
        if (stopExecution) break;

        trace.setVar(stm->id, itVal + 1);
    }

    return 0;
}

int DebugVisitor::visit(ReturnStm* r) {
    eval(r->e);

    std::string repr = formatValue(
            lastValue,
            lastIsFloat ? lastFloatValue : static_cast<double>(lastValue),
            lastIsFloat
    );

    step("Return con valor " + repr);

    // marcamos que ya no se ejecuten más statements en este cuerpo
    stopExecution = true;
    return 0;
}

// ===== EXPRESIONES =====

int DebugVisitor::visit(NumberExp* exp) {
    lastValue      = exp->val;
    lastFloatValue = static_cast<double>(exp->val);
    lastIsFloat    = false;
    return 0;
}

int DebugVisitor::visit(FloatExp* exp) {
    // Guardamos el valor real en double y una versión entera para flags
    lastFloatValue = exp->val;
    lastValue      = static_cast<long long>(exp->val);
    lastIsFloat    = true;
    return 0;
}

int DebugVisitor::visit(BoolExp* exp) {
    lastValue      = exp->val ? 1 : 0;
    lastFloatValue = static_cast<double>(lastValue);
    lastIsFloat    = false;
    return 0;
}

int DebugVisitor::visit(IdExp* exp) {
    auto it = trace.vars.find(exp->val);
    if (it == trace.vars.end()) {
        cerr << "[DEBUG] Variable no inicializada: " << exp->val << "\n";
        lastValue      = 0;
        lastFloatValue = 0.0;
        lastIsFloat    = false;
        return 0;
    }

    lastValue = it->second.value;

    auto itType = isFloatVar.find(exp->val);
    bool isF = (itType != isFloatVar.end() && itType->second);

    if (isF) {
        auto itF = varFloatVal.find(exp->val);
        if (itF != varFloatVal.end()) {
            lastFloatValue = itF->second;
        } else {
            lastFloatValue = static_cast<double>(lastValue);
        }
    } else {
        lastFloatValue = static_cast<double>(lastValue);
    }

    lastIsFloat = isF;
    return 0;
}

int DebugVisitor::visit(BinaryExp* exp) {
    // Evaluar lado izquierdo
    exp->left->accept(this);
    long long lInt    = lastValue;
    double    lFloat  = lastFloatValue;
    bool      lIsF    = lastIsFloat;

    // Evaluar lado derecho
    exp->right->accept(this);
    long long rInt    = lastValue;
    double    rFloat  = lastFloatValue;
    bool      rIsF    = lastIsFloat;

    bool anyFloat = lIsF || rIsF;

    auto asDoubleL = [&](void) {
        return lIsF ? lFloat : static_cast<double>(lInt);
    };
    auto asDoubleR = [&](void) {
        return rIsF ? rFloat : static_cast<double>(rInt);
    };

    switch (exp->op) {
        case PLUS_OP:
        case MINUS_OP:
        case MUL_OP:
        case DIV_OP: {
            if (anyFloat) {
                double lv = asDoubleL();
                double rv = asDoubleR();
                double res = 0.0;
                switch (exp->op) {
                    case PLUS_OP:  res = lv + rv; break;
                    case MINUS_OP: res = lv - rv; break;
                    case MUL_OP:   res = lv * rv; break;
                    case DIV_OP:   res = (rv != 0.0 ? lv / rv : 0.0); break;
                    default: break;
                }
                lastFloatValue = res;
                lastValue      = static_cast<long long>(res);
                lastIsFloat    = true;
            } else {
                switch (exp->op) {
                    case PLUS_OP:  lastValue = lInt + rInt; break;
                    case MINUS_OP: lastValue = lInt - rInt; break;
                    case MUL_OP:   lastValue = lInt * rInt; break;
                    case DIV_OP:   lastValue = (rInt != 0 ? lInt / rInt : 0); break;
                    default: break;
                }
                lastFloatValue = static_cast<double>(lastValue);
                lastIsFloat    = false;
            }
            break;
        }

        case GT_OP:
        case GE_OP:
        case LT_OP:
        case LE_OP:
        case EQ_OP: {
            bool res = false;
            if (anyFloat) {
                double lv = asDoubleL();
                double rv = asDoubleR();
                switch (exp->op) {
                    case GT_OP: res = (lv >  rv); break;
                    case GE_OP: res = (lv >= rv); break;
                    case LT_OP: res = (lv <  rv); break;
                    case LE_OP: res = (lv <= rv); break;
                    case EQ_OP: res = (lv == rv); break;
                    default: break;
                }
            } else {
                switch (exp->op) {
                    case GT_OP: res = (lInt >  rInt); break;
                    case GE_OP: res = (lInt >= rInt); break;
                    case LT_OP: res = (lInt <  rInt); break;
                    case LE_OP: res = (lInt <= rInt); break;
                    case EQ_OP: res = (lInt == rInt); break;
                    default: break;
                }
            }
            lastValue      = res ? 1 : 0;
            lastFloatValue = static_cast<double>(lastValue);
            lastIsFloat    = false;
            break;
        }
    }

    return 0;
}

// ===== LLAMADAS A FUNCIÓN (MODO AST) =====

int DebugVisitor::visit(FCallExp* fcall) {
    auto it = functions.find(fcall->name);
    if (it == functions.end()) {
        cerr << "[DEBUG] Llamada a función no declarada: " << fcall->name << "\n";
        lastValue      = 0;
        lastFloatValue = 0.0;
        lastIsFloat    = false;
        return 0;
    }

    FunDec* fd = it->second;

    // Guardar entorno del "caller"
    auto savedVars        = trace.vars;
    long long savedNext   = trace.nextOffset;
    auto savedRegs        = trace.regs;
    bool savedStop        = stopExecution;
    auto savedIsFloatVar  = isFloatVar;
    auto savedVarFloatVal = varFloatVal;

    // Reset de flag para ejecutar la función completa hasta su return
    stopExecution = false;

    // Pasar argumentos como nuevas variables (parámetros)
    for (size_t i = 0; i < fd->pnames.size(); ++i) {
        long long argInt = 0;
        double    argF   = 0.0;
        bool      argIsF = false;

        if (i < fcall->args.size()) {
            eval(fcall->args[i]);
            argInt = lastValue;
            argF   = lastFloatValue;
            argIsF = lastIsFloat;
        }

        trace.declareVar(fd->pnames[i], argInt);

        isFloatVar[fd->pnames[i]] = argIsF;
        if (argIsF) {
            varFloatVal[fd->pnames[i]] = argF;
            auto itVar = trace.vars.find(fd->pnames[i]);
            if (itVar != trace.vars.end()) {
                itVar->second.value_str = formatValue(argInt, argF, true);
            }
        }
    }

    step("Entrada a función " + fd->name);

    // Ejecutar cuerpo de la función
    if (fd->b)
        fd->b->accept(this);

    long long resultInt      = lastValue;
    double    resultFloat    = lastFloatValue;
    bool      resultIsFloat  = lastIsFloat;

    std::string repr = formatValue(
            resultInt,
            resultIsFloat ? resultFloat : static_cast<double>(resultInt),
            resultIsFloat
    );

    step("Salida de función " + fd->name + " con retorno " + repr);

    // Restaurar entorno del caller
    trace.vars       = savedVars;
    trace.nextOffset = savedNext;
    trace.regs       = savedRegs;
    stopExecution    = savedStop;
    isFloatVar       = savedIsFloatVar;
    varFloatVal      = savedVarFloatVal;

    // Dejar el resultado como último valor
    lastValue      = resultInt;
    lastFloatValue = resultFloat;
    lastIsFloat    = resultIsFloat;

    // Dejar el resultado en "RAX" lógico
    trace.regs.RAX = lastValue;
    updateFlags(lastValue);

    return 0;
}

// ===== TERNARY EXP =====

int DebugVisitor::visit(TernaryExp* e) {
    long long condVal = eval(e->cond);
    step("Evaluando condición ternaria (cond = " + to_string(condVal) + ")");

    if (condVal) {
        eval(e->thenExp);
        std::string repr = formatValue(
                lastValue,
                lastIsFloat ? lastFloatValue : static_cast<double>(lastValue),
                lastIsFloat
        );
        step("Resultado rama then de ternario = " + repr);
    } else {
        eval(e->elseExp);
        std::string repr = formatValue(
                lastValue,
                lastIsFloat ? lastFloatValue : static_cast<double>(lastValue),
                lastIsFloat
        );
        step("Resultado rama else de ternario = " + repr);
    }

    return 0;
}