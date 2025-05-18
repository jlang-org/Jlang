#pragma once

namespace jlang
{
enum class NodeType
{
    InterfaceDecl,
    StructDecl,
    FunctionDecl,
    VariableDecl,

    IfStatement,
    BlockStatement,
    ExprStatement,

    CallExpr,
    BinaryExpr,
    VarExpr,
    LiteralExpr,
    CastExpr
};
} // namespace jlang