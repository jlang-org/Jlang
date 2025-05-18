#pragma once

#include "../Ast.h"

namespace jlang
{

struct TypeRef
{
    std::string name;
    bool isPointer = false;
};

struct InterfaceDecl : public AstNode
{
    std::string name;
    std::vector<std::string> methods;

    InterfaceDecl() { type = NodeType::InterfaceDecl; }

    void Accept(AstVisitor &visitor) override { visitor.VisitInterfaceDecl(*this); }
};

struct StructField
{
    std::string name;
    TypeRef type;
};

struct StructDecl : public AstNode
{
    std::string name;
    std::string interfaceImplemented;
    std::vector<StructField> fields;

    StructDecl() { type = NodeType::StructDecl; }

    void Accept(AstVisitor &visitor) override { visitor.VisitStructDecl(*this); }
};

struct Parameter
{
    std::string name;
    TypeRef type;
};

struct FunctionDecl : public AstNode
{
    std::string name;
    std::vector<Parameter> params;
    TypeRef returnType;
    std::shared_ptr<AstNode> body;

    FunctionDecl() { type = NodeType::FunctionDecl; }

    void Accept(AstVisitor &visitor) override { visitor.VisitFunctionDecl(*this); }
};

struct VariableDecl : public AstNode
{
    std::string name;
    TypeRef varType;
    std::shared_ptr<AstNode> initializer;

    VariableDecl() { type = NodeType::VariableDecl; }

    void Accept(AstVisitor &visitor) override { visitor.VisitVariableDecl(*this); }
};

} // namespace jlang
