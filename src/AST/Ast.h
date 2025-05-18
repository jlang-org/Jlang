#pragma once

#include "../CodeGen/AstVisitor.h"
#include "../Enums/NodeTypes.h"

#include <memory>
#include <string>
#include <vector>

namespace jlang
{

struct AstNode
{
    NodeType type;

    virtual ~AstNode() = default;

    virtual void Accept(AstVisitor &visitor) = 0;
};

using AstNodePtr = std::shared_ptr<AstNode>;

} // namespace jlang
