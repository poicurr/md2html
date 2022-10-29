#pragma once

#include <iostream>
#include <list>
#include <string>

#include "../ParsingUtility.hpp"
#include "../tokenizer/Token.hpp"
#include "Node.hpp"
#include "ParsingContext.hpp"

namespace m2h {
namespace parser {

using namespace tokenizer;

class Parser {
 public:
  Parser() {}

  std::vector<Node *> parse(const std::vector<Token> &tokens) {
    Node *root = new RootNode();
    context.parent = root;

    auto onParsed = []() {};

    bool isBeginOfLine = true;

    std::vector<Token>::const_iterator it = tokens.begin();
    while (it != tokens.end()) {
      isBeginOfLine = it->kind == tokenizer::TokenKind::NewLine;
      Node *last = context.lastChildNode();
      bool followingEmptyLine = last && last->getType() == NodeType::EmptyLine;

      if (it->kind == tokenizer::TokenKind::Indent) {
        context.column += it->value.size();
      }

      if (it->kind == tokenizer::TokenKind::Prefix) {
        // BlockQuote/CodeBlock/InlineCode/ListItem/Heading
        auto bak = it;
        if (parseHeadingNode(it)) continue;
        if (parseBlockQuote(it)) continue;
        it = bak;
        context.column += it->value.size();
      }

      if (it->kind == tokenizer::TokenKind::Text) {
        // ListItem/Paragraph
        // コンテキストに依存する．BlockQuoteの直後なら，そっちに内容を加える．
        auto bak = it;
        if (parseParagraphNode(it)) continue;
        it = bak;
      }

      if (it->kind == tokenizer::TokenKind::NewLine) {
        context.column = 0;
        context.parent = root;
        auto prevToken = it - 1;
        if (prevToken->kind == TokenKind::NewLine) {
          context.addNode(new EmptyLineNode());
        }
      }

      ++it;
    }
    return root->children;
  }

 private:
  bool consume(TokenKind kind, std::vector<Token>::const_iterator &it) {
    if (kind != it->kind) return false;
    it++;
    return true;
  }

  bool parseHeadingNode(std::vector<Token>::const_iterator &it) {
    // prefix
    if (it->kind != tokenizer::TokenKind::Prefix) return false;
    int headingLevel = 0;
    for (char c : it->value) {
      if (c != '#') break;
      ++headingLevel;
    }

    if (!headingLevel) return false;
    ++it;

    // text
    if (it->kind != tokenizer::TokenKind::Text) return false;
    const std::string heading = it->value;
    ++it;

    // eol
    if (!consume(tokenizer::TokenKind::NewLine, it)) return false;

    context.addNode(new HeadingNode(headingLevel, heading));
    return true;
  }

  bool parseBlockQuote(std::vector<Token>::const_iterator &it) {
    if (it->kind != tokenizer::TokenKind::Prefix) return false;
    if (it->value != "> ") return false;
    ++it;

    auto lastChildNode = context.lastChildNode();
    if (!lastChildNode || lastChildNode->type != NodeType::BlockQuote) {
      auto blockquote = new BlockQuoteNode();
      context.addNode(blockquote);
      context.parent = blockquote;
    } else {
      context.parent = lastChildNode;
    }
    return true;
  }

  bool parseParagraphNode(std::vector<Token>::const_iterator &it) {
    auto text = it->value;
    ++it;
    // while (it->kind != TokenKind::NewLine) {
    //   // emphasis(s)/text/backquote
    //   ++it;
    // }
    context.addNode(new ParagraphNode(text));
    return true;
  }

 private:
  ParsingContext context;
};

}  // namespace parser
}  // namespace m2h
