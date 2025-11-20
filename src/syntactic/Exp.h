#pragma once
#include <string>

class Visitor;

struct Value {
    enum Type {
        UNDEFINED,
        BOOL,
        I32,
        I64,
        U32,
        F32,
        UNIT
    } type;
    bool literal {false};
    explicit Value(Type t = UNDEFINED) : type(t) {}
};

class Node {
public:
    int line;
    int col;
    Node(int line, int col) : line(line), col(col) {}
    virtual ~Node() = default;
    virtual Value accept(Visitor* v) = 0;
};

class Exp : public Node {
public:
    using Node::Node;
};

class Stmt : public Node {
public:
    using Node::Node;
};

class Program : public Node {
public:
    using Node::Node;
    // lista de declaraciones globales / funciones
    Value accept(Visitor* v) override;
};
