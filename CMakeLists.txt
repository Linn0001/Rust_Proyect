cmake_minimum_required(VERSION 3.10)
project(Rust_Project)

set(CMAKE_CXX_STANDARD 17)

include_directories(src)
include_directories(src/lexic)
include_directories(src/syntactic)
include_directories(src/semantic)

add_executable(Rust_Project
        main.cpp

        src/lexic/scanner.cpp
        src/lexic/token.cpp

        src/syntactic/parser.cpp
        src/syntactic/parser.h
        src/syntactic/ast.cpp
        src/syntactic/ast.h
        src/semantic/visitor.cpp
        src/semantic/visitor.h
        src/semantic/AsmInterpreter.cpp
        src/semantic/AsmInterpreter.h
        src/semantic/DebugTrace.h
        src/semantic/DebugVisitor.cpp
        src/semantic/DebugVisitor.h
        src/semantic/TypeChecker.cpp
        src/semantic/TypeChecker.h
        src/semantic/environment.h
        src/semantic/semantic_types.h
)
