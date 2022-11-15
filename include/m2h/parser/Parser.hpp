#pragma once

#include <iostream>
#include <list>
#include <string>

#include "../ParsingUtility.hpp"
#include "../tokenizer/Token.hpp"
#include "Node.hpp"
#include "ParsingContext.hpp"

namespace m2h {

using token_iterator = std::vector<Token>::iterator;

class Parser {
 public:
  Parser() {}

  std::vector<Node *> parse(std::vector<Token> &tokens) {
    Node *root = new RootNode();
    context.parent = root;
    context.index = 0;

    token_iterator it = tokens.begin();
    while (it != tokens.end()) {
      auto bak = it;
      if (parseHeading(it)) {
        goto next;
      } else {
        it = bak;
      }

      if (parseHorizontal(it)) {
        goto next;
      } else {
        it = bak;
      }

      if (parseBlockQuote(it)) {
        goto next;
      } else {
        it = bak;
      }

      if (parseOrderedList(it)) {
        goto next;
      } else {
        it = bak;
      }

      if (parseUnorderedList(root, it)) {
        goto next;
      } else {
        it = bak;
      }

      if (parseCodeBlock1(it)) {
        goto next;
      } else {
        it = bak;
      }

      if (parseCodeBlock2(it)) {
        goto next;
      } else {
        it = bak;
      }

      if (parseInlineCode(it)) {
        goto next;
      } else {
        it = bak;
      }

      if (parseEmphasis(it)) {
        goto next;
      } else {
        it = bak;
      }

      if (parseNewline(root, it)) {
        goto next;
      } else {
        it = bak;
      }

      // fallback
      parseParagraph(it);

    next:
      ++it;
    }
    return root->children;
  }

 private:
  bool parseParagraph(token_iterator &it) {
    while (it->kind == TokenKind::Indent) {
      context.index += it->value.size();
      ++it;
    }
    auto prevSibling = context.prevSibling();
    if (prevSibling && prevSibling->type == NodeType::Paragraph) {
      auto paragraph = static_cast<ParagraphNode *>(prevSibling);
      if (paragraph->index == context.index) {
        paragraph->text += "\n" + it->value;
        return true;
      }
    }
    context.append(new ParagraphNode(context.index, it->value));
    return true;
  }

  bool parseHeading(token_iterator &it) {
    if (it->kind != TokenKind::Prefix) return false;
    int level = 0;
    for (char c : it->value) {
      if (c != '#') break;
      ++level;
    }
    if (level == 0) return false;
    ++it;
    if (it->kind != TokenKind::Text) return false;
    const std::string value = it->value;
    context.append(new HeadingNode(level, value));
    return true;
  }

  bool parseHorizontal(token_iterator &it) {
    if (it->kind != TokenKind::Horizontal) return false;
    context.append(new HorizontalNode());
    return true;
  }

  bool parseNewline(Node *root, token_iterator &it) {
    if (it->kind != TokenKind::NewLine) return false;
    auto prevToken = it - 1;
    if (prevToken->value == "> ") {
      context.append(new EmptyLineNode());
    }
    if (prevToken->kind == TokenKind::NewLine) {
      context.append(new EmptyLineNode());
    }
    context.parent = root;
    context.index = 0;
    return true;
  }

  bool parseInlineCode(token_iterator &it) {
    if (it->value != "`") return false;
    ++it;
    auto prevSibling = context.prevSibling();
    if (prevSibling && prevSibling->type == NodeType::Paragraph) {
      auto paragraph = static_cast<ParagraphNode *>(prevSibling);
      paragraph->text += "<code>" + escape(it->value) + "</code>";
    }
    ++it;
    if (it->value != "`") return false;
    return true;
  }

  bool parseEmphasis(token_iterator &it) {
    if (it->kind != TokenKind::Emphasis) return false;
    ++it;
    auto prevSibling = context.prevSibling();
    if (prevSibling && prevSibling->type == NodeType::Paragraph) {
      auto paragraph = static_cast<ParagraphNode *>(prevSibling);
      paragraph->text += "<em>" + it->value + "</em>";
    }
    ++it;
    if (it->kind != TokenKind::Emphasis) return false;
    return true;
  }

  bool parseBlockQuote(token_iterator &it) {
    if (it->value != "> ") return false;
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

  bool parseCodeBlock1(token_iterator &it) {
    if (it->kind != TokenKind::Indent) return false;
    ++it;

    auto code = std::string{};
    while (it->kind != TokenKind::NewLine) {
      code += it->value;
      ++it;
    }

    auto prevSibling = context.prevSibling();
    if (prevSibling && prevSibling->type == NodeType::CodeBlock) {
      auto codeblock = static_cast<CodeBlockNode *>(prevSibling);
      codeblock->text += "\n" + escape(code);
    } else {
      context.append(new CodeBlockNode(escape(code)));
    }

    return true;
  }

  bool parseCodeBlock2(token_iterator &it) {
    for (int i = 0; i < 3; ++i) {
      if (it->kind != TokenKind::BackQuote) return false;
      ++it;
    }

    auto code = std::string{};
    while (it->kind != TokenKind::BackQuote) {
      if (it->kind == TokenKind::NewLine) code += "\n";
      code += it->value;
      ++it;
    }

    for (int i = 0; i < 3; ++i) {
      if (it->kind != TokenKind::BackQuote) return false;
      ++it;
    }

    context.append(new CodeBlockNode(escape(code)));
    return true;
  }

  bool parseUnorderedList(Node *root, token_iterator &it) {
    while (it->kind == TokenKind::Indent) {
      context.index += it->value.size();
      ++it;
    }

    if (it->value != "+ ") return false;
    context.index += it->value.size();
    ++it;

    // ul
    auto prevSibling = context.prevSibling();
    if (prevSibling && prevSibling->type == NodeType::UnorderedList) {
      context.parent = prevSibling;
      auto prevlist = static_cast<UnorderedListNode *>(prevSibling);
      if (prevlist->index < context.index) {
        auto unorderedlist = new UnorderedListNode(context.index);
        context.append(unorderedlist);
        context.parent = unorderedlist;
      }
    } else {
      auto unorderedlist = new UnorderedListNode(context.index);
      context.append(unorderedlist);
      context.parent = unorderedlist;
    }

    // li
    while (it->kind == TokenKind::Indent) {
      context.index += it->value.size();
      ++it;
    }
    context.append(new UnorderedListItemNode(it->value));

    return true;
  }

  bool parseOrderedList(token_iterator &it) {
    while (it->kind == TokenKind::Indent) {
      context.index += it->value.size();
      ++it;
    }
    if (it->kind != TokenKind::Prefix) return false;
    if (it->value[0] != '1') return false;
    context.index += it->value.size();
    ++it;

    while (it->kind == TokenKind::Indent) {
      context.index += it->value.size();
      ++it;
    }

    // ol
    auto prevSibling = context.prevSibling();
    if (prevSibling && prevSibling->type == NodeType::OrderedList) {
      context.parent = prevSibling;
    } else {
      auto orderedlist = new OrderedListNode(context.index);
      context.append(orderedlist);
      context.parent = orderedlist;
    }

    // li
    while (it->kind == TokenKind::Indent) {
      context.index += it->value.size();
      ++it;
    }
    context.append(new OrderedListItemNode(it->value));

    return true;
  }

 private:
  ParsingContext context;
};

}  // namespace m2h
