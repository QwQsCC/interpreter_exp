// Draw语言语义分析器的声明
// 实现语义计算和绘图操作

#pragma once

#include "DrawLangAST.hpp"
#include "DrawLangParser.hpp"
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace interpreter_exp {
namespace semantic {

// 像素属性
struct PixelAttribute {
  unsigned char r = 255;
  unsigned char g = 0;
  unsigned char b = 0;
  double size = 1.0;

  void setColor(unsigned char red, unsigned char green, unsigned char blue) {
    r = red;
    g = green;
    b = blue;
  }

  void setColor(double red, double green, double blue) {
    r = static_cast<unsigned char>(std::clamp(red, 0.0, 255.0));
    g = static_cast<unsigned char>(std::clamp(green, 0.0, 255.0));
    b = static_cast<unsigned char>(std::clamp(blue, 0.0, 255.0));
  }

  void setSize(double s) { size = s > 0 ? s : 1.0; }
};

// 绘图回调函数类型
using DrawPixelCallback =
    std::function<void(double x, double y, const PixelAttribute &attr)>;

// 语义分析配置
struct SemanticConfig {
  bool enableDebugOutput = true; // 调试输出（默认开启以方便调试）
  bool enableDemoMode = false;   // 是否启用演示模式（Zorro）
};

// Draw语言语义分析器
class DrawLangSemanticAnalyzer {
public:
  // 构造函数
  explicit DrawLangSemanticAnalyzer(parser::DrawLangParser *parser = nullptr);
  ~DrawLangSemanticAnalyzer();

  // 设置Parser
  void setParser(parser::DrawLangParser *parser);

  // 语义分析入口
  // 遍历AST并执行语义动作（绘图）
  int run(ast::ProgramNode *program);

  // 设置绘图回调
  void setDrawCallback(DrawPixelCallback callback) {
    drawCallback_ = std::move(callback);
  }

  // 获取/设置绘图参数
  double getOriginX() const { return originX_; }
  double getOriginY() const { return originY_; }
  double getScaleX() const { return scaleX_; }
  double getScaleY() const { return scaleY_; }
  double getRotAngle() const { return rotAngle_; }
  const PixelAttribute &getPixelAttribute() const { return attr_; }

  void setOrigin(double x, double y) {
    originX_ = x;
    originY_ = y;
  }
  void setScale(double sx, double sy) {
    scaleX_ = sx;
    scaleY_ = sy;
  }
  void setRotation(double angle) { rotAngle_ = angle; }

  // 配置
  void setConfig(const SemanticConfig &config) { config_ = config; }
  const SemanticConfig &getConfig() const { return config_; }

private:
  // 语句处理
  void executeStatement(ast::StatementNode *stmt);
  void executeOriginStmt(ast::OriginStmtNode *stmt);
  void executeScaleStmt(ast::ScaleStmtNode *stmt);
  void executeRotStmt(ast::RotStmtNode *stmt);
  void executeForDrawStmt(ast::ForDrawStmtNode *stmt);
  void executeColorStmt(ast::ColorStmtNode *stmt);
  void executeSizeStmt(ast::SizeStmtNode *stmt);

  // 坐标计算
  void calcCoord(ast::ExpressionNode *xTree, ast::ExpressionNode *yTree,
                 double *ptrX, double *ptrY);

  // 绘制循环
  void drawLoop(ast::ExpressionNode *startTree, ast::ExpressionNode *endTree,
                ast::ExpressionNode *stepTree, ast::ExpressionNode *xTree,
                ast::ExpressionNode *yTree);

  // 绘制单个像素
  void drawPixel(double x, double y);

  // 演示模式：绘制Zorro图案
  void executeZorroDemo(ast::ProgramNode *program);

private:
  parser::DrawLangParser *parser_;

  // 绘图参数
  double originX_ = 0.0;
  double originY_ = 0.0;
  double scaleX_ = 1.0;
  double scaleY_ = 1.0;
  double rotAngle_ = 0.0;

  // T值存储
  double tStorage_ = 0.0;

  // 像素属性
  PixelAttribute attr_;

  // 绘图回调
  DrawPixelCallback drawCallback_;

  // 配置
  SemanticConfig config_;
};

// 完整的解释器封装
class DrawLangInterpreter {
public:
  DrawLangInterpreter();
  ~DrawLangInterpreter();

  // 从字符串执行
  bool executeFromString(const std::string &source,
                         const std::string &name = "string");

  // 从文件执行
  bool executeFromFile(const std::string &filename);

  // 设置绘图回调
  void setDrawCallback(DrawPixelCallback callback);

  // 获取语义分析器（用于访问绘图参数等）
  DrawLangSemanticAnalyzer *getSemanticAnalyzer() {
    return semanticAnalyzer_.get();
  }

  // 获取错误信息
  bool hasErrors() const;
  std::vector<std::string> getErrors() const;

private:
  std::unique_ptr<lexer::SimpleLexer> lexer_;
  std::unique_ptr<parser::DrawLangParser> parser_;
  std::unique_ptr<DrawLangSemanticAnalyzer> semanticAnalyzer_;
  std::unique_ptr<ast::ProgramNode> ast_;

  std::vector<std::string> errors_;
};

} // namespace semantic
} // namespace interpreter_exp
