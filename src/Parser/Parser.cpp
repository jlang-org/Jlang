#include "Parser.h"

#include "../Common/Logger.h"

namespace jlang
{

Parser::Parser(const std::vector<Token> &tokens) : m_Tokens(tokens), m_CurrentPosition(0) {}

std::vector<std::shared_ptr<AstNode>> Parser::Parse()
{
    std::vector<std::shared_ptr<AstNode>> program;

    while (!IsEndReached())
    {
        auto declaration = ParseDeclaration();

        if (declaration)
        {
            program.push_back(declaration);
        }
    }

    return program;
}

bool Parser::IsMatched(TokenType type)
{
    if (Check(type))
    {
        Advance();
        return true;
    }

    return false;
}

bool Parser::Check(TokenType type) const
{
    if (IsEndReached())
    {
        return false;
    }

    return Peek().m_type == type;
}

const Token &Parser::Advance()
{
    if (!IsEndReached())
    {
        m_CurrentPosition++;
    }

    return Previous();
}

const Token &Parser::Peek() const
{
    return m_Tokens[m_CurrentPosition];
}

const Token &Parser::Previous() const
{
    return m_Tokens[m_CurrentPosition - 1];
}

bool Parser::IsEndReached() const
{
    return Peek().m_type == TokenType::EndOfFile;
}

// Everything is kinda hardcoded for now!! -> will change that later on, just trying to get stuff rolling..
std::shared_ptr<AstNode> Parser::ParseDeclaration()
{
    if (Check(TokenType::Interface))
    {
        return ParseInterface();
    }

    if (Check(TokenType::Struct))
    {
        return ParseStruct();
    }

    if (Check(TokenType::Void) || Check(TokenType::Int32))
    {
        return ParseFunction();
    }

    Advance();

    return nullptr;
}

std::shared_ptr<AstNode> Parser::ParseInterface()
{
    Advance();

    if (!IsMatched(TokenType::Identifier))
    {
        JLANG_ERROR("Expected interaface name");
    }

    const std::string &name = Previous().m_lexeme;

    if (!IsMatched(TokenType::LBrace))
    {
        JLANG_ERROR("Expected '{' after interface name!");
    }

    auto interfaceDeclNode = std::make_shared<InterfaceDecl>();
    interfaceDeclNode->name = name;

    while (!Check(TokenType::RBrace) && !IsEndReached())
    {
        if (!IsMatched(TokenType::Void))
        {
            if (!IsMatched(TokenType::Void))
            {
                JLANG_ERROR("Expected 'void' in interface method");
            }

            if (!IsMatched(TokenType::Identifier))
            {
                JLANG_ERROR("Expected method name");
            }

            std::string methodName = Previous().m_lexeme;

            if (!IsMatched(TokenType::LParen) || !IsMatched(TokenType::RParen) ||
                !IsMatched(TokenType::Semicolon))
            {
                JLANG_ERROR("Expected '()' and ';' after method name");
            }

            interfaceDeclNode->methods.push_back(methodName);
        }
    }

    if (!IsMatched(TokenType::RBrace))
    {
        JLANG_ERROR("Expected '}' at end of interface");
    }

    return interfaceDeclNode;
}

std::shared_ptr<AstNode> Parser::ParseStruct()
{
    Advance();

    if (!IsMatched(TokenType::Identifier))
    {
        JLANG_ERROR("Expected struct name");
    }

    const std::string name = Previous().m_lexeme;

    std::string implementedInterface;

    if (IsMatched(TokenType::Arrow))
    {
        if (!IsMatched(TokenType::Identifier))
        {
            JLANG_ERROR("Expected interface name after '->'");
        }

        implementedInterface = Previous().m_lexeme;
    }

    if (!IsMatched(TokenType::LBrace))
    {
        JLANG_ERROR("Expected '{' after struct declaration");
    }

    auto structDeclNode = std::make_shared<StructDecl>();
    structDeclNode->name = name;
    structDeclNode->interfaceImplemented = implementedInterface;

    while (!Check(TokenType::RBrace) && !IsEndReached())
    {
        if (!IsMatched(TokenType::Identifier))
        {
            JLANG_ERROR("Expected field name");
        }

        std::string fieldName = Previous().m_lexeme;

        if (!IsMatched(TokenType::Identifier))
        {
            JLANG_ERROR("Expected field type");
        }

        std::string typeName = Previous().m_lexeme;
        bool isPointer = false;

        if (IsMatched(TokenType::Star))
        {
            isPointer = true;
        }

        if (!IsMatched(TokenType::Semicolon))
        {
            JLANG_ERROR("Expected ';' after struct field");
        }

        StructField field{fieldName, TypeRef{typeName, isPointer}};
        structDeclNode->fields.push_back(field);
    }

    if (!IsMatched(TokenType::RBrace))
    {
        JLANG_ERROR("Expected '}' after struct body");
    }

    return structDeclNode;
}

std::shared_ptr<AstNode> Parser::ParseFunction()
{
    Advance();

    TokenType returnTokenType = Previous().m_type;

    if (!IsMatched(TokenType::Identifier))
    {
        JLANG_ERROR("Expected function name!");
    }

    const std::string &functionName = Previous().m_lexeme;

    // Hardcoded for no arguments, currently ..... will change that
    if (!IsMatched(TokenType::LParen) || !IsMatched(TokenType::RParen))
    {
        JLANG_ERROR("Expected () after function name");
    }

    TypeRef returnType;

    if (returnTokenType == TokenType::Void)
    {
        returnType = TypeRef{"void", false};
    }
    else
    {
        returnType = TypeRef{Previous().m_lexeme, false};
    }

    std::vector<Parameter> params;

    if (IsMatched(TokenType::Arrow))
    {
        if (!IsMatched(TokenType::Identifier))
        {
            JLANG_ERROR("Expected paramter type identifier '->' ");
        }

        const std::string &paramType = Previous().m_lexeme;
        bool isPointer = IsMatched(TokenType::Star);

        if (!IsMatched(TokenType::Identifier))
        {
            JLANG_ERROR("Expected paramter name!");
        }

        const std::string &paramName = Previous().m_lexeme;

        params.push_back(Parameter{paramName, TypeRef{paramType, isPointer}});
    }

    auto body = ParseBlock();

    auto functionDeclNode = std::make_shared<FunctionDecl>();
    functionDeclNode->name = functionName;
    functionDeclNode->params = params;
    functionDeclNode->returnType = returnType;
    functionDeclNode->body = body;

    return functionDeclNode;
}

std::shared_ptr<AstNode> Parser::ParseBlock()
{
    if (!IsMatched(TokenType::LBrace))
    {
        JLANG_ERROR("Expected '{' at the beginning of the block");
    }

    auto blockStmt = std::make_shared<BlockStatement>();

    while (!Check(TokenType::RBrace) && !IsEndReached())
    {
        auto statement = ParseStatement();

        if (statement)
        {
            blockStmt->statements.push_back(statement);
        }
    }

    if (!IsMatched(TokenType::RBrace))
    {
        JLANG_ERROR("Expected '}' after block");
    }

    return blockStmt;
}

std::shared_ptr<AstNode> Parser::ParseStatement()
{
    return std::shared_ptr<AstNode>();
}

std::shared_ptr<AstNode> Parser::ParseIfStatement()
{
    Advance();

    if (!IsMatched(TokenType::LParen))
    {
        JLANG_ERROR("Expected '(' after 'if'");
    }

    auto condition = ParseExpression();

    if (!IsMatched(TokenType::RParen))
    {
        JLANG_ERROR("Expected ')' after condition");
    }

    auto thenBranch = ParseStatement();

    std::shared_ptr<AstNode> elseBranch = nullptr;
    if (IsMatched(TokenType::Else))
    {
        elseBranch = ParseStatement();
    }

    auto node = std::make_shared<IfStatement>();

    node->condition = condition;
    node->thenBranch = thenBranch;
    node->elseBranch = elseBranch;

    return node;
}

std::shared_ptr<AstNode> Parser::ParseExpression()
{
    auto expersion = ParseExpression();

    if (!IsMatched(TokenType::Semicolon))
    {
        JLANG_ERROR("Expected ';' after expression");
    }

    auto stmt = std::make_shared<ExprStatement>();
    stmt->expression = expersion;

    return stmt;
}

std::shared_ptr<AstNode> Parser::ParseExprStatement()
{
    auto expression = ParseExpression();

    if (!IsMatched(TokenType::Semicolon))
    {
        JLANG_ERROR("Expected ';' after expression");
    }

    auto stmt = std::make_shared<ExprStatement>();
    stmt->expression = expression;

    return stmt;
}

std::shared_ptr<AstNode> Parser::ParsePrimary()
{

    if (IsMatched(TokenType::Identifier))
    {
        const std::string &name = Previous().m_lexeme;

        if (IsMatched(TokenType::LParen))
        {
            auto call = std::make_shared<CallExpr>();
            call->callee = name;

            if (!IsMatched(TokenType::RParen))
            {
                do
                {
                    auto arg = ParseExpression();
                    call->arguments.push_back(arg);
                } while (IsMatched(TokenType::Comma));
            }

            if (!IsMatched(TokenType::RParen))
            {
                JLANG_ERROR("Expected ')' after arguments");
            }

            return call;
        }
        else
        {
            auto var = std::make_shared<VarExpr>();
            var->name = name;
            return var;
        }
    }

    if (IsMatched(TokenType::Identifier))
    {
        auto experssion = std::make_shared<VarExpr>();
        experssion->name = Previous().m_lexeme;
        return experssion;
    }

    if (IsMatched(TokenType::StringLiteral))
    {
        auto experssion = std::make_shared<LiteralExpr>();
        experssion->value = Previous().m_lexeme;
        return experssion;
    }

    JLANG_ERROR("Expected expression");
}

} // namespace jlang
