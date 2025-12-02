// SimpleLexer的测试示例
// 测试词法分析器对draw语言的词法分析，旧版本的测试，更全的测试用gtest在test目录下面

#include <iomanip>
#include <iostream>
#include <memory>
#include <string>

// 包含词法分析器相关头文件
#include "HardCodedDFA.hpp"
#include "SimpleLexer.hpp"

using namespace interpreter_exp;
using namespace interpreter_exp::lexer;

// Token类型转字符串
const char *tokenTypeToString(TokenType type) {
  switch (type) {
  case TokenType::Keyword:
    return "Keyword";
  case TokenType::Identifier:
    return "Identifier";
  case TokenType::Literal:
    return "Literal";
  case TokenType::Operator:
    return "Operator";
  case TokenType::Punctuation:
    return "Punctuation";
  case TokenType::Comment:
    return "Comment";
  case TokenType::Eof:
    return "Eof";
  case TokenType::Invalid:
    return "Invalid";
  default:
    return "Unknown";
  }
}

// KeywordType转字符串
const char *keywordTypeToString(KeywordType kw) {
  switch (kw) {
  case KeywordType::If:
    return "If";
  case KeywordType::Else:
    return "Else";
  case KeywordType::While:
    return "While";
  case KeywordType::For:
    return "For";
  case KeywordType::From:
    return "From";
  case KeywordType::To:
    return "To";
  case KeywordType::Step:
    return "Step";
  case KeywordType::Draw:
    return "Draw";
  case KeywordType::T:
    return "T";
  case KeywordType::Return:
    return "Return";
  case KeywordType::Color:
    return "Color";
  case KeywordType::Scale:
    return "Scale";
  case KeywordType::Rot:
    return "Rot";
  case KeywordType::Origin:
    return "Origin";
  case KeywordType::L_bracket:
    return "L_bracket";
  case KeywordType::R_bracket:
    return "R_bracket";
  case KeywordType::Semico:
    return "Semico";
  case KeywordType::Comma:
    return "Comma";
  case KeywordType::Assign:
    return "Assign";
  case KeywordType::Plus:
    return "Plus";
  case KeywordType::Minus:
    return "Minus";
  case KeywordType::Mul:
    return "Mul";
  case KeywordType::Div:
    return "Div";
  case KeywordType::Power:
    return "Power";
  case KeywordType::Func:
    return "Func";
  default:
    return "None";
  }
}

void printToken(const Token &token) {
  std::cout << std::left << std::setw(12) << tokenTypeToString(token.type);
  std::cout << std::setw(15) << std::string(token.lexeme);

  if (token.isKeyword()) {
    std::cout << std::setw(12) << keywordTypeToString(token.keyword());
  } else if (token.isLiteral()) {
    std::cout << std::setw(12) << token.literalValue();
  } else {
    std::cout << std::setw(12) << "-";
  }

  std::cout << "(" << token.sourceLocation.line << ","
            << token.sourceLocation.column << ")";
  std::cout << std::endl;
}

int main() {
  std::cout << "=== SimpleLexer Test Example ===" << std::endl;
  std::cout << std::endl;

  // 测试用例1：基本的draw语句
  const std::string testCase1 = R"(
origin is (100, 200);
scale is (10, 10);
rot is pi / 6;
for t from 0 to 2*pi step pi/100 draw (cos(t), sin(t));
)";

  std::cout << "Test Case 1: Basic draw statements (using TableDrivenDFA)"
            << std::endl;
  std::cout << "Input:" << std::endl;
  std::cout << testCase1 << std::endl;
  std::cout << "Tokens:" << std::endl;
  std::cout << std::left << std::setw(12) << "Type" << std::setw(15) << "Lexeme"
            << std::setw(12) << "Value/Kw"
            << "Location" << std::endl;
  std::cout << std::string(50, '-') << std::endl;

  // 使用新的构造函数API: 传入InputSource和DFA
  auto input1 = std::make_unique<StringInputSource>(testCase1);
  auto dfa1 = std::make_unique<TableDrivenDFA>();
  SimpleLexer lexer1(std::move(input1), std::move(dfa1));

  while (lexer1.hasMoreTokens()) {
    auto token = lexer1.nextToken();
    if (token->type == TokenType::Eof) {
      std::cout << "[EOF]" << std::endl;
      break;
    }
    printToken(*token);
  }

  std::cout << std::endl;

  // 测试用例2：带注释的代码
  const std::string testCase2 = R"(
// This is a comment
origin is (0, 0); -- another comment style
scale is (1, 1);
)";

  std::cout << "Test Case 2: Code with comments (using HardCodedDFA)"
            << std::endl;
  std::cout << "Input:" << std::endl;
  std::cout << testCase2 << std::endl;
  std::cout << "Tokens:" << std::endl;
  std::cout << std::left << std::setw(12) << "Type" << std::setw(15) << "Lexeme"
            << std::setw(12) << "Value/Kw"
            << "Location" << std::endl;
  std::cout << std::string(50, '-') << std::endl;

  // 使用HardCodedDFA
  auto input2 = std::make_unique<StringInputSource>(testCase2);
  auto dfa2 = std::make_unique<HardCodedDFA>();
  SimpleLexer lexer2(std::move(input2), std::move(dfa2));

  while (lexer2.hasMoreTokens()) {
    auto token = lexer2.nextToken();
    if (token->type == TokenType::Eof) {
      std::cout << "[EOF]" << std::endl;
      break;
    }
    printToken(*token);
  }

  std::cout << std::endl;

  // 测试用例3：科学计数法
  const std::string testCase3 = R"(
1.5e10 + 2.3E-5 - 1e+3
)";

  std::cout << "Test Case 3: Scientific notation (using default TableDrivenDFA)"
            << std::endl;
  std::cout << "Input:" << std::endl;
  std::cout << testCase3 << std::endl;
  std::cout << "Tokens:" << std::endl;
  std::cout << std::left << std::setw(12) << "Type" << std::setw(15) << "Lexeme"
            << std::setw(12) << "Value/Kw"
            << "Location" << std::endl;
  std::cout << std::string(50, '-') << std::endl;

  // 使用默认DFA（不传入DFA参数，默认使用TableDrivenDFA）
  auto input3 = std::make_unique<StringInputSource>(testCase3);
  SimpleLexer lexer3(std::move(input3));

  while (lexer3.hasMoreTokens()) {
    auto token = lexer3.nextToken();
    if (token->type == TokenType::Eof) {
      std::cout << "[EOF]" << std::endl;
      break;
    }
    printToken(*token);
  }

  std::cout << std::endl;

  // 测试用例4：所有运算符
  const std::string testCase4 = "1 + 2 - 3 * 4 / 5 ** 6";

  std::cout << "Test Case 4: All operators" << std::endl;
  std::cout << "Input: " << testCase4 << std::endl;
  std::cout << "Tokens:" << std::endl;
  std::cout << std::left << std::setw(12) << "Type" << std::setw(15) << "Lexeme"
            << std::setw(12) << "Value/Kw"
            << "Location" << std::endl;
  std::cout << std::string(50, '-') << std::endl;

  auto input4 = std::make_unique<StringInputSource>(testCase4);
  SimpleLexer lexer4(std::move(input4));

  while (lexer4.hasMoreTokens()) {
    auto token = lexer4.nextToken();
    if (token->type == TokenType::Eof) {
      std::cout << "[EOF]" << std::endl;
      break;
    }
    printToken(*token);
  }

  std::cout << std::endl;
  std::cout << "=== All tests completed ===" << std::endl;

  return 0;
}
