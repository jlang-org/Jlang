#include "../Lexer/Lexer.h"

#include <cctype>
#include <unordered_map>

namespace jlang
{

static std::unordered_map<std::string, TokenType> s_Keywords = {
    {"interface", TokenType::Interface}, {"struct", TokenType::Struct}, {"void", TokenType::Void},
    {"int32", TokenType::Int32},         {"var", TokenType::Var},       {"if", TokenType::If},
    {"else", TokenType::Else},           {"return", TokenType::Return}};

Lexer::Lexer(const std::string &source) : m_Source(source) {}

std::vector<Token> Lexer::Tokenize()
{
    while (!IsEndReached())
    {
        m_Start = m_CurrentPosition;
        ScanToken();
    }

    m_Tokens.emplace_back(TokenType::EndOfFile, "", m_CurrentLine);
    return m_Tokens;
}

void Lexer::ScanToken()
{
    SkipWhitespace();

    if (IsEndReached())
    {
        return;
    }

    char c = Advance();

    switch (c)
    {
    case '{':
        AddToken(TokenType::LBrace);
        break;
    case '}':
        AddToken(TokenType::RBrace);
        break;
    case '(':
        AddToken(TokenType::LParen);
        break;
    case ')':
        AddToken(TokenType::RParen);
        break;
    case ';':
        AddToken(TokenType::Semicolon);
        break;
    case ',':
        AddToken(TokenType::Comma);
        break;
    case '.':
        AddToken(TokenType::Dot);
        break;
    case '*':
        AddToken(TokenType::Star);
        break;
    case '=':
        AddToken(IsMatched('=') ? TokenType::EqualEqual : TokenType::Equal);
        break;
    case '!':
        AddToken(IsMatched('=') ? TokenType::NotEqual : TokenType::Unknown);
        break;
    case '<':
        AddToken(TokenType::Less);
        break;
    case '>':
        AddToken(TokenType::Greater);
        break;
    case '-':
        if (IsMatched('>'))
        {
            AddToken(TokenType::Arrow);
        }
        else
        {
            AddToken(TokenType::Unknown);
        }
        break;
    case '"':
        AddStringLiteral();
        break;
    default:
        if (std::isdigit(c))
        {
            AddNumber();
        }
        else if (std::isalpha(c) || c == '_')
        {
            AddIdentifier();
        }
        else
        {
            AddToken(TokenType::Unknown);
        }
    }
}

void Lexer::SkipWhitespace()
{
    while (!IsEndReached())
    {
        char c = Peek();

        if (c == ' ' || c == '\r' || c == '\t')
        {
            Advance();
        }
        else if (c == '\n')
        {
            m_CurrentLine++;
            Advance();
        }
        else
        {
            break;
        }
    }
}

char Lexer::Advance()
{
    return m_Source[m_CurrentPosition++];
}

char Lexer::Peek() const
{
    return IsEndReached() ? '\0' : m_Source[m_CurrentPosition];
}

char Lexer::PeekNext() const
{
    return (m_CurrentPosition + 1 >= m_Source.length()) ? '\0' : m_Source[m_CurrentPosition + 1];
}

bool Lexer::IsMatched(char expected)
{
    if (IsEndReached() || m_Source[m_CurrentPosition] != expected)
    {
        return false;
    }

    m_CurrentPosition++;
    return true;
}

bool Lexer::IsEndReached() const
{
    return m_CurrentPosition >= m_Source.length();
}

void Lexer::AddToken(TokenType type)
{
    AddToken(type, m_Source.substr(m_Start, m_CurrentPosition - m_Start));
}

void Lexer::AddToken(TokenType type, const std::string &lexeme)
{
    m_Tokens.emplace_back(type, lexeme, m_CurrentLine);
}

void Lexer::AddIdentifier()
{
    while (std::isalnum(Peek()) || Peek() == '_')
    {
        Advance();
    }

    std::string text = m_Source.substr(m_Start, m_CurrentPosition - m_Start);
    TokenType type = IsKeywordOrIndetifier(text);

    AddToken(type, text);
}

void Lexer::AddNumber()
{
    while (std::isdigit(Peek()))
    {
        Advance();
    }

    AddToken(TokenType::NumberLiteral);
}

void Lexer::AddStringLiteral()
{
    while (Peek() != '"' && !IsEndReached())
    {
        if (Peek() == '\n')
        {
            m_CurrentLine++;
        }

        Advance();
    }

    if (IsEndReached())
    {
        return;
    }

    Advance();

    std::string value = m_Source.substr(m_Start + 1, m_CurrentPosition - m_Start - 2);

    AddToken(TokenType::StringLiteral, value);
}

TokenType Lexer::IsKeywordOrIndetifier(const std::string &text)
{
    auto it = s_Keywords.find(text);
    return it != s_Keywords.end() ? it->second : TokenType::Identifier;
}

} // namespace jlang
