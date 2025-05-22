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
    for (const auto &node : program)
    {
        if (node)
        {
            node->accept(*this);
        }
    }
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

void CodeGenerator::VisitInterfaceDecl(InterfaceDecl &)
{
    JLANG_DEBUG(STR("Skipping codegen for interface: %s", node.name.c_str()));
}

void CodeGenerator::VisitStructDecl(StructDecl &)
{
    std::vector<llvm::Type *> fieldTypes;
    for (const auto &field : node.fields)
    {
        fieldTypes.push_back(MapType(field.type));
    }

    llvm::StructType *structType = llvm::StructType::create(m_Context, node.name);
    structType->setBody(fieldTypes);

    JLANG_DEBUG(STR("Defined struct type: %s", node.name.c_str()));
}

void CodeGenerator::VisitVariableDecl(VariableDecl &)
{
    llvm::Type *varType = MapType(node.varType);
    if (!varType)
    {
        JLANG_ERROR(STR("Unknown variable type: %s", node.varType.name.c_str()));
        return;
    }

    llvm::AllocaInst *alloca = m_IRBuilder.CreateAlloca(varType, nullptr, node.name);

    if (node.initializer)
    {
        node.initializer->accept(*this);
        if (!m_LastValue)
        {
            JLANG_ERROR(STR("Failed to evaluate initializer for variable: %s", node.name.c_str()));
            return;
        }

        m_IRBuilder.CreateStore(m_LastValue, alloca);
    }

    m_namedValues[node.name] = alloca;
}

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

void CodeGenerator::VisitExprStatement(ExprStatement &)
{
    if (node.expression)
    {
        node.expression->accept(*this);
        // m_LastValue is ignored — result discarded
    }
}

void CodeGenerator::VisitCallExpr(CallExpr &)
{
    llvm::Function *callee = m_Module->getFunction(node.callee);

    if (!callee)
    {
        JLANG_ERROR(STR("Unknown function: %s", node.callee.c_str()));
        return;
    }

    std::vector<llvm::Value *> args;
    for (auto &arg : node.arguments)
    {
        arg->accept(*this);
        if (!m_LastValue)
        {
            JLANG_ERROR(STR("Invalid argument in call to %s", node.callee.c_str()));
            return;
        }
        args.push_back(m_LastValue);
    }

    m_LastValue = m_IRBuilder.CreateCall(callee, args, node.callee + "_call");
}

void CodeGenerator::VisitBinaryExpr(BinaryExpr &)
{
    node.left->accept(*this);
    llvm::Value *lhs = m_LastValue;

    node.right->accept(*this);
    llvm::Value *rhs = m_LastValue;

    if (!lhs || !rhs)
    {
        JLANG_ERROR("Invalid operands in binary expression");
        return;
    }

    if (node.op == "+")
    {
        if (lhs->getType()->isIntegerTy() && rhs->getType()->isIntegerTy())
        {
            m_LastValue = m_IRBuilder.CreateAdd(lhs, rhs, "addtmp");
        }
        else
        {
            JLANG_ERROR("Addition only supported on integer types");
        }
    }
    else if (node.op == "==")
    {
        m_LastValue = m_IRBuilder.CreateICmpEQ(lhs, rhs, "eqtmp");
    }
    else
    {
        JLANG_ERROR(STR("Unsupported binary operator: %s", node.op.c_str()));
    }
}

void CodeGenerator::VisitLiteralExpr(LiteralExpr &)
{
    // TODO --> don't to it like this
    //  Try to parse as integer literal
    try
    {
        int value = std::stoi(node.value);
        m_LastValue = llvm::ConstantInt::get(m_Context, llvm::APInt(32, value));
    }
    catch (const std::invalid_argument &)
    {
        // Not a number — treat as string
        m_LastValue = m_IRBuilder.CreateGlobalStringPtr(node.value);
    }
}

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
