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

using namespace std;

extern int ejecutar_scanner(Scanner* scanner, const string& InputFile);

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
        cout << "Uso: ./program <archivo.rs>" << endl;
        return 1;
    }

    string inputFile = argv[1];
    string code = loadFile(inputFile);

    // Crear el scanner
    Scanner scanner(code.c_str());

    // Ejecutar scanner
    ejecutar_scanner(&scanner, inputFile);

    return 0;
}

