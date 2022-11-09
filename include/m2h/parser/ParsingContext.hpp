#pragma once

#include "Node.hpp"

namespace m2h {

struct ParsingContext {
  int line, column;
  Node *parent;

  NodeType prevNodeType() const {
    auto children = parent->children;
    return children.empty() ? NodeType::None : children.back()->type;
  }

  Node *lastChildNode() {
    auto children = parent->children;
    return children.size() > 0 ? children.back() : nullptr;
  }

  void addNode(Node *node) { parent->children.push_back(node); }
};

}  // namespace m2h
