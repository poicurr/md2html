#pragma once

#include <string>
#include <vector>

namespace m2h {
namespace tokenizer {

enum class TokenKind {
  Prefix,  // reserved characters '+ - * # <digit>*.'
  Indent,
  Emphasis,
  Text,
  Horizontal,
  NewLine,
  BackQuote,  // `
};

struct Token {
  explicit Token(TokenKind kind, const std::string& value, const char* location)
      : kind{kind}, value{value}, location{location} {}
  TokenKind kind;
  std::string value;
  const char* location;
};

}  // namespace tokenizer
}  // namespace m2h
