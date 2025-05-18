#pragma once

namespace jlang
{
enum class TokenType
{
    // Keywords
    Interface,
    Struct,
    Var,
    Void,
    Int32,
    If,
    Else,
    Return,

    // Symbols
    LBrace,
    RBrace,
    LParen,
    RParen,
    Semicolon,
    Colon,
    Arrow,
    Assign,
    Star,
    Comma,
    Dot,
    NotEqual,
    EqualEqual,
    Less,
    Greater,
    Equal,

    // Literals and identifiers
    Identifier,
    StringLiteral,
    NumberLiteral,

    EndOfFile,
    Unknown
};

} // namespace jlang
