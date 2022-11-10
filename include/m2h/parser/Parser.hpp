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
      if (it->kind == TokenKind::BackQuote) {
        // InlineCode
        auto bak = it;
        if (parseInlineCode(it)) continue;
        it = bak;
      }

      if (it->kind == TokenKind::Horizontal) {
        auto bak = it;
        if (parseHorizontal(it)) continue;
        it = bak;
      }

      if (it->kind == TokenKind::Prefix) {
        // BlockQuote/CodeBlock/InlineCode/ListItem/Heading
        auto bak = it;
        if (parseHeading(it)) continue;
        if (parseBlockQuote(it)) continue;
        if (parseOrderedList(it)) continue;
        if (parseUnorderedList(it)) continue;
        it = bak;
      }

      if (it->kind == TokenKind::Text) {
        // InlineCode/Emphasis/ListItem/Paragraph
        auto bak = it;
        if (parseOrderedListItem(it)) continue;
        if (parseUnorderedListItem(it)) continue;
        if (parseParagraph(it)) continue;
        it = bak;
      }

      if (it->kind == TokenKind::Indent) {
        // CodeBlock
        auto bak = it;
        if (parseCodeBlock(it)) continue;
        it = bak;
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

  bool parseHorizontal(std::vector<Token>::const_iterator &it) {
    if (it->kind != TokenKind::Horizontal) return false;
    ++it;
    context.append(new HorizontalNode());
    return true;
  }

  bool parseCodeBlock(std::vector<Token>::const_iterator &it) {
    if (it->kind != TokenKind::Indent) return false;
    auto ws = it->value;
    ++it;
    if (ws.size() < 4) return false;
    auto prevSibling = context.prevSibling();
    if (prevSibling && prevSibling->type == NodeType::CodeBlock) {
      auto codeblock = static_cast<CodeBlockNode *>(prevSibling);
      codeblock->lines.push_back(escape(it->value));
      context.parent = codeblock;
    } else {
      auto codeblock = new CodeBlockNode(escape(it->value));
      context.append(codeblock);
      context.parent = codeblock;
    }
    ++it;
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
    if (prevSibling && prevSibling->type == NodeType::BlockQuote) {
      context.parent = prevSibling;
    } else {
      auto blockquote = new BlockQuoteNode();
      context.append(blockquote);
      context.parent = blockquote;
    }
    return true;
  }

  bool parseOrderedList(std::vector<Token>::const_iterator &it) {
    if (it->kind != TokenKind::Prefix) return false;
    if (it->value != "1.") return false;
    ++it;

    auto prevSibling = context.prevSibling();
    if (prevSibling && prevSibling->type == NodeType::OrderedList) {
      context.parent = prevSibling;
    } else {
      auto orderedList = new OrderedListNode();
      context.append(orderedList);
      context.parent = orderedList;
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
    if (prevSibling && prevSibling->type == NodeType::UnorderedList) {
      context.parent = prevSibling;
    } else {
      auto unorderedList = new UnorderedListNode();
      context.append(unorderedList);
      context.parent = unorderedList;
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
