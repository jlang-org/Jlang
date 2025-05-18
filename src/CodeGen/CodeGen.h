#pragma once

#include "AstVisitor.h"

#include "../AST/Ast.h"
#include "../AST/Expressions/Expressions.h"
#include "../AST/Statements/Statements.h"
#include "../AST/TopLevelDecl/TopLevelDecl.h"

#include <memory>
#include <unordered_map>

#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>

namespace jlang
{

class CodeGenerator : public AstVisitor
{
  public:
    CodeGenerator();

    void Generate(const std::vector<std::shared_ptr<AstNode>> &program);
    void DumpIR();

  private:
    virtual void VisitFunctionDecl(FunctionDecl &) override;
    virtual void VisitInterfaceDecl(InterfaceDecl &) override;
    virtual void VisitStructDecl(StructDecl &) override;
    virtual void VisitVariableDecl(VariableDecl &) override;

    virtual void VisitIfStatement(IfStatement &) override;
    virtual void VisitBlockStatement(BlockStatement &) override;
    virtual void VisitExprStatement(ExprStatement &) override;

    virtual void VisitCallExpr(CallExpr &) override;
    virtual void VisitBinaryExpr(BinaryExpr &) override;
    virtual void VisitLiteralExpr(LiteralExpr &) override;
    virtual void VisitVarExpr(VarExpr &) override;
    virtual void VisitCastExpr(CastExpr &) override;

  private:
    llvm::Type *MapType(const TypeRef &typeRef);

  private:
    llvm::LLVMContext m_Context;
    std::unique_ptr<llvm::Module> m_Module;
    llvm::IRBuilder<> m_IRBuilder;

    std::unordered_map<std::string, llvm::Value *> m_namedValues;
    llvm::Value *m_LastValue = nullptr;
};

} // namespace jlang
