#pragma once

#include "../Ast.h"

namespace jlang
{

struct Expression : public AstNode
{
};

struct CallExpr : public Expression
{
    std::string callee;
    std::vector<std::shared_ptr<AstNode>> arguments;

    CallExpr() { type = NodeType::CallExpr; }

    void Accept(AstVisitor &visitor) override { visitor.VisitCallExpr(*this); }
};

struct BinaryExpr : public Expression
{
    std::string op;
    std::shared_ptr<AstNode> left;
    std::shared_ptr<AstNode> right;

    BinaryExpr() { type = NodeType::BinaryExpr; }

    void Accept(AstVisitor &visitor) override { visitor.VisitBinaryExpr(*this); }
};

struct VarExpr : public Expression
{
    std::string name;

    VarExpr() { type = NodeType::VarExpr; }

    void Accept(AstVisitor &visitor) override { visitor.VisitVarExpr(*this); }
};

struct LiteralExpr : public Expression
{
    std::string value;

    LiteralExpr() { type = NodeType::LiteralExpr; }

    void Accept(AstVisitor &visitor) override { visitor.VisitLiteralExpr(*this); }
};

struct CastExpr : public Expression
{
    TypeRef targetType;
    std::shared_ptr<AstNode> expr;

    CastExpr() { type = NodeType::CastExpr; }

    void Accept(AstVisitor &visitor) override { visitor.VisitCastExpr(*this); }
};

} // namespace jlang
