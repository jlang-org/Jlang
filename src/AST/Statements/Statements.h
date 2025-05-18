#pragma once

#include "../Ast.h"

namespace jlang
{

struct Statement : public AstNode
{
};

struct IfStatement : public Statement
{
    std::shared_ptr<AstNode> condition;
    std::shared_ptr<AstNode> thenBranch;
    std::shared_ptr<AstNode> elseBranch;

    IfStatement() { type = NodeType::IfStatement; }

    void Accept(AstVisitor &visitor) override { visitor.VisitIfStatement(*this); }
};

struct BlockStatement : public Statement
{
    std::vector<std::shared_ptr<AstNode>> statements;

    BlockStatement() { type = NodeType::BlockStatement; }

    void Accept(AstVisitor &visitor) override { visitor.VisitBlockStatement(*this); }
};

struct ExprStatement : public Statement
{
    std::shared_ptr<AstNode> expression;

    ExprStatement() { type = NodeType::ExprStatement; }

    void Accept(AstVisitor &visitor) override { visitor.VisitExprStatement(*this) : }
};

} // namespace jlang
