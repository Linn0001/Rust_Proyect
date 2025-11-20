#include <iostream>
#include "src/lexic/Scanner.h"
#include "src/syntactic/Parser.h"
#include "src/semantic/TypeCheck.h"
#include "src/semantic/CodeGen.h"

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Uso: " << argv[0] << " archivo.rs\n";
        return 1;
    }

    try {
        Scanner scanner(argv[1]);
        Parser parser(&scanner);
        Program* ast = parser.parse();     // AST raíz

        SymbolTable table;
        TypeCheck tc(&table);
        ast->accept(&tc);                  // chequeo de tipos

        CodeGen cg("out.s");               // genera ensamblador
        ast->accept(&cg);

        delete ast;
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << '\n';
        return 1;
    }

    return 0;
}
