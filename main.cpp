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
    size_t lastSlash = filepath.find_last_of("/\\");
    size_t lastDot = filepath.find_last_of(".");

    string filename;
    if (lastSlash != string::npos) {
        filename = filepath.substr(lastSlash + 1);
    } else {
        filename = filepath;
    }

    if (lastDot != string::npos) {
        size_t start = (lastSlash != string::npos) ? lastSlash + 1 : 0;
        return filename.substr(0, lastDot - start);
    }

    return filename;
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

        // ----------------------------
        // 4. Ejecutar CODEGEN
        // ----------------------------
        cout << "\n=== Ejecutando CODEGEN ===\n";

        // Crear directorio output si no existe
        string outputDir = "output";
        if (!fs::exists(outputDir)) {
            fs::create_directory(outputDir);
        }

        // Generar nombre del archivo assembly
        string baseName = getBaseName(inputFile);
        string asmFile = outputDir + "/" + baseName + ".s";

        // Abrir archivo de salida
        ofstream asmOut(asmFile);
        if (!asmOut.is_open()) {
            cerr << "Error: no se pudo crear el archivo " << asmFile << endl;
            delete prog;
            delete parser;
            delete parserScanner;
            return 1;
        }

        // Generar código assembly
        GenCodeVisitor codegen(asmOut);
        codegen.generar(prog);
        asmOut.close();

        cout << "\n>>> Generación de código exitosa\n";
        cout << "    Assembly generado en: " << asmFile << "\n";

        delete prog;
        delete parser;
        delete parserScanner;
    }
    catch (exception& ex) {
        cout << "\n*** Error en parser: " << ex.what() << " ***\n";
    }

    return 0;
}