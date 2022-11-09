#include <fstream>
#include <iostream>
#include <string>

#include "m2h/parser/Parser.hpp"
#include "m2h/tokenizer/Tokenizer.hpp"

int main(int argc, char const* argv[]) {
  if (argc != 2) {
    std::cerr << "usage: ./md2html /path/to/markdown.md" << std::endl;
    return 1;
  }

  std::ifstream ifs(argv[1]);
  if (!ifs) {
    std::cerr << "failed to open: '" << argv[1] << "'" << std::endl;
    return 1;
  }

  std::string s;
  char buf[1024];
  while (!ifs.eof()) {
    ifs.read(buf, 1024);
    s.append(buf, ifs.gcount());
  }

  const char* p = s.c_str();

  std::cout << "[info] start tokenizing" << std::endl;
  m2h::Tokenizer tokenizer;
  std::vector<m2h::Token> tokens = tokenizer.tokenize(p);
  for (auto token : tokens) {
    switch (token.kind) {
      case m2h::TokenKind::Prefix:
        std::cout << "<Prefix '" << token.value << "'>";
        break;
      case m2h::TokenKind::BackQuote:
        std::cout << "<Backquote>";
        break;
      case m2h::TokenKind::Indent:
        std::cout << "<Indent '" << token.value << "'>";
        break;
      case m2h::TokenKind::Text:
        std::cout << "<Text '" << token.value << "'>";
        break;
      case m2h::TokenKind::NewLine:
        std::cout << "<NewLine>" << std::endl;
        break;
    }
  }

  m2h::Parser parser;
  std::vector<m2h::Node*> nodes = parser.parse(tokens);

  std::cout << "[info] generated html" << std::endl;
  for (auto&& node : nodes) {
    node->print("");
  }
}
