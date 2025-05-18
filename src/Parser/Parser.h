#pragma once

#include "../AST/Ast.h"

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace jlang
{

class Parser
{
  public:
    explicit Parser(const std::vector<Token> &tokens);
    std::vector<std::shared_ptr<AstNode>> Parse();

  private:
    bool IsMatched(TokenType type);
    bool Check(TokenType type) const;
    const Token &Advance();
    const Token &Peek() const;
    const Token &Previous() const;
    bool IsEndReached() const;

    std::shared_ptr<AstNode> ParseDeclaration();
    std::shared_ptr<AstNode> ParseInterface();
    std::shared_ptr<AstNode> ParseStruct();
    std::shared_ptr<AstNode> ParseFunction();
    std::shared_ptr<AstNode> ParseStatement();
    std::shared_ptr<AstNode> ParseBlock();
    std::shared_ptr<AstNode> ParseIfStatement();
    std::shared_ptr<AstNode> ParseExpression();
    std::shared_ptr<AstNode> ParseExprStatement();
    std::shared_ptr<AstNode> ParsePrimary();

  private:
    const std::vector<Token> &m_Tokens;
    size_t m_CurrentPosition;
};

} // namespace jlang
