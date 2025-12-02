/**
 * @file parser_test.cc
 * @brief 语法分析器单元测试
 */

#include "DrawLangAST.hpp"
#include "DrawLangParser.hpp"
#include "SimpleLexer.hpp"
#include <gtest/gtest.h>

using namespace interpreter_exp;
using namespace interpreter_exp::lexer;
using namespace interpreter_exp::ast;
using namespace interpreter_exp::parser;

// =============================================================================
// 测试夹具类
// =============================================================================

class ParserTest : public ::testing::Test {
protected:
  void SetUp() override {
    // 每个测试前的初始化
  }

  void TearDown() override {
    // 每个测试后的清理
  }

  // 创建语法分析器的辅助函数
  std::unique_ptr<DrawLangParser> createParser(const std::string &source) {
    auto lexer = createLexerFromString(source, DFAType::HardCoded);
    return std::make_unique<DrawLangParser>(lexer.release());
  }
};

// =============================================================================
// ORIGIN 语句测试
// =============================================================================

TEST_F(ParserTest, ParseOriginStatement) {
  auto parser = createParser("ORIGIN IS (100, 200);");
  auto ast = parser->parse();

  ASSERT_NE(ast, nullptr);
  ASSERT_GT(ast->getChildCount(), 0u);

  auto *stmt = ast->getChild(0);
  ASSERT_NE(stmt, nullptr);
  EXPECT_EQ(stmt->getNodeType(), DrawASTNodeType::OriginStmt);
}

TEST_F(ParserTest, ParseOriginWithExpression) {
  auto parser = createParser("ORIGIN IS (100+50, 200*2);");
  auto ast = parser->parse();

  ASSERT_NE(ast, nullptr);
  ASSERT_GT(ast->getChildCount(), 0u);

  auto *stmt = dynamic_cast<OriginStmtNode *>(ast->getChild(0));
  ASSERT_NE(stmt, nullptr);

  // 验证表达式存在
  EXPECT_NE(stmt->getXExpr(), nullptr);
  EXPECT_NE(stmt->getYExpr(), nullptr);
}

// =============================================================================
// SCALE 语句测试
// =============================================================================

TEST_F(ParserTest, ParseScaleStatement) {
  auto parser = createParser("SCALE IS (2, 3);");
  auto ast = parser->parse();

  ASSERT_NE(ast, nullptr);
  ASSERT_GT(ast->getChildCount(), 0u);

  auto *stmt = ast->getChild(0);
  ASSERT_NE(stmt, nullptr);
  EXPECT_EQ(stmt->getNodeType(), DrawASTNodeType::ScaleStmt);
}

// =============================================================================
// ROT 语句测试
// =============================================================================

TEST_F(ParserTest, ParseRotStatement) {
  auto parser = createParser("ROT IS PI/2;");
  auto ast = parser->parse();

  ASSERT_NE(ast, nullptr);
  ASSERT_GT(ast->getChildCount(), 0u);

  auto *stmt = ast->getChild(0);
  ASSERT_NE(stmt, nullptr);
  EXPECT_EQ(stmt->getNodeType(), DrawASTNodeType::RotStmt);
}

// =============================================================================
// FOR DRAW 语句测试
// =============================================================================

TEST_F(ParserTest, ParseForDrawStatement) {
  auto parser =
      createParser("FOR T FROM 0 TO 2*PI STEP PI/50 DRAW(cos(T), sin(T));");
  auto ast = parser->parse();

  ASSERT_NE(ast, nullptr);
  ASSERT_GT(ast->getChildCount(), 0u);

  auto *stmt = ast->getChild(0);
  ASSERT_NE(stmt, nullptr);
  EXPECT_EQ(stmt->getNodeType(), DrawASTNodeType::ForDrawStmt);
}

TEST_F(ParserTest, ForStatementHasExpressions) {
  auto parser = createParser("FOR T FROM 0 TO 10 STEP 1 DRAW(T, T);");
  auto ast = parser->parse();

  ASSERT_NE(ast, nullptr);
  ASSERT_GT(ast->getChildCount(), 0u);

  auto *stmt = dynamic_cast<ForDrawStmtNode *>(ast->getChild(0));
  ASSERT_NE(stmt, nullptr);

  // 验证所有表达式存在
  EXPECT_NE(stmt->getStartExpr(), nullptr);
  EXPECT_NE(stmt->getEndExpr(), nullptr);
  EXPECT_NE(stmt->getStepExpr(), nullptr);
  EXPECT_NE(stmt->getXExpr(), nullptr);
  EXPECT_NE(stmt->getYExpr(), nullptr);
}

// =============================================================================
// COLOR 语句测试
// =============================================================================

TEST_F(ParserTest, ParseColorRGBStatement) {
  auto parser = createParser("COLOR IS (255, 128, 64);");
  auto ast = parser->parse();

  ASSERT_NE(ast, nullptr);
  ASSERT_GT(ast->getChildCount(), 0u);

  auto *stmt = ast->getChild(0);
  ASSERT_NE(stmt, nullptr);
  EXPECT_EQ(stmt->getNodeType(), DrawASTNodeType::ColorStmt);
}

TEST_F(ParserTest, ParseColorNameStatement) {
  auto parser = createParser("COLOR IS RED;");
  auto ast = parser->parse();

  ASSERT_NE(ast, nullptr);
  ASSERT_GT(ast->getChildCount(), 0u);

  auto *stmt = ast->getChild(0);
  ASSERT_NE(stmt, nullptr);
  EXPECT_EQ(stmt->getNodeType(), DrawASTNodeType::ColorStmt);
}

// =============================================================================
// SIZE 语句测试
// =============================================================================

TEST_F(ParserTest, ParseSizeStatement) {
  auto parser = createParser("SIZE IS 5;");
  auto ast = parser->parse();

  ASSERT_NE(ast, nullptr);
  ASSERT_GT(ast->getChildCount(), 0u);

  auto *stmt = ast->getChild(0);
  ASSERT_NE(stmt, nullptr);
  EXPECT_EQ(stmt->getNodeType(), DrawASTNodeType::SizeStmt);
}

TEST_F(ParserTest, ParsePixsizeStatement) {
  auto parser = createParser("pixsize is 5;");
  auto ast = parser->parse();

  ASSERT_NE(ast, nullptr);
  ASSERT_GT(ast->getChildCount(), 0u);

  auto *stmt = ast->getChild(0);
  ASSERT_NE(stmt, nullptr);
  EXPECT_EQ(stmt->getNodeType(), DrawASTNodeType::SizeStmt);
}

TEST_F(ParserTest, ParsePixelsizeStatement) {
  auto parser = createParser("PIXELSIZE IS 10;");
  auto ast = parser->parse();

  ASSERT_NE(ast, nullptr);
  ASSERT_GT(ast->getChildCount(), 0u);

  auto *stmt = ast->getChild(0);
  ASSERT_NE(stmt, nullptr);
  EXPECT_EQ(stmt->getNodeType(), DrawASTNodeType::SizeStmt);
}

// =============================================================================
// 表达式测试
// =============================================================================

TEST_F(ParserTest, ParseBinaryExpression) {
  auto parser = createParser("ORIGIN IS (1+2, 3*4);");
  auto ast = parser->parse();

  ASSERT_NE(ast, nullptr);
  auto *stmt = dynamic_cast<OriginStmtNode *>(ast->getChild(0));
  ASSERT_NE(stmt, nullptr);

  auto *xExpr = stmt->getXExpr();
  ASSERT_NE(xExpr, nullptr);
  EXPECT_EQ(xExpr->getNodeType(), DrawASTNodeType::BinaryExpr);
}

TEST_F(ParserTest, ParseUnaryExpression) {
  auto parser = createParser("ORIGIN IS (-100, 200);");
  auto ast = parser->parse();

  ASSERT_NE(ast, nullptr);
  auto *stmt = dynamic_cast<OriginStmtNode *>(ast->getChild(0));
  ASSERT_NE(stmt, nullptr);

  auto *xExpr = stmt->getXExpr();
  ASSERT_NE(xExpr, nullptr);
  EXPECT_EQ(xExpr->getNodeType(), DrawASTNodeType::UnaryExpr);
}

TEST_F(ParserTest, ParseFunctionCallExpression) {
  auto parser = createParser("ORIGIN IS (sin(PI), cos(0));");
  auto ast = parser->parse();

  ASSERT_NE(ast, nullptr);
  auto *stmt = dynamic_cast<OriginStmtNode *>(ast->getChild(0));
  ASSERT_NE(stmt, nullptr);

  auto *xExpr = stmt->getXExpr();
  ASSERT_NE(xExpr, nullptr);
  EXPECT_EQ(xExpr->getNodeType(), DrawASTNodeType::FuncCallExpr);
}

TEST_F(ParserTest, ParseConstantExpression) {
  auto parser = createParser("ROT IS PI;");
  auto ast = parser->parse();

  ASSERT_NE(ast, nullptr);
  auto *stmt = dynamic_cast<RotStmtNode *>(ast->getChild(0));
  ASSERT_NE(stmt, nullptr);

  // 验证 PI 的值正确
  double value = stmt->getAngle();
  EXPECT_NEAR(value, 3.1415926535897932, 1e-10);
}

// =============================================================================
// 表达式求值测试
// =============================================================================

TEST_F(ParserTest, EvaluateAdditionExpression) {
  auto parser = createParser("ORIGIN IS (1+2, 3);");
  auto ast = parser->parse();

  auto *stmt = dynamic_cast<OriginStmtNode *>(ast->getChild(0));
  ASSERT_NE(stmt, nullptr);

  double xValue = stmt->getXExpr()->value();
  EXPECT_DOUBLE_EQ(xValue, 3.0);
}

TEST_F(ParserTest, EvaluateMultiplicationExpression) {
  auto parser = createParser("ORIGIN IS (2*3, 0);");
  auto ast = parser->parse();

  auto *stmt = dynamic_cast<OriginStmtNode *>(ast->getChild(0));
  ASSERT_NE(stmt, nullptr);

  double xValue = stmt->getXExpr()->value();
  EXPECT_DOUBLE_EQ(xValue, 6.0);
}

TEST_F(ParserTest, EvaluateDivisionExpression) {
  auto parser = createParser("ORIGIN IS (10/2, 0);");
  auto ast = parser->parse();

  auto *stmt = dynamic_cast<OriginStmtNode *>(ast->getChild(0));
  ASSERT_NE(stmt, nullptr);

  double xValue = stmt->getXExpr()->value();
  EXPECT_DOUBLE_EQ(xValue, 5.0);
}

TEST_F(ParserTest, EvaluatePowerExpression) {
  auto parser = createParser("ORIGIN IS (2**3, 0);");
  auto ast = parser->parse();

  auto *stmt = dynamic_cast<OriginStmtNode *>(ast->getChild(0));
  ASSERT_NE(stmt, nullptr);

  double xValue = stmt->getXExpr()->value();
  EXPECT_DOUBLE_EQ(xValue, 8.0);
}

TEST_F(ParserTest, EvaluatePIDivisionExpression) {
  // 这个测试验证 PI/50 能正确求值（之前的bug）
  auto parser = createParser("FOR T FROM 0 TO PI STEP PI/50 DRAW(T, T);");
  auto ast = parser->parse();

  auto *stmt = dynamic_cast<ForDrawStmtNode *>(ast->getChild(0));
  ASSERT_NE(stmt, nullptr);

  double stepValue = stmt->getStepExpr()->value();
  double expectedValue = 3.1415926535897932 / 50.0;
  EXPECT_NEAR(stepValue, expectedValue, 1e-10);
  EXPECT_GT(stepValue, 0.0); // 确保步长不为零
}

// =============================================================================
// 多语句测试
// =============================================================================

TEST_F(ParserTest, ParseMultipleStatements) {
  auto parser =
      createParser("ORIGIN IS (100, 100);\n"
                   "SCALE IS (2, 2);\n"
                   "ROT IS PI/4;\n"
                   "FOR T FROM 0 TO 2*PI STEP PI/50 DRAW(cos(T), sin(T));");
  auto ast = parser->parse();

  ASSERT_NE(ast, nullptr);
  EXPECT_EQ(ast->getChildCount(), 4u);

  EXPECT_EQ(ast->getChild(0)->getNodeType(), DrawASTNodeType::OriginStmt);
  EXPECT_EQ(ast->getChild(1)->getNodeType(), DrawASTNodeType::ScaleStmt);
  EXPECT_EQ(ast->getChild(2)->getNodeType(), DrawASTNodeType::RotStmt);
  EXPECT_EQ(ast->getChild(3)->getNodeType(), DrawASTNodeType::ForDrawStmt);
}

// =============================================================================
// FOR语句详细测试
// =============================================================================

TEST_F(ParserTest, ForStatementExpressionValues) {
  auto parser = createParser("FOR T FROM 0 TO 10 STEP 1 DRAW(T, T*2);");
  auto ast = parser->parse();

  ASSERT_NE(ast, nullptr);
  ASSERT_GT(ast->getChildCount(), 0u);

  auto *stmt = dynamic_cast<ForDrawStmtNode *>(ast->getChild(0));
  ASSERT_NE(stmt, nullptr);

  // 验证起始值、结束值、步长
  EXPECT_DOUBLE_EQ(stmt->getStart(), 0.0);
  EXPECT_DOUBLE_EQ(stmt->getEnd(), 10.0);
  EXPECT_DOUBLE_EQ(stmt->getStep(), 1.0);
}

TEST_F(ParserTest, ForStatementWithPIExpressions) {
  auto parser =
      createParser("FOR T FROM 0 TO 2*PI STEP PI/50 DRAW(T, sin(T));");
  auto ast = parser->parse();

  ASSERT_NE(ast, nullptr);
  ASSERT_GT(ast->getChildCount(), 0u);

  auto *stmt = dynamic_cast<ForDrawStmtNode *>(ast->getChild(0));
  ASSERT_NE(stmt, nullptr);

  // 验证起始值
  EXPECT_DOUBLE_EQ(stmt->getStart(), 0.0);

  // 验证结束值 (2*PI)
  double expectedEnd = 2 * 3.1415926535897932;
  EXPECT_NEAR(stmt->getEnd(), expectedEnd, 1e-10);

  // 验证步长 (PI/50)
  double expectedStep = 3.1415926535897932 / 50.0;
  EXPECT_NEAR(stmt->getStep(), expectedStep, 1e-10);
  EXPECT_GT(stmt->getStep(), 0.0); // 确保步长不为零
}

TEST_F(ParserTest, ForStatementDrawExpressions) {
  auto parser = createParser("FOR T FROM 0 TO 5 STEP 1 DRAW(T+1, T*2+3);");
  auto ast = parser->parse();

  ASSERT_NE(ast, nullptr);
  auto *stmt = dynamic_cast<ForDrawStmtNode *>(ast->getChild(0));
  ASSERT_NE(stmt, nullptr);

  // 验证X和Y表达式存在
  ASSERT_NE(stmt->getXExpr(), nullptr);
  ASSERT_NE(stmt->getYExpr(), nullptr);

  // 验证表达式类型（应该是二元表达式）
  EXPECT_EQ(stmt->getXExpr()->getNodeType(), DrawASTNodeType::BinaryExpr);
  EXPECT_EQ(stmt->getYExpr()->getNodeType(), DrawASTNodeType::BinaryExpr);
}

TEST_F(ParserTest, ForStatementWithTrigFunctions) {
  auto parser =
      createParser("FOR T FROM 0 TO PI STEP 0.1 DRAW(cos(T), sin(T));");
  auto ast = parser->parse();

  ASSERT_NE(ast, nullptr);
  auto *stmt = dynamic_cast<ForDrawStmtNode *>(ast->getChild(0));
  ASSERT_NE(stmt, nullptr);

  // 验证X和Y表达式是函数调用
  ASSERT_NE(stmt->getXExpr(), nullptr);
  ASSERT_NE(stmt->getYExpr(), nullptr);
  EXPECT_EQ(stmt->getXExpr()->getNodeType(), DrawASTNodeType::FuncCallExpr);
  EXPECT_EQ(stmt->getYExpr()->getNodeType(), DrawASTNodeType::FuncCallExpr);
}

// =============================================================================
// 错误处理测试
// =============================================================================

TEST_F(ParserTest, HandleMissingSemicolon) {
  auto parser = createParser("ORIGIN IS (100, 200)");
  auto ast = parser->parse();

  // 即使缺少分号也应该尝试解析
  // 这取决于实现，可能产生部分AST或错误
  // 此测试验证不会崩溃
  SUCCEED();
}

TEST_F(ParserTest, HandleEmptyInput) {
  auto parser = createParser("");
  auto ast = parser->parse();

  // 空输入应该产生空的程序节点
  ASSERT_NE(ast, nullptr);
  EXPECT_EQ(ast->getChildCount(), 0u);
}

// =============================================================================
// 主函数
// =============================================================================

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
