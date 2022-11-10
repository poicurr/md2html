#pragma once

#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

using namespace std::string_literals;

namespace m2h {

enum class NodeType {
  None = 0,
  Paragraph,
  BlockQuote,
  UnorderedList,
  UnorderedListItem,
  OrderedList,
  OrderedListItem,
  EmptyLine,
  Horizontal,
  Heading,
  InlineCode,
  CodeBlock,
};

struct Node {
  explicit Node(NodeType&& type) : type{type}, children{} {}
  virtual void print(const std::string& prefix) = 0;
  virtual NodeType getType() { return type; }
  void addChild(Node* node) { children.push_back(node); }
  NodeType type;
  std::vector<Node*> children;
};

struct RootNode : Node {
  RootNode() : Node(NodeType::None) {}
  virtual void print(const std::string& prefix) override {
    for (auto&& child : children) {
      child->print("");
    }
  }
};

struct HeadingNode : Node {
  HeadingNode(int level, const std::string& heading)
      : Node(NodeType::Heading), level{level}, heading{heading} {}

  virtual void print(const std::string& prefix) override {
    const std::string lvl = std::to_string(level);
    std::string t1 = "<h"s + lvl + ">"s;
    std::string t2 = "</h"s + lvl + ">"s;
    std::cout << prefix << t1 << heading << t2 << std::endl;
  }

  int level;
  std::string heading;
};

struct BlockQuoteNode : Node {
  BlockQuoteNode() : Node(NodeType::BlockQuote) {}
  virtual void print(const std::string& prefix) override {
    std::cout << prefix << "<blockquote>" << std::endl;
    for (auto&& childNode : children) {
      childNode->print(prefix + "  ");
    }
    std::cout << prefix << "</blockquote>" << std::endl;
  }
};

struct ParagraphNode : Node {
  ParagraphNode(const std::string& text)
      : Node(NodeType::Paragraph), text{text} {}
  virtual void print(const std::string& prefix) override {
    std::cout << prefix << "<p>" << text << "</p>" << std::endl;
  }
  std::string text;
};

struct OrderedListNode : Node {
  OrderedListNode() : Node(NodeType::OrderedList) {}
  virtual void print(const std::string& prefix) override {
    std::cout << prefix << "<ol>" << std::endl;
    for (auto&& childNode : children) {
      childNode->print(prefix + "  ");
    }
    std::cout << prefix << "</ol>" << std::endl;
  }
};

struct OrderedListItemNode : Node {
  OrderedListItemNode(const std::string& text)
      : Node(NodeType::OrderedListItem), text{text} {}
  virtual void print(const std::string& prefix) override {
    std::cout << prefix << "<li>" << text << "</li>" << std::endl;
  }
  std::string text;
};

struct UnorderedListNode : Node {
  UnorderedListNode() : Node(NodeType::UnorderedList) {}
  virtual void print(const std::string& prefix) override {
    std::cout << prefix << "<ul>" << std::endl;
    for (auto&& childNode : children) {
      childNode->print(prefix + "  ");
    }
    std::cout << prefix << "</ul>" << std::endl;
  }
};

struct UnorderedListItemNode : Node {
  UnorderedListItemNode(const std::string& text)
      : Node(NodeType::UnorderedListItem), text{text} {}
  virtual void print(const std::string& prefix) override {
    std::cout << prefix << "<li>" << text << "</li>" << std::endl;
  }
  std::string text;
};

struct HorizontalNode : Node {
  HorizontalNode() : Node(NodeType::Horizontal) {}
  virtual void print(const std::string& prefix) override {
    std::cout << "<hr />" << std::endl;
  }
};

struct CodeBlockNode : Node {
  CodeBlockNode(const std::string& text)
      : Node(NodeType::CodeBlock), lines{text} {}
  virtual void print(const std::string& prefix) override {
    std::cout << prefix << "<pre><code>";
    for (auto line : lines) std::cout << line << std::endl;
    std::cout << "</code></pre>" << std::endl;
  }
  std::vector<std::string> lines;
};

struct EmptyLineNode : Node {
  EmptyLineNode() : Node(NodeType::EmptyLine) {}
  virtual void print(const std::string& prefix) override {
    std::cout << "<p><!-- empty --></p>" << std::endl;
  }
};

}  // namespace m2h
