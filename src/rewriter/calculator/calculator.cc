// Copyright 2010-2021, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "rewriter/calculator/calculator_interface.h"

// Following include directives are used in parser.c included by anonymous
// namespace. We need to do this outside the namespace, otherwise features
// declared in those header files are included by the namespace as well, which
// causes build error.
#include <assert.h>
#include <stdio.h>
#ifdef _WIN32
#include <float.h>
#endif  // _WIN32
#include <string.h>

#include <cmath>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "base/japanese_util.h"
#include "base/logging.h"
#include "base/number_util.h"
#include "base/singleton.h"
#include "absl/strings/str_format.h"
#include "absl/strings/string_view.h"

// parser.c is a source file auto-generated by Lemon Parser.  It is not compiled
// directly and is included as a part of calculator.cc.  It defines global
// symbols with the same names as symbols defined in other files, so we put the
// whole thing in an anonymous namespace.
namespace {

bool IsFinite(double x) { return std::isfinite(x); }

#include "rewriter/calculator/parser.c.inc"
}  // namespace

namespace mozc {
namespace {

class CalculatorImpl : public CalculatorInterface {
 public:
  CalculatorImpl();

  bool CalculateString(const std::string &key,
                       std::string *result) const override;

 private:
  typedef std::vector<std::pair<int, double> > TokenSequence;

  // Max byte length of operator character
  static constexpr size_t kMaxLengthOfOperator = 3;
  static constexpr size_t kBufferSizeOfOutputNumber = 32;

  // Tokenizes |expression_body| and sets the tokens into |tokens|.
  // It returns false if |expression_body| includes an invalid token or
  // does not include both of a number token and an operator token.
  // Parenthesis is not considered as an operator.
  bool Tokenize(absl::string_view expression_body, TokenSequence *tokens) const;

  // Perform calculation with a given sequence of token.
  bool CalculateTokens(const TokenSequence &tokens, double *result_value) const;

  // Mapping from operator character such as '+' to the corresponding
  // token type such as PLUS.
  std::map<std::string, int> operator_map_;
};

CalculatorImpl::CalculatorImpl() {
  operator_map_["+"] = PLUS;
  operator_map_["-"] = MINUS;
  // "ー". It is called cho-ompu, onbiki, bobiki, or "nobashi-bou" casually.
  // It is not a full-width hyphen, and may appear in conversion segments by
  // typing '-' more than one time continuouslly.
  operator_map_["ー"] = MINUS;
  operator_map_["*"] = TIMES;
  operator_map_["/"] = DIVIDE;
  operator_map_["・"] = DIVIDE;  // "･". Consider it as "/".
  operator_map_["%"] = MOD;
  operator_map_["^"] = POW;
  operator_map_["("] = LP;
  operator_map_[")"] = RP;
}

// Basic arithmetic operations are available.
// TODO(tok): Add more number of operators.
bool CalculatorImpl::CalculateString(const std::string &key,
                                     std::string *result) const {
  DCHECK(result);
  if (key.empty()) {
    LOG(ERROR) << "Key is empty.";
    return false;
  }
  std::string normalized_key;
  japanese_util::FullWidthAsciiToHalfWidthAscii(key, &normalized_key);

  absl::string_view expression_body;
  if (normalized_key.front() == '=') {
    // Expression starts with '='.
    expression_body =
        absl::string_view(normalized_key.data() + 1, normalized_key.size() - 1);
  } else if (normalized_key.back() == '=') {
    // Expression is ended with '='.
    expression_body =
        absl::string_view(normalized_key.data(), normalized_key.size() - 1);
  } else {
    // Expression does not start nor end with '='.
    result->clear();
    return false;
  }

  TokenSequence tokens;
  if (!Tokenize(expression_body, &tokens)) {
    // normalized_key is not valid sequence of tokens
    result->clear();
    return false;
  }

  double result_value = 0.0;
  if (!CalculateTokens(tokens, &result_value)) {
    // Calculation is failed. Syntax error or arithmetic error such as
    // overflow, divide-by-zero, etc.
    result->clear();
    return false;
  }
  char buffer[kBufferSizeOfOutputNumber];
  absl::SNPrintF(buffer, sizeof(buffer), "%.8g", result_value);
  *result = buffer;
  return true;
}

bool CalculatorImpl::Tokenize(absl::string_view expression_body,
                              TokenSequence *tokens) const {
  const char *current = expression_body.data();
  const char *end = expression_body.data() + expression_body.size();
  int num_operator = 0;  // Number of operators appeared
  int num_value = 0;     // Number of values appeared

  DCHECK(tokens);
  tokens->clear();

  while (current < end) {
    // Skip spaces
    while ((*current == ' ') || (*current == '\t')) {
      ++current;
    }
    const char *token_begin = current;

    // Read value token
    while (((*current >= '0') && (*current <= '9')) || (*current == '.')) {
      ++current;
    }
    if (token_begin < current) {
      std::string number_token(token_begin, current - token_begin);
      double value = 0.0;
      if (!NumberUtil::SafeStrToDouble(number_token, &value)) {
        return false;
      }
      tokens->push_back(std::make_pair(INTEGER, value));
      ++num_value;
      continue;
    }

    // Read operator token
    for (size_t length = 1; length <= kMaxLengthOfOperator; ++length) {
      if (current + length > end) {
        // Invalid token
        return false;
      }
      std::string window(current, length);
      std::map<std::string, int>::const_iterator op_it =
          operator_map_.find(window);
      if (op_it == operator_map_.end()) {
        continue;
      }
      tokens->push_back(std::make_pair(op_it->second, 0.0));
      current += length;
      // Does not count parenthesis as an operator.
      if ((op_it->second != LP) && (op_it->second != RP)) {
        ++num_operator;
      }
      break;
    }
    if (token_begin < current) {
      continue;
    }

    // Invalid token
    return false;
  }

  if (num_operator == 0 || num_value == 0) {
    // Must contain at least one operator and one value.
    return false;
  }
  return true;
}

bool CalculatorImpl::CalculateTokens(const TokenSequence &tokens,
                                     double *result_value) const {
  DCHECK(result_value);
  void *parser = ParseAlloc(malloc);
  Result result;
  for (size_t i = 0; i < tokens.size(); ++i) {
    Parse(parser, tokens[i].first, tokens[i].second, &result);
  }
  Parse(parser, 0, 0.0, &result);
  ParseFree(parser, free);

  if (result.error_type != Result::ACCEPTED) {
    return false;
  }
  *result_value = result.value;
  return true;
}

CalculatorInterface *g_calculator = nullptr;
}  // namespace

CalculatorInterface *CalculatorFactory::GetCalculator() {
  if (g_calculator == nullptr) {
    return Singleton<CalculatorImpl>::get();
  } else {
    return g_calculator;
  }
}

void CalculatorFactory::SetCalculator(CalculatorInterface *calculator) {
  g_calculator = calculator;
}
}  // namespace mozc
