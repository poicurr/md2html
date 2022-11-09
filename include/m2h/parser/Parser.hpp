#pragma once

#include <iostream>
#include <list>
#include <string>

#include "../ParsingUtility.hpp"
#include "../tokenizer/Token.hpp"
#include "Node.hpp"
#include "ParsingContext.hpp"

namespace m2h {

class Parser {
 public:
  Parser() {}

  std::vector<Node *> parse(const std::vector<Token> &tokens) {
    Node *root = new RootNode();
    context.parent = root;

    std::vector<Token>::const_iterator it = tokens.begin();
    while (it != tokens.end()) {
      if (it->kind == TokenKind::Indent) {
        // CodeBlock
      }

      if (it->kind == TokenKind::BackQuote) {
        // InlineCode
        if (parseInlineCode(it)) continue;
      }

      if (it->kind == TokenKind::Prefix) {
        // BlockQuote/CodeBlock/InlineCode/ListItem/Heading
        if (parseHeading(it)) continue;
        if (parseBlockQuote(it)) continue;
        if (parseOrderedList(it)) continue;
        if (parseUnorderedList(it)) continue;
      }

      if (it->kind == TokenKind::Text) {
        // InlineCode/Emphasis/ListItem/Paragraph
        if (parseOrderedListItem(it)) continue;
        if (parseUnorderedListItem(it)) continue;
        if (parseParagraph(it)) continue;
      }

      if (it->kind == TokenKind::NewLine) {
        context.parent = root;
        auto prevToken = it - 1;
        if (prevToken->kind == TokenKind::NewLine) {
          context.append(new EmptyLineNode());
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

  bool parseHeading(std::vector<Token>::const_iterator &it) {
    if (it->kind != TokenKind::Prefix) return false;
    int headingLevel = 0;
    for (char c : it->value) {
      if (c != '#') break;
      ++headingLevel;
    }
    if (headingLevel == 0) return false;
    ++it;

    if (it->kind != TokenKind::Text) return false;
    const std::string value = it->value;
    ++it;

    if (!consume(TokenKind::NewLine, it)) return false;

    context.append(new HeadingNode(headingLevel, value));
    return true;
  }

  bool parseInlineCode(std::vector<Token>::const_iterator &it) {
    if (it->value != "`") return false;
    ++it;

    auto prevSibling = context.prevSibling();
    if (prevSibling && prevSibling->type == NodeType::Paragraph) {
      auto paragraph = static_cast<ParagraphNode *>(prevSibling);
      // todo: escape html tags
      paragraph->text += "<code>" + escape(it->value) + "</code>";
    }
    ++it;

    if (it->value != "`") return false;
    ++it;

    return true;
  }

  bool parseBlockQuote(std::vector<Token>::const_iterator &it) {
    if (it->value != "> ") return false;
    ++it;

    auto prevSibling = context.prevSibling();
    if (!prevSibling || prevSibling->type != NodeType::BlockQuote) {
      auto blockquote = new BlockQuoteNode();
      context.append(blockquote);
      context.parent = blockquote;
    } else {
      context.parent = prevSibling;
    }
    return true;
  }

  bool parseOrderedList(std::vector<Token>::const_iterator &it) {
    if (it->kind != TokenKind::Prefix) return false;
    if (it->value != "1.") return false;
    ++it;

    auto prevSibling = context.prevSibling();
    if (!prevSibling || prevSibling->type != NodeType::OrderedList) {
      auto orderedList = new OrderedListNode();
      context.append(orderedList);
      context.parent = orderedList;
    } else {
      context.parent = prevSibling;
    }
    return true;
  }

  bool parseOrderedListItem(std::vector<Token>::const_iterator &it) {
    if (context.parent->type != NodeType::OrderedList) return false;
    auto text = it->value;
    ++it;
    context.append(new OrderedListItemNode(text));
    return true;
  }

  bool parseUnorderedList(std::vector<Token>::const_iterator &it) {
    if (it->kind != TokenKind::Prefix) return false;
    if (it->value != "+ ") return false;
    ++it;

    auto prevSibling = context.prevSibling();
    if (!prevSibling || prevSibling->type != NodeType::UnorderedList) {
      auto unorderedList = new UnorderedListNode();
      context.append(unorderedList);
      context.parent = unorderedList;
    } else {
      context.parent = prevSibling;
    }
    return true;
  }

  bool parseUnorderedListItem(std::vector<Token>::const_iterator &it) {
    if (context.parent->type != NodeType::UnorderedList) return false;
    auto text = it->value;
    ++it;
    context.append(new UnorderedListItemNode(text));
    return true;
  }

  bool parseParagraph(std::vector<Token>::const_iterator &it) {
    auto text = it->value;
    ++it;
    context.append(new ParagraphNode(text));
    return true;
  }

 private:
  ParsingContext context;
};

}  // namespace m2h
