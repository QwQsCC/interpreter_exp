/**
 * @file lexer_test.cc
 * @brief 词法分析器单元测试
 */

#include "SimpleLexer.hpp"
#include "Token.hpp"
#include <gtest/gtest.h>


using namespace interpreter_exp;
using namespace interpreter_exp::lexer;

// =============================================================================
// 测试夹具类
// =============================================================================

class LexerTest : public ::testing::Test {
protected:
  void SetUp() override {
    // 每个测试前的初始化
  }

  void TearDown() override {
    // 每个测试后的清理
  }

  // 创建词法分析器的辅助函数
  std::unique_ptr<SimpleLexer> createLexer(const std::string &source) {
    return createLexerFromString(source, DFAType::HardCoded);
  }

  // 获取所有token的辅助函数
  std::vector<Token> getAllTokens(SimpleLexer &lexer) {
    std::vector<Token> tokens;
    while (lexer.hasMoreTokens()) {
      auto token = lexer.nextToken();
      if (token && token->type != TokenType::Eof) {
        tokens.push_back(*token);
      } else {
        break;
      }
    }
    return tokens;
  }
};

// =============================================================================
// 关键字识别测试
// =============================================================================

TEST_F(LexerTest, RecognizeOriginKeyword) {
  auto lexer = createLexer("ORIGIN");
  auto token = lexer->nextToken();

  ASSERT_NE(token, nullptr);
  EXPECT_EQ(token->type, TokenType::Keyword);
  EXPECT_EQ(token->keyword(), KeywordType::Origin);
}

TEST_F(LexerTest, RecognizeScaleKeyword) {
  auto lexer = createLexer("SCALE");
  auto token = lexer->nextToken();

  ASSERT_NE(token, nullptr);
  EXPECT_EQ(token->type, TokenType::Keyword);
  EXPECT_EQ(token->keyword(), KeywordType::Scale);
}

TEST_F(LexerTest, RecognizeRotKeyword) {
  auto lexer = createLexer("ROT");
  auto token = lexer->nextToken();

  ASSERT_NE(token, nullptr);
  EXPECT_EQ(token->type, TokenType::Keyword);
  EXPECT_EQ(token->keyword(), KeywordType::Rot);
}

TEST_F(LexerTest, RecognizeForKeyword) {
  auto lexer = createLexer("FOR");
  auto token = lexer->nextToken();

  ASSERT_NE(token, nullptr);
  EXPECT_EQ(token->type, TokenType::Keyword);
  EXPECT_EQ(token->keyword(), KeywordType::For);
}

TEST_F(LexerTest, RecognizeColorKeyword) {
  auto lexer = createLexer("COLOR");
  auto token = lexer->nextToken();

  ASSERT_NE(token, nullptr);
  EXPECT_EQ(token->type, TokenType::Keyword);
  EXPECT_EQ(token->keyword(), KeywordType::Color);
}

TEST_F(LexerTest, RecognizeSizeKeyword) {
  auto lexer = createLexer("SIZE");
  auto token = lexer->nextToken();

  ASSERT_NE(token, nullptr);
  EXPECT_EQ(token->type, TokenType::Keyword);
  EXPECT_EQ(token->keyword(), KeywordType::Size);
}

TEST_F(LexerTest, RecognizePixsizeKeyword) {
  auto lexer = createLexer("PIXSIZE");
  auto token = lexer->nextToken();

  ASSERT_NE(token, nullptr);
  EXPECT_EQ(token->type, TokenType::Keyword);
  EXPECT_EQ(token->keyword(), KeywordType::Size);
}

TEST_F(LexerTest, RecognizePixelsizeKeyword) {
  auto lexer = createLexer("PIXELSIZE");
  auto token = lexer->nextToken();

  ASSERT_NE(token, nullptr);
  EXPECT_EQ(token->type, TokenType::Keyword);
  EXPECT_EQ(token->keyword(), KeywordType::Size);
}

TEST_F(LexerTest, KeywordsCaseInsensitive) {
  // 测试大小写不敏感
  auto lexer1 = createLexer("origin");
  auto lexer2 = createLexer("Origin");
  auto lexer3 = createLexer("ORIGIN");

  auto token1 = lexer1->nextToken();
  auto token2 = lexer2->nextToken();
  auto token3 = lexer3->nextToken();

  ASSERT_NE(token1, nullptr);
  ASSERT_NE(token2, nullptr);
  ASSERT_NE(token3, nullptr);

  EXPECT_EQ(token1->keyword(), KeywordType::Origin);
  EXPECT_EQ(token2->keyword(), KeywordType::Origin);
  EXPECT_EQ(token3->keyword(), KeywordType::Origin);
}

// =============================================================================
// 常量识别测试
// =============================================================================

TEST_F(LexerTest, RecognizePIConstant) {
  auto lexer = createLexer("PI");
  auto token = lexer->nextToken();

  ASSERT_NE(token, nullptr);
  EXPECT_EQ(token->type, TokenType::Literal);
  EXPECT_EQ(token->lexeme, "PI");
}

TEST_F(LexerTest, RecognizeEConstant) {
  auto lexer = createLexer("E");
  auto token = lexer->nextToken();

  ASSERT_NE(token, nullptr);
  EXPECT_EQ(token->type, TokenType::Literal);
  EXPECT_EQ(token->lexeme, "E");
}

TEST_F(LexerTest, RecognizeIntegerLiteral) {
  auto lexer = createLexer("42");
  auto token = lexer->nextToken();

  ASSERT_NE(token, nullptr);
  EXPECT_EQ(token->type, TokenType::Literal);
  EXPECT_EQ(token->lexeme, "42");
}

TEST_F(LexerTest, RecognizeFloatLiteral) {
  auto lexer = createLexer("3.14");
  auto token = lexer->nextToken();

  ASSERT_NE(token, nullptr);
  EXPECT_EQ(token->type, TokenType::Literal);
  EXPECT_EQ(token->lexeme, "3.14");
}

// =============================================================================
// 运算符识别测试
// =============================================================================

TEST_F(LexerTest, RecognizePlusOperator) {
  auto lexer = createLexer("+");
  auto token = lexer->nextToken();

  ASSERT_NE(token, nullptr);
  EXPECT_EQ(token->type, TokenType::Operator);
  EXPECT_EQ(token->lexeme, "+");
}

TEST_F(LexerTest, RecognizeMinusOperator) {
  auto lexer = createLexer("-");
  auto token = lexer->nextToken();

  ASSERT_NE(token, nullptr);
  EXPECT_EQ(token->type, TokenType::Operator);
  EXPECT_EQ(token->lexeme, "-");
}

TEST_F(LexerTest, RecognizeMulOperator) {
  auto lexer = createLexer("*");
  auto token = lexer->nextToken();

  ASSERT_NE(token, nullptr);
  EXPECT_EQ(token->type, TokenType::Operator);
  EXPECT_EQ(token->lexeme, "*");
}

TEST_F(LexerTest, RecognizeDivOperator) {
  auto lexer = createLexer("/");
  auto token = lexer->nextToken();

  ASSERT_NE(token, nullptr);
  EXPECT_EQ(token->type, TokenType::Operator);
  EXPECT_EQ(token->lexeme, "/");
}

TEST_F(LexerTest, RecognizePowerOperator) {
  auto lexer = createLexer("**");
  auto token = lexer->nextToken();

  ASSERT_NE(token, nullptr);
  EXPECT_EQ(token->type, TokenType::Operator);
  EXPECT_EQ(token->lexeme, "**");
}

// =============================================================================
// 函数识别测试
// =============================================================================

TEST_F(LexerTest, RecognizeSinFunction) {
  auto lexer = createLexer("SIN");
  auto token = lexer->nextToken();

  ASSERT_NE(token, nullptr);
  EXPECT_EQ(token->type, TokenType::Keyword);
  EXPECT_EQ(token->keyword(), KeywordType::Func);
}

TEST_F(LexerTest, RecognizeCosFunction) {
  auto lexer = createLexer("COS");
  auto token = lexer->nextToken();

  ASSERT_NE(token, nullptr);
  EXPECT_EQ(token->type, TokenType::Keyword);
  EXPECT_EQ(token->keyword(), KeywordType::Func);
}

TEST_F(LexerTest, FunctionsCaseInsensitive) {
  auto lexer1 = createLexer("sin");
  auto lexer2 = createLexer("Sin");
  auto lexer3 = createLexer("SIN");

  auto token1 = lexer1->nextToken();
  auto token2 = lexer2->nextToken();
  auto token3 = lexer3->nextToken();

  ASSERT_NE(token1, nullptr);
  ASSERT_NE(token2, nullptr);
  ASSERT_NE(token3, nullptr);

  EXPECT_EQ(token1->keyword(), KeywordType::Func);
  EXPECT_EQ(token2->keyword(), KeywordType::Func);
  EXPECT_EQ(token3->keyword(), KeywordType::Func);
}

// =============================================================================
// 复合语句测试
// =============================================================================

TEST_F(LexerTest, TokenizeOriginStatement) {
  auto lexer = createLexer("ORIGIN IS (100, 200);");
  auto tokens = getAllTokens(*lexer);

  ASSERT_GE(tokens.size(), 7u);
  EXPECT_EQ(tokens[0].keyword(), KeywordType::Origin);
  EXPECT_EQ(tokens[1].lexeme, "IS"); // IS 或 ==
}

TEST_F(LexerTest, TokenizeForDrawStatement) {
  auto lexer = createLexer("FOR T FROM 0 TO 2*PI STEP PI/50 DRAW(T, sin(T));");
  auto tokens = getAllTokens(*lexer);

  ASSERT_GE(tokens.size(), 10u);
  EXPECT_EQ(tokens[0].keyword(), KeywordType::For);
  EXPECT_EQ(tokens[1].keyword(), KeywordType::T);
  EXPECT_EQ(tokens[2].keyword(), KeywordType::From);
}

TEST_F(LexerTest, TokenizeSizeStatement) {
  auto lexer = createLexer("SIZE IS 5;");
  auto tokens = getAllTokens(*lexer);

  ASSERT_GE(tokens.size(), 3u);
  EXPECT_EQ(tokens[0].keyword(), KeywordType::Size);
}

TEST_F(LexerTest, TokenizePixsizeStatement) {
  auto lexer = createLexer("pixsize is 5;");
  auto tokens = getAllTokens(*lexer);

  ASSERT_GE(tokens.size(), 3u);
  EXPECT_EQ(tokens[0].keyword(), KeywordType::Size);
}

// =============================================================================
// 注释处理测试
// =============================================================================

TEST_F(LexerTest, SkipLineComment) {
  auto lexer = createLexer("// this is a comment\nORIGIN");
  auto token = lexer->nextToken();

  ASSERT_NE(token, nullptr);
  EXPECT_EQ(token->type, TokenType::Keyword);
  EXPECT_EQ(token->keyword(), KeywordType::Origin);
}

TEST_F(LexerTest, SkipDashComment) {
  auto lexer = createLexer("-- this is a comment\nORIGIN");
  auto token = lexer->nextToken();

  ASSERT_NE(token, nullptr);
  EXPECT_EQ(token->type, TokenType::Keyword);
  EXPECT_EQ(token->keyword(), KeywordType::Origin);
}

// =============================================================================
// 错误处理测试
// =============================================================================

TEST_F(LexerTest, HandleUnknownCharacter) {
  auto lexer = createLexer("@");
  auto token = lexer->nextToken();

  // 应该产生一个无效token或者跳过
  ASSERT_NE(token, nullptr);
  EXPECT_TRUE(token->type == TokenType::Invalid ||
              token->type == TokenType::Eof);
}

// =============================================================================
// 源位置测试
// =============================================================================

TEST_F(LexerTest, TrackSourceLocation) {
  auto lexer = createLexer("ORIGIN\nSCALE");

  auto token1 = lexer->nextToken();
  ASSERT_NE(token1, nullptr);
  EXPECT_EQ(token1->sourceLocation.line, 1u);

  auto token2 = lexer->nextToken();
  ASSERT_NE(token2, nullptr);
  EXPECT_EQ(token2->sourceLocation.line, 2u);
}

// =============================================================================
// 主函数
// =============================================================================

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
