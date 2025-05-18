#include "CodeGen.h"

#include "../Common/Logger.h"

#include <iostream>

#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Verifier.h>

namespace jlang
{

CodeGenerator::CodeGenerator()
    : m_Module(std::make_unique<llvm::Module>("JlangModule", m_Context)), m_IRBuilder(m_Context)
{
}

void CodeGenerator::Generate(const std::vector<std::shared_ptr<AstNode>> &program)
{
}

void CodeGenerator::VisitFunctionDecl(FunctionDecl &)
{
    std::vector<llvm::Type *> paramTypes;
    for (const auto &param : node.params)
    {
        paramTypes.push_back(MapType(param.type));
    }

    llvm::FunctionType *funcType = llvm::FunctionType::get(MapType(node.returnType), paramTypes, false);

    llvm::Function *function =
        llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, node.name, m_Module.get());

    llvm::BasicBlock *entry = llvm::BasicBlock::Create(m_Context, "entry", function);
    m_IRBuilder.SetInsertPoint(entry);

    unsigned i = 0;
    for (auto &arg : function->args())
    {
        arg.setName(node.params[i].name);
        m_namedValues[node.params[i].name] = &arg;
        ++i;
    }

    if (node.body)
    {
        node.body->Accept(*this);
    }

    if (node.returnType.name == "void")
    {
        m_IRBuilder.CreateRetVoid();
    }

    llvm::verifyFunction(*function);
}
void CodeGenerator::VisitInterfaceDecl(InterfaceDecl &) {}
void CodeGenerator::VisitStructDecl(StructDecl &) {}
void CodeGenerator::VisitVariableDecl(VariableDecl &) {}

void CodeGenerator::VisitIfStatement(IfStatement &)
{
    node.condition->Accept(*this);
    llvm::Value *isConditionalValue = m_LastValue;

    if (!isConditionalValue)
    {
        JLANG_ERROR("Invalid condition in if statement");
    }

    if (isConditionalValue->getType()->isIntegerTy(32))
    {
        isConditionalValue = m_IRBuilder.CreateICmpNE(
            isConditionalValue, llvm::ConstantInt::get(isConditionalValue->getType(), 0), "ifcond");
    }

    llvm::Function *parentFunction = m_IRBuilder.GetInsertBlock()->getParent();

    llvm::BasicBlock *thenBlock = llvm::BasicBlock::Create(m_Context, "then", parentFunction);
    llvm::BasicBlock *elseBlock = llvm::BasicBlock::Create(m_Context, "else");
    llvm::BasicBlock *mergeBlock = llvm::BasicBlock::Create(m_Context, "ifcont");

    m_IRBuilder.CreateCondBr(isConditionalValue, thenBlock, elseBlock);

    m_IRBuilder.SetInsertPoint(thenBlock);
    node.thenBranch->Accept(*this);
    m_IRBuilder.CreateBr(mergeBlock);

    parentFunction->getBasicBlockList().push_back(elseBlock);
    m_IRBuilder.SetInsertPoint(elseBlock);
    if (node.elseBranch)
    {
        node.elseBranch->Accept(*this);
    }
    m_IRBuilder.CreateBr(mergeBlock);

    parentFunction->getBasicBlockList().push_back(mergeBlock);
    m_IRBuilder.SetInsertPoint(mergeBlock);
}

void CodeGenerator::VisitBlockStatement(BlockStatement &)
{
    for (auto &statement : node.statements)
    {
        if (statement)
        {
            statement->Accept(*this);
        }
    }
}

void CodeGenerator::VisitExprStatement(ExprStatement &) {}

void CodeGenerator::VisitCallExpr(CallExpr &) {}

void CodeGenerator::VisitBinaryExpr(BinaryExpr &) {}

void CodeGenerator::VisitLiteralExpr(LiteralExpr &) {}

void CodeGenerator::VisitCallExpr(BinaryExpr &)
{
    llvm::Function *callee = m_Module->getFunction(node.callee);

    if (!callee)
    {
        JLANG_ERROR(STR("Unknown function: %s", node.callee.c_str()));
    }

    std::vector<llvm::Value *> args;

    for (auto &arg : node.arguments)
    {
        arg->Accept(*this);

        if (!m_LastValue)
        {
            JLANG_ERROR(STR("Invalid argument in call to %s", node.callee.c_str()));
        }
        args.push_back(m_LastValue);
    }

    m_LastValue = m_IRBuilder.CreateCall(callee, args, node.callee + "_call");
}

void CodeGenerator::VisitVarExpr(VarExpr &)
{
    auto it = m_namedValues.find(node.name);

    if (it == m_namedValues.end())
    {
        JLANG_ERROR(STR("Undefined variable: %s", node.name.c_str()));
    }

    m_LastValue = it->second;
}

void CodeGenerator::VisitCastExpr(CastExpr &)
{
    node.expr->Accept(*this);
    llvm::Value *valueToCast = m_LastValue;

    if (!valueToCast)
    {
        JLANG_ERROR("Invalid expression in cast");
    }

    llvm::Type *targetLLVMType = MapType(node.targetType);

    if (!targetLLVMType)
    {
        JLANG_ERROR("Unknown target type in cast");
    }

    bool isPointerToPointerCast = valueToCast->getType()->isPointerTy() && targetLLVMType->isPointerTy();

    if (isPointerToPointerCast)
    {
        m_LastValue = m_IRBuilder.CreateBitCast(valueToCast, targetLLVMType, "ptrcast");
    }
    else
    {
        JLANG_ERROR(STR("Unsupported cast from %s to %s",
                        valueToCast->getType()->getStructName().str().c_str(),
                        targetLLVMType->getStructName().str().c_str()));
    }
}

llvm::Type *CodeGenerator::MapType(const TypeRef &typeRef)
{
    if (typeRef.name == "void")
    {
        return llvm::Type::getVoidTy(m_Context);
    }

    if (typeRef.name == "int32")
    {
        return llvm::Type::getInt32Ty(m_Context);
    }

    if (typeRef.name == "char")
    {
        llvm::Type *charType = llvm::Type::getInt8Ty(m_Context);
        return typeRef.isPointer ? llvm::PointerType::getUnqual(charType) : charType;
    }

    return llvm::Type::getVoidTy(m_Context);
}

} // namespace jlang
