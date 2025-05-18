#pragma once

#include "../AST/Ast.h"
#include "../AST/Expressions/Expressions.h"
#include "../AST/Statements/Statements.h"
#include "../AST/TopLevelDecl/TopLevelDecl.h"

namespace jlang
{
class AstVisitor
{
  public:
    virtual ~AstVisitor() = default;

    virtual void VisitFunctionDecl(FunctionDecl &) = 0;
    virtual void VisitInterfaceDecl(InterfaceDecl &) = 0;
    virtual void VisitStructDecl(StructDecl &) = 0;
    virtual void VisitVariableDecl(VariableDecl &) = 0;

    virtual void VisitIfStatement(IfStatement &) = 0;
    virtual void VisitBlockStatement(BlockStatement &) = 0;
    virtual void VisitExprStatement(ExprStatement &) = 0;

    virtual void VisitCallExpr(CallExpr &) = 0;
    virtual void VisitBinaryExpr(BinaryExpr &) = 0;
    virtual void VisitLiteralExpr(LiteralExpr &) = 0;
    virtual void VisitVarExpr(VarExpr &) = 0;
    virtual void VisitCastExpr(CastExpr &) = 0;
};
} // namespace jlang