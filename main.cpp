#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>

#include "src/lexic/scanner.h"
#include "src/lexic/token.h"
#include "src/syntactic/parser.h"
#include "src/syntactic/ast.h"
#include "src/semantic/TypeChecker.h"
#include "src/semantic/visitor.h"
#include "src/semantic/DebugVisitor.h"
#include "src/semantic/DebugTrace.h"
#include "src/semantic/AsmInterpreter.h"

using namespace std;
namespace fs = std::filesystem;

// Función del scanner definida en scanner.cpp
extern int ejecutar_scanner(Scanner* scanner, const string& InputFile);

// Leer archivo completo
string loadFile(const string& filename) {
    ifstream in(filename);
    if (!in.is_open()) {
        cerr << "Error: no se pudo abrir el archivo " << filename << endl;
        exit(1);
    }
    stringstream buffer;
    buffer << in.rdbuf();
    return buffer.str();
}

// Obtener nombre base del archivo sin extensión
string getBaseName(const string& filepath) {
    fs::path p(filepath);
    return p.stem().string(); // p.ej. "Test4.rs" -> "Test4"
}

int main(int argc, char* argv[]) {

    if (argc < 2) {
        cout << "Uso: ./Rust_Project <archivo.rs> [--debug]" << endl;
        return 1;
    }

    string inputFile = argv[1];
    bool debugMode   = (argc >= 3 && string(argv[2]) == "--debug");

    string code = loadFile(inputFile);

    cout << "=== Código fuente cargado ===\n\n";
    cout << code << "\n\n";

    // 1. SCANNER
    cout << "=== Ejecutando SCANNER ===\n";

    Scanner scanner(code.c_str());
    ejecutar_scanner(&scanner, inputFile);

    // 2. PARSER
    cout << "\n=== Ejecutando PARSER ===\n";

    try {
        Scanner* parserScanner = new Scanner(code.c_str());
        Parser*  parser        = new Parser(parserScanner);

        Program* prog = parser->parseProgram();

        cout << "\n>>> Parser exitoso\n";

        // 3. TYPECHECKER
        cout << "\n=== Ejecutando TYPECHECKER ===\n";

        TypeChecker tc;
        tc.typecheck(prog);

        cout << "\n>>> Typechecking exitoso\n";

        // Carpeta output + nombre base
        string outputDir = "output";
        if (!fs::exists(outputDir)) {
            fs::create_directory(outputDir);
        }
        string baseName  = getBaseName(inputFile);

        // 3.5 (opcional) DEBUGGER a nivel AST
        if (debugMode) {
            cout << "\n=== Ejecutando DEBUGGER AST (DebugVisitor) ===\n";

            DebugVisitor debugger;
            debugger.run(prog);

            string traceAstFile = outputDir + "/" + baseName + "_trace_ast.json";
            debugger.trace.writeJson(traceAstFile);
            cout << "\n>>> Trace AST generado en: " << traceAstFile << "\n";
        }

        // 4. CODEGEN -> genera el .s
        cout << "\n=== Ejecutando CODEGEN ===\n";

        string asmFile = outputDir + "/" + baseName + ".s";

        ofstream asmOut(asmFile);
        if (!asmOut.is_open()) {
            cerr << "Error: no se pudo crear el archivo " << asmFile << endl;
            delete prog;
            delete parser;
            delete parserScanner;
            return 1;
        }

        GenCodeVisitor codegen(asmOut);
        codegen.generar(prog);
        asmOut.close();

        cout << "\n>>> Generación de código exitosa\n";
        cout << "    Assembly generado en: " << asmFile << "\n";

        // 5. DEBUGGER opcional a nivel ASM (mini-emulador x86-64)
        if (debugMode) {
            cout << "\n=== Ejecutando INTÉRPRETE DE ASM (mini-emulador x86-64) ===\n";

            DebugTrace asmTrace;
            AsmInterpreter interp(asmTrace);

            if (!interp.load(asmFile)) {
                cerr << "[AsmDebug] No se pudo cargar el archivo ASM: " << asmFile << "\n";
            } else {
                // ejecuta instrucción por instrucción hasta ret/fin
                interp.run();

                string traceAsmFile = outputDir + "/" + baseName + "_trace_asm.json";
                asmTrace.writeJson(traceAsmFile);
                cout << "\n>>> Trace ASM generado en: " << traceAsmFile << "\n";
                cout << "    (Cada step corresponde a una instrucción y tiene 'line' del .s)\n";
            }
        }

        // Liberar AST y parser-
        delete prog;
        delete parser;
        delete parserScanner;
    }
    catch (exception& ex) {
        cout << "\n*** Error en parser: " << ex.what() << " ***\n";
    }

    return 0;
}
