/**
 * @file semantic_test.cc
 * @brief 语义分析器单元测试
 */

#include "DrawLangAST.hpp"
#include "DrawLangParser.hpp"
#include "DrawLangSemantic.hpp"
#include "SimpleLexer.hpp"
#include <cmath>
#include <gtest/gtest.h>
#include <vector>

using namespace interpreter_exp;
using namespace interpreter_exp::lexer;
using namespace interpreter_exp::ast;
using namespace interpreter_exp::parser;
using namespace interpreter_exp::semantic;

// =============================================================================
// 测试夹具类
// =============================================================================

class SemanticTest : public ::testing::Test {
protected:
  std::unique_ptr<DrawLangSemanticAnalyzer> analyzer_;
  std::vector<std::tuple<double, double, PixelAttribute>> drawnPixels_;

  void SetUp() override {
    analyzer_ = std::make_unique<DrawLangSemanticAnalyzer>();
    drawnPixels_.clear();

    // 设置绘制回调，记录所有绘制的像素
    analyzer_->setDrawCallback(
        [this](double x, double y, const PixelAttribute &attr) {
          drawnPixels_.emplace_back(x, y, attr);
        });
  }

  void TearDown() override {
    analyzer_.reset();
    drawnPixels_.clear();
  }

  // 创建语法分析器的辅助函数
  std::unique_ptr<DrawLangParser> createParser(const std::string &source) {
    auto lexer = createLexerFromString(source, DFAType::HardCoded);
    return std::make_unique<DrawLangParser>(lexer.release());
  }

  // 解析并分析源代码
  void parseAndAnalyze(const std::string &source) {
    auto parser = createParser(source);
    // 设置语义分析器使用的parser，这样T变量可以正确更新
    analyzer_->setParser(parser.get());
    auto ast = parser->parse();
    if (ast) {
      analyzer_->run(ast.get());
    }
  }
};

// =============================================================================
// 坐标变换测试
// =============================================================================

TEST_F(SemanticTest, DefaultOrigin) {
  // 默认原点应该是 (0, 0)
  parseAndAnalyze("FOR T FROM 0 TO 0 STEP 1 DRAW(0, 0);");

  ASSERT_EQ(drawnPixels_.size(), 1u);
  EXPECT_DOUBLE_EQ(std::get<0>(drawnPixels_[0]), 0.0);
  EXPECT_DOUBLE_EQ(std::get<1>(drawnPixels_[0]), 0.0);
}

TEST_F(SemanticTest, OriginTranslation) {
  parseAndAnalyze("ORIGIN IS (100, 200);\n"
                  "FOR T FROM 0 TO 0 STEP 1 DRAW(0, 0);");

  ASSERT_EQ(drawnPixels_.size(), 1u);
  EXPECT_DOUBLE_EQ(std::get<0>(drawnPixels_[0]), 100.0);
  EXPECT_DOUBLE_EQ(std::get<1>(drawnPixels_[0]), 200.0);
}

TEST_F(SemanticTest, ScaleTransformation) {
  parseAndAnalyze("SCALE IS (2, 3);\n"
                  "FOR T FROM 0 TO 0 STEP 1 DRAW(10, 10);");

  ASSERT_EQ(drawnPixels_.size(), 1u);
  EXPECT_DOUBLE_EQ(std::get<0>(drawnPixels_[0]), 20.0); // 10 * 2
  EXPECT_DOUBLE_EQ(std::get<1>(drawnPixels_[0]), 30.0); // 10 * 3
}

TEST_F(SemanticTest, RotationTransformation) {
  parseAndAnalyze("ROT IS PI/2;\n" // 90度顺时针旋转
                  "FOR T FROM 0 TO 0 STEP 1 DRAW(1, 0);");

  ASSERT_EQ(drawnPixels_.size(), 1u);
  // 点 (1, 0) 顺时针旋转90度应变成 (0, -1)
  // 顺时针公式: x' = x*cos(θ) + y*sin(θ), y' = y*cos(θ) - x*sin(θ)
  EXPECT_NEAR(std::get<0>(drawnPixels_[0]), 0.0, 1e-10);
  EXPECT_NEAR(std::get<1>(drawnPixels_[0]), -1.0, 1e-10);
}

TEST_F(SemanticTest, CombinedTransformations) {
  parseAndAnalyze("ORIGIN IS (100, 100);\n"
                  "SCALE IS (10, 10);\n"
                  "FOR T FROM 0 TO 0 STEP 1 DRAW(1, 1);");

  ASSERT_EQ(drawnPixels_.size(), 1u);
  // 点 (1, 1) 缩放10倍后平移 -> (110, 110)
  EXPECT_DOUBLE_EQ(std::get<0>(drawnPixels_[0]), 110.0);
  EXPECT_DOUBLE_EQ(std::get<1>(drawnPixels_[0]), 110.0);
}

// =============================================================================
// FOR 循环测试
// =============================================================================

TEST_F(SemanticTest, ForLoopIterations) {
  parseAndAnalyze("FOR T FROM 0 TO 5 STEP 1 DRAW(T, T);");

  // 应该绘制6个点 (0到5，包括边界)
  EXPECT_EQ(drawnPixels_.size(), 6u);
}

TEST_F(SemanticTest, ForLoopWithFractionalStep) {
  parseAndAnalyze("FOR T FROM 0 TO 1 STEP 0.5 DRAW(T, T);");

  // 应该绘制3个点 (0, 0.5, 1)
  EXPECT_EQ(drawnPixels_.size(), 3u);
}

TEST_F(SemanticTest, ForLoopWithPIStep) {
  parseAndAnalyze("FOR T FROM 0 TO PI STEP PI/2 DRAW(T, T);");

  // 应该绘制3个点 (0, PI/2, PI)
  EXPECT_EQ(drawnPixels_.size(), 3u);
}

TEST_F(SemanticTest, ForLoopTVariable) {
  parseAndAnalyze("FOR T FROM 0 TO 2 STEP 1 DRAW(T, T*2);");

  ASSERT_EQ(drawnPixels_.size(), 3u);

  // 验证T变量正确使用
  EXPECT_DOUBLE_EQ(std::get<0>(drawnPixels_[0]), 0.0);
  EXPECT_DOUBLE_EQ(std::get<1>(drawnPixels_[0]), 0.0);

  EXPECT_DOUBLE_EQ(std::get<0>(drawnPixels_[1]), 1.0);
  EXPECT_DOUBLE_EQ(std::get<1>(drawnPixels_[1]), 2.0);

  EXPECT_DOUBLE_EQ(std::get<0>(drawnPixels_[2]), 2.0);
  EXPECT_DOUBLE_EQ(std::get<1>(drawnPixels_[2]), 4.0);
}

TEST_F(SemanticTest, ForLoopTVariableWithTrigFunctions) {
  // 测试sin(T)在循环中的值变化
  parseAndAnalyze("FOR T FROM 0 TO PI STEP PI/2 DRAW(T, sin(T));");

  ASSERT_EQ(drawnPixels_.size(), 3u);

  // T=0: sin(0) = 0
  EXPECT_NEAR(std::get<0>(drawnPixels_[0]), 0.0, 1e-10);
  EXPECT_NEAR(std::get<1>(drawnPixels_[0]), 0.0, 1e-10);

  // T=PI/2: sin(PI/2) = 1
  double piOver2 = 3.1415926535897932 / 2.0;
  EXPECT_NEAR(std::get<0>(drawnPixels_[1]), piOver2, 1e-10);
  EXPECT_NEAR(std::get<1>(drawnPixels_[1]), 1.0, 1e-10);

  // T=PI: sin(PI) ≈ 0
  double pi = 3.1415926535897932;
  EXPECT_NEAR(std::get<0>(drawnPixels_[2]), pi, 1e-10);
  EXPECT_NEAR(std::get<1>(drawnPixels_[2]), 0.0, 1e-10);
}

TEST_F(SemanticTest, ForLoopWithManyPoints) {
  // 测试绘制大量点（类似于实际使用情况）
  parseAndAnalyze("SCALE IS (20, 20);\n"
                  "FOR T FROM 0 TO 2*PI STEP PI/50 DRAW(T, sin(T));");

  // 应该绘制大约 101 个点 (0, PI/50, 2*PI/50, ..., 100*PI/50 = 2*PI)
  // 2*PI / (PI/50) = 100, 加上起点共101个
  EXPECT_GE(drawnPixels_.size(), 100u);
  EXPECT_LE(drawnPixels_.size(), 102u);

  // 验证第一个点和最后一个点
  EXPECT_NEAR(std::get<0>(drawnPixels_[0]), 0.0, 1e-5); // 缩放后x仍为0

  // 验证Y值在合理范围内（经过缩放后）
  for (const auto &pixel : drawnPixels_) {
    double y = std::get<1>(pixel);
    // sin(T) * 20 应该在 [-20, 20] 范围内
    EXPECT_GE(y, -21.0);
    EXPECT_LE(y, 21.0);
  }
}

// =============================================================================
// 颜色设置测试
// =============================================================================

TEST_F(SemanticTest, ColorRGB) {
  parseAndAnalyze("COLOR IS (255, 128, 64);\n"
                  "FOR T FROM 0 TO 0 STEP 1 DRAW(0, 0);");

  ASSERT_EQ(drawnPixels_.size(), 1u);
  const auto &attr = std::get<2>(drawnPixels_[0]);
  EXPECT_EQ(attr.r, 255);
  EXPECT_EQ(attr.g, 128);
  EXPECT_EQ(attr.b, 64);
}

TEST_F(SemanticTest, ColorRed) {
  parseAndAnalyze("COLOR IS RED;\n"
                  "FOR T FROM 0 TO 0 STEP 1 DRAW(0, 0);");

  ASSERT_EQ(drawnPixels_.size(), 1u);
  const auto &attr = std::get<2>(drawnPixels_[0]);
  EXPECT_EQ(attr.r, 255);
  EXPECT_EQ(attr.g, 0);
  EXPECT_EQ(attr.b, 0);
}

TEST_F(SemanticTest, ColorGreen) {
  parseAndAnalyze("COLOR IS GREEN;\n"
                  "FOR T FROM 0 TO 0 STEP 1 DRAW(0, 0);");

  ASSERT_EQ(drawnPixels_.size(), 1u);
  const auto &attr = std::get<2>(drawnPixels_[0]);
  EXPECT_EQ(attr.r, 0);
  EXPECT_EQ(attr.g, 255);
  EXPECT_EQ(attr.b, 0);
}

TEST_F(SemanticTest, ColorBlue) {
  parseAndAnalyze("COLOR IS BLUE;\n"
                  "FOR T FROM 0 TO 0 STEP 1 DRAW(0, 0);");

  ASSERT_EQ(drawnPixels_.size(), 1u);
  const auto &attr = std::get<2>(drawnPixels_[0]);
  EXPECT_EQ(attr.r, 0);
  EXPECT_EQ(attr.g, 0);
  EXPECT_EQ(attr.b, 255);
}

// =============================================================================
// SIZE 设置测试
// =============================================================================

TEST_F(SemanticTest, SizeStatement) {
  parseAndAnalyze("SIZE IS 5;\n"
                  "FOR T FROM 0 TO 0 STEP 1 DRAW(0, 0);");

  ASSERT_EQ(drawnPixels_.size(), 1u);
  const auto &attr = std::get<2>(drawnPixels_[0]);
  EXPECT_EQ(attr.size, 5);
}

TEST_F(SemanticTest, PixsizeStatement) {
  parseAndAnalyze("pixsize is 10;\n"
                  "FOR T FROM 0 TO 0 STEP 1 DRAW(0, 0);");

  ASSERT_EQ(drawnPixels_.size(), 1u);
  const auto &attr = std::get<2>(drawnPixels_[0]);
  EXPECT_EQ(attr.size, 10);
}

// =============================================================================
// 数学函数测试
// =============================================================================

TEST_F(SemanticTest, SinFunction) {
  parseAndAnalyze("FOR T FROM 0 TO 0 STEP 1 DRAW(sin(PI/2), 0);");

  ASSERT_EQ(drawnPixels_.size(), 1u);
  EXPECT_NEAR(std::get<0>(drawnPixels_[0]), 1.0, 1e-10);
}

TEST_F(SemanticTest, CosFunction) {
  parseAndAnalyze("FOR T FROM 0 TO 0 STEP 1 DRAW(cos(0), 0);");

  ASSERT_EQ(drawnPixels_.size(), 1u);
  EXPECT_NEAR(std::get<0>(drawnPixels_[0]), 1.0, 1e-10);
}

TEST_F(SemanticTest, CircleDrawing) {
  parseAndAnalyze("ORIGIN IS (100, 100);\n"
                  "SCALE IS (50, 50);\n"
                  "FOR T FROM 0 TO 2*PI STEP PI/4 DRAW(cos(T), sin(T));");

  // 验证绘制了多个点
  EXPECT_GE(drawnPixels_.size(), 8u);

  // 验证第一个点在圆的最右侧 (cos(0)=1, sin(0)=0) -> (150, 100)
  EXPECT_NEAR(std::get<0>(drawnPixels_[0]), 150.0, 1e-10);
  EXPECT_NEAR(std::get<1>(drawnPixels_[0]), 100.0, 1e-10);
}

// =============================================================================
// 综合测试
// =============================================================================

TEST_F(SemanticTest, CompleteProgram) {
  parseAndAnalyze("pixsize is 2;\n"
                  "ORIGIN IS (400, 300);\n"
                  "SCALE IS (100, 100);\n"
                  "ROT IS 0;\n"
                  "COLOR IS (255, 0, 0);\n"
                  "FOR T FROM 0 TO 2*PI STEP PI/50 DRAW(cos(T), sin(T));");

  // 验证绘制了多个点
  EXPECT_GE(drawnPixels_.size(), 100u);

  // 验证所有点的颜色都是红色
  for (const auto &pixel : drawnPixels_) {
    const auto &attr = std::get<2>(pixel);
    EXPECT_EQ(attr.r, 255);
    EXPECT_EQ(attr.g, 0);
    EXPECT_EQ(attr.b, 0);
    EXPECT_EQ(attr.size, 2);
  }
}

TEST_F(SemanticTest, MultipleForLoops) {
  parseAndAnalyze("COLOR IS RED;\n"
                  "FOR T FROM 0 TO 2 STEP 1 DRAW(T, 0);\n"
                  "COLOR IS BLUE;\n"
                  "FOR T FROM 0 TO 2 STEP 1 DRAW(T, 1);");

  EXPECT_EQ(drawnPixels_.size(), 6u);

  // 前3个点应该是红色
  for (int i = 0; i < 3; ++i) {
    const auto &attr = std::get<2>(drawnPixels_[i]);
    EXPECT_EQ(attr.r, 255);
    EXPECT_EQ(attr.g, 0);
    EXPECT_EQ(attr.b, 0);
  }

  // 后3个点应该是蓝色
  for (int i = 3; i < 6; ++i) {
    const auto &attr = std::get<2>(drawnPixels_[i]);
    EXPECT_EQ(attr.r, 0);
    EXPECT_EQ(attr.g, 0);
    EXPECT_EQ(attr.b, 255);
  }
}

// =============================================================================
// 边界条件测试
// =============================================================================

TEST_F(SemanticTest, EmptyProgram) {
  parseAndAnalyze("");

  // 空程序不应该绘制任何点
  EXPECT_EQ(drawnPixels_.size(), 0u);
}

TEST_F(SemanticTest, OnlySettings) {
  parseAndAnalyze("ORIGIN IS (100, 100);\n"
                  "SCALE IS (2, 2);\n"
                  "COLOR IS RED;");

  // 只有设置语句，没有FOR循环，不应该绘制任何点
  EXPECT_EQ(drawnPixels_.size(), 0u);
}

// =============================================================================
// 主函数
// =============================================================================

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
