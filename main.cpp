// #include <iostream>
// #include "src/lexic/scanner.h"
// #include "src/syntactic/Parser.h"
// #include "src/semantic/TypeCheck.h"
// #include "src/semantic/CodeGen.h"
//
// int main(int argc, char** argv) {
//     if (argc < 2) {
//         std::cerr << "Uso: " << argv[0] << " archivo.rs\n";
//         return 1;
//     }
//
//     try {
//         scanner scanner(argv[1]);
//         Parser parser(&scanner);
//         Program* ast = parser.parse();     // AST raíz
//
//         SymbolTable table;
//         TypeCheck tc(&table);
//         ast->accept(&tc);                  // chequeo de tipos
//
//         CodeGen cg("out.s");               // genera ensamblador
//         ast->accept(&cg);
//
//         delete ast;
//     } catch (const std::exception& ex) {
//         std::cerr << "Error: " << ex.what() << '\n';
//         return 1;
//     }
//
//     return 0;
// }

#include <iostream>
#include <fstream>
#include <sstream>

#include "src/lexic/scanner.h"
#include "src/lexic/token.h"
#include "src/syntactic/parser.h"
#include "src/syntactic/ast.h"
#include "src/semantic/TypeChecker.h"

using namespace std;

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

int main(int argc, char* argv[]) {

    if (argc < 2) {
        cout << "Uso: ./Rust_Project <archivo.rs>" << endl;
        return 1;
    }

    string inputFile = argv[1];
    string code = loadFile(inputFile);

    cout << "=== Código fuente cargado ===\n\n";
    cout << code << "\n\n";

    // ----------------------------
    // 1. Ejecutar el SCANNER
    // ----------------------------
    cout << "=== Ejecutando SCANNER ===\n";

    Scanner scanner(code.c_str());
    ejecutar_scanner(&scanner, inputFile);

    // ----------------------------
    // 2. Ejecutar el PARSER
    // ----------------------------
    cout << "\n=== Ejecutando PARSER ===\n";

    try {
        // IMPORTANTE: un parser necesita un scanner nuevo
        Scanner* parserScanner = new Scanner(code.c_str());
        Parser* parser = new Parser(parserScanner);

        Program* prog = parser->parseProgram();

        cout << "\n>>> Parser exitoso\n";

        // ----------------------------
        // 3. Ejecutar TYPECHECKER
        // ----------------------------
        cout << "\n=== Ejecutando TYPECHECKER ===\n";

        TypeChecker tc;
        tc.typecheck(prog);

        cout << "\n>>> Typechecking exitoso\n";


        delete prog;
        delete parser;
        delete parserScanner;
    }
    catch (exception& ex) {
        cout << "\n*** Error en parser: " << ex.what() << " ***\n";
    }

    return 0;
}