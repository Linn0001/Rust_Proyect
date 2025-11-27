#ifndef RUST_PROJECT_DEBUGTRACE_H
#define RUST_PROJECT_DEBUGTRACE_H

#include <string>
#include <unordered_map>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>

struct DebugRegisterState {
    long long RAX = 0;
    long long RBX = 0;
    long long RCX = 0;
    long long RDX = 0;
    long long RSI = 0;
    long long RDI = 0;
    long long RBP = 0;
    long long RSP = 0;

    int ZF = 0;  // Zero flag
    int SF = 0;  // Sign flag
};

struct DebugStackCell {
    long long offset;      // ej: 0, -8, -16, ...
    long long value;       // valor numérico
    std::string label;     // nombre de la variable o puntero
};

struct DebugStep {
    std::string label;                 // descripción del paso
    int line = -1;                     // número de línea del .s (o -1 en modo AST)
    DebugRegisterState regs;           // valores de registros
    std::vector<DebugStackCell> stack; // snapshot de la pila en este paso
};

class DebugTrace {
public:
    // mapa nombre variable -> (offset en la pila, valor actual)
    std::unordered_map<std::string, std::pair<long long,long long>> vars;
    long long nextOffset = 0;          // empezamos en 0, luego -8, -16, ...
    DebugRegisterState regs;
    std::vector<DebugStep> steps;

    // Registrar variable (modo AST)
    void declareVar(const std::string& name, long long initialValue = 0) {
        nextOffset -= 8;
        vars[name] = { nextOffset, initialValue };
    }

    void setVar(const std::string& name, long long value) {
        auto it = vars.find(name);
        if (it != vars.end()) {
            it->second.second = value;
        }
    }

    long long getVar(const std::string& name) const {
        auto it = vars.find(name);
        if (it == vars.end()) return 0;
        return it->second.second;
    }

    // =========================
    // 1) Snapshot modo AST
    // =========================
    void snapshot(const std::string& label, int line = -1) {
        DebugStep s;
        s.label = label;
        s.line  = line;
        s.regs  = regs;

        // celda 0: RBP_OLD en offset 0
        s.stack.push_back(DebugStackCell{0, regs.RBP, "RBP_OLD"});

        for (auto& kv : vars) {
            const std::string& name = kv.first;
            long long offset = kv.second.first;
            long long value  = kv.second.second;
            s.stack.push_back(DebugStackCell{offset, value, name});
        }

        std::sort(s.stack.begin(), s.stack.end(),
                  [](const DebugStackCell& a, const DebugStackCell& b) {
                      return a.offset > b.offset;
                  });

        steps.push_back(std::move(s));
    }

    // =========================
    // 2) Snapshot modo ASM crudo
    //    (no la usamos ahora, pero queda disponible)
    // =========================
    void snapshotFromStack(const std::string& label,
                           int line,
                           const DebugRegisterState& regsState,
                           const std::unordered_map<long long,long long>& stackSlots)
    {
        DebugStep s;
        s.label = label;
        s.line  = line;
        s.regs  = regsState;

        for (auto& kv : stackSlots) {
            long long offset = kv.first;
            long long value  = kv.second;
            s.stack.push_back(DebugStackCell{offset, value, ""});
        }

        std::sort(s.stack.begin(), s.stack.end(),
                  [](const DebugStackCell& a, const DebugStackCell& b){
                      return a.offset > b.offset;
                  });

        steps.push_back(std::move(s));
    }

    // =========================-
    // Escribir trace en JSON
    // =========================
    void writeJson(const std::string& path) const {
        std::ofstream out(path);
        if (!out.is_open()) return;

        out << "{\n";
        out << "  \"steps\": [\n";

        for (size_t i = 0; i < steps.size(); ++i) {
            const DebugStep& s = steps[i];
            out << "    {\n";
            out << "      \"label\": \"" << escape(s.label) << "\",\n";
            out << "      \"line\": "  << s.line << ",\n";
            out << "      \"registers\": {\n";

            out << "        \"RAX\": " << s.regs.RAX << ",\n";
            out << "        \"RBX\": " << s.regs.RBX << ",\n";
            out << "        \"RCX\": " << s.regs.RCX << ",\n";
            out << "        \"RDX\": " << s.regs.RDX << ",\n";
            out << "        \"RSI\": " << s.regs.RSI << ",\n";
            out << "        \"RDI\": " << s.regs.RDI << ",\n";
            out << "        \"RBP\": " << s.regs.RBP << ",\n";
            out << "        \"RSP\": " << s.regs.RSP << ",\n";
            out << "        \"ZF\": "  << s.regs.ZF  << ",\n";
            out << "        \"SF\": "  << s.regs.SF  << "\n";
            out << "      },\n";
            out << "      \"stack\": [\n";

            for (size_t j = 0; j < s.stack.size(); ++j) {
                const DebugStackCell& c = s.stack[j];
                out << "        {\"offset\": " << c.offset
                    << ", \"value\": " << c.value
                    << ", \"label\": \"" << escape(c.label) << "\"}";
                if (j + 1 < s.stack.size()) out << ",";
                out << "\n";
            }

            out << "      ]\n";
            out << "    }";
            if (i + 1 < steps.size()) out << ",";
            out << "\n";
        }

        out << "  ]\n";
        out << "}\n";
    }

private:
    static std::string escape(const std::string& s) {
        std::ostringstream oss;
        for (char c : s) {
            if (c == '\\')      oss << "\\\\";
            else if (c == '\"') oss << "\\\"";
            else if (c == '\n') oss << "\\n";
            else                oss << c;
        }
        return oss.str();
    }
};

#endif // RUST_PROJECT_DEBUGTRACE_H
