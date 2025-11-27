#include "AsmInterpreter.h"
#include <fstream>
#include <sstream>
#include <cctype>
#include <iostream>
#include <cstdint>
#include <algorithm>
#include <cstring>   // memcpy
#include <iomanip>   // setprecision

using namespace std;

string AsmInterpreter::trim(const std::string& s) {
    size_t i = 0, j = s.size();
    while (i < j && isspace((unsigned char)s[i])) ++i;
    while (j > i && isspace((unsigned char)s[j-1])) --j;
    return s.substr(i, j - i);
}

void AsmInterpreter::splitOp(const std::string& line,
                             std::string& op,
                             std::vector<std::string>& args)
{
    string t = trim(line);
    if (t.empty()) return;

    stringstream ss(t);
    ss >> op;
    string rest;
    getline(ss, rest);
    rest = trim(rest);
    if (rest.empty()) return;

    string cur;
    stringstream ss2(rest);
    while (getline(ss2, cur, ',')) {
        string a = trim(cur);
        if (!a.empty())
            args.push_back(a);
    }
}

bool AsmInterpreter::getMemOffset(const std::string& operand, long long& offset) {
    string op = trim(operand);
    size_t parenPos = op.find('(');
    if (parenPos == string::npos) return false;
    offset = stoll(op.substr(0, parenPos));
    return true;
}

bool AsmInterpreter::load(const std::string& path) {
    ifstream in(path);
    if (!in.is_open()) return false;

    prog.clear();
    labelToIndex.clear();
    memByOffset.clear();
    offsetToName.clear();

    string line;
    int lineNo = 0;

    while (getline(in, line)) {
        ++lineNo;
        string t = trim(line);
        if (t.empty()) continue;

        // Comentarios / DEBUG_VAR generados por GenCodeVisitor
        if (!t.empty() && t[0] == '#') {
            // Formato: "# DEBUG_VAR <nombre> <offset>"
            if (t.rfind("# DEBUG_VAR", 0) == 0) {
                string sharp, tag, name;
                long long offset;
                stringstream ss(t);
                ss >> sharp >> tag >> name >> offset;
                if (!name.empty()) {
                    offsetToName[offset] = name;
                }
            }
            // Ignoramos cualquier comentario a nivel de instrucciones
            continue;
        }

        // Directivas .data, .text, .section, etc.
        if (t[0] == '.') continue;

        // Etiquetas: main:, while_0:, suma_rango:, etc.
        if (t.back() == ':') {
            string label = t.substr(0, t.size() - 1);
            labelToIndex[label] = (int)prog.size();
            continue;
        }

        Instr ins;
        ins.text       = line;
        ins.lineNumber = lineNo;
        splitOp(line, ins.op, ins.args);

        if (ins.op.empty())
            continue;

        prog.push_back(ins);
    }

    return true;
}

long long AsmInterpreter::readOperand(const std::string& operand) {
    string op = trim(operand);

    // Inmediato: $11
    if (!op.empty() && op[0] == '$') {
        return stoll(op.substr(1));
    }

    // Registro (los tratamos lógicamente como 64 bits)
    if (!op.empty() && op[0] == '%') {
        if (op == "%rax" || op == "%eax" || op == "%ax" || op == "%al") return cpu.RAX;
        if (op == "%rbx" || op == "%ebx" || op == "%bx" || op == "%bl") return cpu.RBX;
        if (op == "%rcx" || op == "%ecx" || op == "%cx" || op == "%cl") return cpu.RCX;
        if (op == "%rdx" || op == "%edx" || op == "%dx" || op == "%dl") return cpu.RDX;
        if (op == "%rsi" || op == "%esi" || op == "%si" || op == "%sil") return cpu.RSI;
        if (op == "%rdi" || op == "%edi" || op == "%di" || op == "%dil") return cpu.RDI;
        if (op == "%rbp" || op == "%ebp" || op == "%bp" || op == "%bpl") return cpu.RBP;
        if (op == "%rsp" || op == "%esp" || op == "%sp" || op == "%spl") return cpu.RSP;
        return 0;
    }

    // Memoria tipo offset(%rbp) u offset(%rsp) – usamos solo el offset lógico.
    size_t parenPos = op.find('(');
    if (parenPos != string::npos) {
        long long offset = stoll(op.substr(0, parenPos));
        auto it = memByOffset.find(offset);
        if (it != memByOffset.end()) return it->second;
        return 0;
    }

    return 0;
}

void AsmInterpreter::writeOperand(const std::string& operand, long long value) {
    string op = trim(operand);

    // Registro
    if (!op.empty() && op[0] == '%') {
        if (op == "%rax" || op == "%eax" || op == "%ax" || op == "%al") { cpu.RAX = value; return; }
        if (op == "%rbx" || op == "%ebx" || op == "%bx" || op == "%bl") { cpu.RBX = value; return; }
        if (op == "%rcx" || op == "%ecx" || op == "%cx" || op == "%cl") { cpu.RCX = value; return; }
        if (op == "%rdx" || op == "%edx" || op == "%dx" || op == "%dl") { cpu.RDX = value; return; }
        if (op == "%rsi" || op == "%esi" || op == "%si" || op == "%sil") { cpu.RSI = value; return; }
        if (op == "%rdi" || op == "%edi" || op == "%di" || op == "%dil") { cpu.RDI = value; return; }
        if (op == "%rbp" || op == "%ebp" || op == "%bp" || op == "%bpl") { cpu.RBP = value; return; }
        if (op == "%rsp" || op == "%esp" || op == "%sp" || op == "%spl") { cpu.RSP = value; return; }
        return;
    }

    // Memoria: offset(%rbp) / offset(%rsp)
    size_t parenPos = op.find('(');
    if (parenPos != string::npos) {
        long long offset = stoll(op.substr(0, parenPos));
        memByOffset[offset] = value;
        return;
    }
}

void AsmInterpreter::snapshot(const Instr& ins) {
    // Copiar CPU a trace.regs
    trace.regs.RAX = cpu.RAX;
    trace.regs.RBX = cpu.RBX;
    trace.regs.RCX = cpu.RCX;
    trace.regs.RDX = cpu.RDX;
    trace.regs.RSI = cpu.RSI;
    trace.regs.RDI = cpu.RDI;
    trace.regs.RBP = cpu.RBP;
    trace.regs.RSP = cpu.RSP;
    trace.regs.ZF  = cpu.ZF;
    trace.regs.SF  = cpu.SF;

    // Reconstruir pila lógica desde memByOffset como variables
    trace.vars.clear();
    trace.nextOffset = 0;

    for (auto& kv : memByOffset) {
        long long offset = kv.first;
        long long val    = kv.second;

        if (offset == 0) {
            continue;
        }

        DebugVarInfo info;
        info.offset     = offset;
        info.value      = val;
        info.bits       = 64;
        info.isUnsigned = false;

        auto itf = floatSlots.find(offset);

        if (itf != floatSlots.end()) {
            double d = itf->second;
            std::ostringstream oss;
            oss.setf(std::ios::fixed);
            oss << std::setprecision(6) << d;
            info.value_str = oss.str();
            info.type      = "f64";
        } else {
            info.value_str = std::to_string(val);
            info.type      = "i64";
        }

        std::string name;
        auto itName = offsetToName.find(offset);
        if (itName != offsetToName.end()) {
            name = itName->second;
        } else {
            name = "off" + std::to_string(offset);
        }

        trace.vars[name] = info;
        trace.nextOffset = std::min(trace.nextOffset, offset);
    }

    std::string label =
            "Línea " + std::to_string(ins.lineNumber) + ": " + trim(ins.text);
    trace.snapshot(label, ins.lineNumber);
}

void AsmInterpreter::execute(const Instr& ins, int& ip) {
    const string& op = ins.op;
    const auto&  a   = ins.args;

    // ================= MOV =================
    if (op == "movq" || op == "movl" || op == "movw" || op == "movb") {
        long long v = readOperand(a[0]);
        writeOperand(a[1], v);
        ++ip;
    }
    else if (op == "movabsq") {
        long long v = readOperand(a[0]); // inmediato $imm64
        writeOperand(a[1], v);
        ++ip;
    }
    else if (op == "movzbl" || op == "movzbq") {
        long long v = readOperand(a[0]) & 0xFF;
        writeOperand(a[1], v);
        ++ip;
    }
    else if (op == "movzwl" || op == "movzwq") {
        long long v = readOperand(a[0]) & 0xFFFF;
        writeOperand(a[1], v);
        ++ip;
    }
    else if (op == "movsbl" || op == "movsbq" || op == "movswl" || op == "movswq") {
        long long v = readOperand(a[0]);
        writeOperand(a[1], v);
        ++ip;
    }
    else if (op == "movslq") {
        long long v = readOperand(a[0]);
        int32_t v32 = (int32_t)v;
        long long res = (long long)v32;
        writeOperand(a[1], res);
        ++ip;
    }

        // ================= PUSH / POP =================
    else if (op == "pushq") {
        long long v = readOperand(a[0]);
        stack.push_back(v);
        cpu.RSP -= 8;
        ++ip;
    }
    else if (op == "popq") {
        long long v = 0;
        if (!stack.empty()) {
            v = stack.back();
            stack.pop_back();
        }
        cpu.RSP += 8;
        writeOperand(a[0], v);
        ++ip;
    }

        // ================= ARITMÉTICA =================
    else if (op == "addq" || op == "addl") {
        long long src = readOperand(a[0]);
        long long dst = readOperand(a[1]);

        if (op == "addl") {
            int32_t res32 = (int32_t)dst + (int32_t)src;
            long long res = (int64_t)res32;
            writeOperand(a[1], res);
            cpu.ZF = (res32 == 0);
            cpu.SF = (res32 < 0);
        } else {
            long long res = dst + src;
            writeOperand(a[1], res);
            cpu.ZF = (res == 0);
            cpu.SF = (res < 0);
        }
        ++ip;
    }
    else if (op == "subq" || op == "subl") {
        long long src = readOperand(a[0]);
        long long dst = readOperand(a[1]);

        if (op == "subl") {
            int32_t res32 = (int32_t)dst - (int32_t)src;
            long long res = (int64_t)res32;
            writeOperand(a[1], res);
            cpu.ZF = (res32 == 0);
            cpu.SF = (res32 < 0);
        } else {
            long long res = dst - src;
            writeOperand(a[1], res);
            cpu.ZF = (res == 0);
            cpu.SF = (res < 0);
        }
        ++ip;
    }
    else if (op == "imulq" || op == "imull") {
        long long src = readOperand(a[0]);
        long long dst = readOperand(a[1]);

        if (op == "imull") {
            int32_t res32 = (int32_t)dst * (int32_t)src;
            long long res = (int64_t)res32;
            writeOperand(a[1], res);
            cpu.ZF = (res32 == 0);
            cpu.SF = (res32 < 0);
        } else {
            long long res = dst * src;
            writeOperand(a[1], res);
            cpu.ZF = (res == 0);
            cpu.SF = (res < 0);
        }
        ++ip;
    }
    else if (op == "incq" || op == "incl") {
        long long dst = readOperand(a[0]);
        if (op == "incl") {
            int32_t res32 = (int32_t)dst + 1;
            long long res = (int64_t)res32;
            writeOperand(a[0], res);
            cpu.ZF = (res32 == 0);
            cpu.SF = (res32 < 0);
        } else {
            long long res = dst + 1;
            writeOperand(a[0], res);
            cpu.ZF = (res == 0);
            cpu.SF = (res < 0);
        }
        ++ip;
    }

        // ================= COMPARACIÓN =================
    else if (op == "cmpq" || op == "cmpl") {
        long long src = readOperand(a[0]);
        long long dst = readOperand(a[1]);

        lastCmpLhs = dst;
        lastCmpRhs = src;

        long long res;
        if (op == "cmpl") {
            int32_t d32 = (int32_t)dst;
            int32_t s32 = (int32_t)src;
            res = (long long)(d32 - s32);
        } else {
            res = dst - src;
        }
        lastCmp = res;
        cpu.ZF   = (res == 0);
        cpu.SF   = (res < 0);
        ++ip;
    }

        // ================= SALTOS (SIGNED) =================
    else if (op == "jge") { // signed >=
        long long L = lastCmpLhs;
        long long R = lastCmpRhs;
        std::string label = a[0];
        if (labelToIndex.count(label) && L >= R)
            ip = labelToIndex[label];
        else
            ++ip;
    }
    else if (op == "jg") { // signed >
        long long L = lastCmpLhs;
        long long R = lastCmpRhs;
        std::string label = a[0];
        if (labelToIndex.count(label) && L > R)
            ip = labelToIndex[label];
        else
            ++ip;
    }
    else if (op == "jle") { // signed <=
        long long L = lastCmpLhs;
        long long R = lastCmpRhs;
        std::string label = a[0];
        if (labelToIndex.count(label) && L <= R)
            ip = labelToIndex[label];
        else
            ++ip;
    }
    else if (op == "jl") { // signed <
        long long L = lastCmpLhs;
        long long R = lastCmpRhs;
        std::string label = a[0];
        if (labelToIndex.count(label) && L < R)
            ip = labelToIndex[label];
        else
            ++ip;
    }
    else if (op == "je" || op == "jz") { // ==
        long long L = lastCmpLhs;
        long long R = lastCmpRhs;
        std::string label = a[0];
        if (labelToIndex.count(label) && L == R)
            ip = labelToIndex[label];
        else
            ++ip;
    }
    else if (op == "jne" || op == "jnz") { // !=
        long long L = lastCmpLhs;
        long long R = lastCmpRhs;
        std::string label = a[0];
        if (labelToIndex.count(label) && L != R)
            ip = labelToIndex[label];
        else
            ++ip;
    }

        // ================= SALTOS (UNSIGNED) =================
    else if (op == "ja" || op == "jnbe") { // unsigned >
        std::string label = a[0];
        unsigned long long L = (unsigned long long)lastCmpLhs;
        unsigned long long R = (unsigned long long)lastCmpRhs;
        if (labelToIndex.count(label) && L > R)
            ip = labelToIndex[label];
        else
            ++ip;
    }
    else if (op == "jae" || op == "jnb") { // unsigned >=
        std::string label = a[0];
        unsigned long long L = (unsigned long long)lastCmpLhs;
        unsigned long long R = (unsigned long long)lastCmpRhs;
        if (labelToIndex.count(label) && L >= R)
            ip = labelToIndex[label];
        else
            ++ip;
    }
    else if (op == "jb" || op == "jnae") { // unsigned <
        std::string label = a[0];
        unsigned long long L = (unsigned long long)lastCmpLhs;
        unsigned long long R = (unsigned long long)lastCmpRhs;
        if (labelToIndex.count(label) && L < R)
            ip = labelToIndex[label];
        else
            ++ip;
    }
    else if (op == "jbe" || op == "jna") { // unsigned <=
        std::string label = a[0];
        unsigned long long L = (unsigned long long)lastCmpLhs;
        unsigned long long R = (unsigned long long)lastCmpRhs;
        if (labelToIndex.count(label) && L <= R)
            ip = labelToIndex[label];
        else
            ++ip;
    }

        // ================= SETcc =================
    else if (op == "setg") {
        long long L = lastCmpLhs, R = lastCmpRhs;
        writeOperand(a[0], (L > R) ? 1 : 0);
        ++ip;
    }
    else if (op == "setge") {
        long long L = lastCmpLhs, R = lastCmpRhs;
        writeOperand(a[0], (L >= R) ? 1 : 0);
        ++ip;
    }
    else if (op == "setl") {
        long long L = lastCmpLhs, R = lastCmpRhs;
        writeOperand(a[0], (L < R) ? 1 : 0);
        ++ip;
    }
    else if (op == "setle") {
        long long L = lastCmpLhs, R = lastCmpRhs;
        writeOperand(a[0], (L <= R) ? 1 : 0);
        ++ip;
    }
    else if (op == "sete" || op == "setz") {
        long long L = lastCmpLhs, R = lastCmpRhs;
        writeOperand(a[0], (L == R) ? 1 : 0);
        ++ip;
    }
    else if (op == "setne" || op == "setnz") {
        long long L = lastCmpLhs, R = lastCmpRhs;
        writeOperand(a[0], (L != R) ? 1 : 0);
        ++ip;
    }

        // ================= JMP =================
    else if (op == "jmp") {
        std::string label = a[0];
        if (labelToIndex.count(label)) {
            ip = labelToIndex[label];
        } else {
            ++ip;
        }
    }

        // ================= LEAQ =================
    else if (op == "leaq") {
        writeOperand(a[1], 0);
        ++ip;
    }

        // ================= LLAMADAS =================
    else if (op == "call") {
        std::string label = a[0];
        auto it = labelToIndex.find(label);
        if (it != labelToIndex.end()) {
            // función de usuario
            callStack.push_back(ip + 1);
            ip = it->second;
        } else {
            // printf u otra externa → ignoramos efectos, solo avanzamos
            ++ip;
        }
    }

        // ================= FLOAT x87 =================
    else if (op == "fldl") {
        long long bits = readOperand(a[0]);
        double d;
        std::memcpy(&d, &bits, sizeof(double));
        fstack.push_back(d);

        long long off;
        if (getMemOffset(a[0], off)) {
            floatSlots[off] = d;
        }

        ++ip;
    }
    else if (op == "fstpl") {
        if (!fstack.empty()) {
            double d = fstack.back();
            fstack.pop_back();
            long long bits;
            std::memcpy(&bits, &d, sizeof(double));
            writeOperand(a[0], bits);

            long long off;
            if (getMemOffset(a[0], off)) {
                floatSlots[off] = d;
            }
        }
        ++ip;
    }
    else if (op == "faddp" || op == "fsubp" || op == "fmulp" || op == "fdivp") {
        if (fstack.size() >= 2) {
            double a0 = fstack[fstack.size() - 2];
            double a1 = fstack[fstack.size() - 1];
            double res = 0.0;

            if (op == "faddp") res = a0 + a1;
            else if (op == "fsubp") res = a0 - a1;
            else if (op == "fmulp") res = a0 * a1;
            else if (op == "fdivp") res = a0 / a1;

            fstack[fstack.size() - 2] = res;
            fstack.pop_back();
        }
        ++ip;
    }

        // ================= FLOAT SSE (stub seguro) =================
    else if (op == "movsd" || op == "addsd" || op == "subsd" ||
             op == "mulsd" || op == "divsd" || op == "ucomisd" ||
             op == "comisd") {
        // Por ahora no simulamos XMM, solo avanzamos
        ++ip;
    }

        // ================= LEAVE / RET =================
    else if (op == "leave") {
        cpu.RSP = cpu.RBP;
        long long v = 0;
        if (!stack.empty()) {
            v = stack.back();
            stack.pop_back();
        }
        cpu.RBP = v;
        ++ip;
    }
    else if (op == "ret") {
        if (!callStack.empty()) {
            ip = callStack.back();
            callStack.pop_back();
        } else {
            ip = (int)prog.size(); // fin
        }
    }

        // ================= DEFAULT =================
    else {
        ++ip;
    }

    // Snapshot después de ejecutar la instrucción
    snapshot(ins);
}

void AsmInterpreter::run() {

    trace.vars.clear();
    trace.steps.clear();
    trace.nextOffset = 0;
    trace.regs = DebugRegisterState{};

    // Resetear CPU y estructuras internas
    cpu = Cpu{};
    memByOffset.clear();
    // offsetToName NO se limpia aquí, se llena en load() y se mantiene
    stack.clear();
    callStack.clear();
    fstack.clear();
    floatSlots.clear();
    lastCmp    = 0;
    lastCmpLhs = 0;
    lastCmpRhs = 0;

    int ip = 0;
    auto it = labelToIndex.find("main");
    if (it != labelToIndex.end()) {
        ip = it->second;   // primera instrucción después de la etiqueta main:
    }

    // Ejecutar instrucción por instrucción
    while (ip >= 0 && ip < (int)prog.size()) {
        execute(prog[ip], ip);
    }
}