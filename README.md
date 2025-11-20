# ProyectoRustBase

Base mínima para el proyecto del compilador tipo Rust.

## Qué incluye

- Estructura de directorios (`src/lexic`, `src/syntactic`, `src/semantic`, `input`, `output`).
- `main.cpp` que conecta Scanner, Parser, TypeCheck y CodeGen.
- Definición de tipos numéricos básicos: `i32`, `i64`, `u32`, `f32`.
- Esqueleto para:
  - Analizador léxico (`Scanner`).
  - AST (`Exp`, `Stmt`, `Program`).
  - Visitor, TypeCheck y CodeGen.
  - Espacio para expresión ternaria `?:` y sobrecarga básica de operadores numéricos.

## Cómo usar

1. Crear build:

```bash
mkdir build
cd build
cmake ..
make
```

2. Ejecutar:

```bash
./proyecto_rust_base ../input/test.rs
```
