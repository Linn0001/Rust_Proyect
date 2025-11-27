#ifndef RUST_PROJECT_ASMINTERPRETER_H
#define RUST_PROJECT_ASMINTERPRETER_H

#include <string>
#include <vector>
#include <unordered_map>
#include "DebugTrace.h"

class AsmInterpreter {
public:
    explicit AsmInterpreter(DebugTrace& t) : trace(t) {}

    bool load(const std::string& path);
    void run();

private:
    struct Instr {
        std::string text;              // línea completa del .s
        std::string op;                // movq, addq, ...
        std::vector<std::string> args; // operandos
        int lineNumber = 0;            // línea 1-based en el .s
    };

    DebugTrace& trace;

    struct Cpu {
        long long RAX = 0;
        long long RBX = 0;
        long long RCX = 0;
        long long RDX = 0;
        long long RSI = 0;
        long long RDI = 0;
        long long RBP = 0;
        long long RSP = 0;
        int ZF = 0;
        int SF = 0;
    } cpu;

    std::vector<Instr> prog;
    std::unordered_map<std::string,int> labelToIndex;

    // memoria virtual: offset respecto a RBP -> valor (ej: -16(%rbp))
    std::unordered_map<long long,long long> memByOffset;
    // pila lógica para pushq/popq
    std::vector<long long> stack;
    // pila de llamadas (return addresses)
    std::vector<int> callStack;

    long long lastCmp    = 0;  // resultado dst - src
    long long lastCmpLhs = 0;  // dst (para signed/unsigned)
    long long lastCmpRhs = 0;  // src

    static std::string trim(const std::string& s);
    static void splitOp(const std::string& line,
                        std::string& op,
                        std::vector<std::string>& args);

    long long readOperand(const std::string& operand);
    void writeOperand(const std::string& operand, long long value);

    void execute(const Instr& ins, int& ip);
    void snapshot(const Instr& ins);
};

#endif // RUST_PROJECT_ASMINTERPRETER_H
