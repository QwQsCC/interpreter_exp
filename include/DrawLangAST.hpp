// Draw语言的AST节点定义

#pragma once

#include "Token.hpp"
#include <cmath>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace interpreter_exp {
namespace ast {

// 前向声明
class ASTVisitor;
// AST节点类型
enum class DrawASTNodeType {
  // 程序结构
  Program, // 程序根节点（语句列表）

  // 语句类型
  OriginStmt,  // origin is (x, y);
  ScaleStmt,   // scale is (sx, sy);
  RotStmt,     // rot is angle;
  ForDrawStmt, // for t from start to end step s draw (x, y);
  ColorStmt,   // color is (r, g, b); 或 color is NAME;
  SizeStmt,    // size is s; 或 size is (w, h);

  // 表达式类型
  BinaryExpr,    // 二元表达式: +, -, *, /, **
  UnaryExpr,     // 一元表达式: +, -
  FuncCallExpr,  // 函数调用: sin(x), cos(x)等
  ConstExpr,     // 常量: 数字、PI、E等
  ParamExpr,     // 参数T
  ColorNameExpr, // 颜色名称

  // 其他
  ErrorNode
};
// 位置信息
struct ASTLocation {
  SourceLocation start;
  SourceLocation end;

  ASTLocation() = default;
  ASTLocation(SourceLocation s) : start(s), end(s) {}
  ASTLocation(SourceLocation s, SourceLocation e) : start(s), end(e) {}

  std::string toString() const {
    return "[" + std::to_string(start.line) + ":" +
           std::to_string(start.column) + "]";
  }
};
// 函数指针类型（用于内置函数）
using MathFunc = double (*)(double);
// AST节点基类
class DrawASTNode {
public:
  virtual ~DrawASTNode() = default;

  // 获取节点类型
  virtual DrawASTNodeType getNodeType() const = 0;

  // 获取位置信息
  virtual ASTLocation getLocation() const = 0;

  // 计算表达式值（对于表达式节点）
  virtual double value() const { return 0.0; }

  // 获取子节点
  virtual DrawASTNode *getChild(size_t index) const { return nullptr; }
  virtual size_t getChildCount() const { return 0; }
  virtual double childValue(size_t index) const {
    auto child = getChild(index);
    return child ? child->value() : 0.0;
  }

  // 获取Token信息
  virtual const Token &getToken() const = 0;

  // 打印AST树
  virtual void print(int indent = 0) const = 0;

  // 转字符串
  virtual std::string toString() const = 0;

protected:
  DrawASTNode() = default;
};
// 表达式节点基类
class ExpressionNode : public DrawASTNode {
public:
  ExpressionNode(const Token &token, const ASTLocation &loc)
      : token_(token), location_(loc) {}

  const Token &getToken() const override { return token_; }
  ASTLocation getLocation() const override { return location_; }

protected:
  Token token_;
  ASTLocation location_;
  std::vector<std::unique_ptr<ExpressionNode>> children_;
};
// 二元表达式节点
class BinaryExprNode : public ExpressionNode {
public:
  BinaryExprNode(const Token &op, std::unique_ptr<ExpressionNode> left,
                 std::unique_ptr<ExpressionNode> right)
      : ExpressionNode(op, ASTLocation(op.sourceLocation)),
        left_(std::move(left)), right_(std::move(right)) {}

  DrawASTNodeType getNodeType() const override {
    return DrawASTNodeType::BinaryExpr;
  }

  double value() const override;

  DrawASTNode *getChild(size_t index) const override {
    if (index == 0)
      return left_.get();
    if (index == 1)
      return right_.get();
    return nullptr;
  }
  size_t getChildCount() const override { return 2; }

  double leftValue() const { return left_ ? left_->value() : 0.0; }
  double rightValue() const { return right_ ? right_->value() : 0.0; }

  ExpressionNode *getLeft() const { return left_.get(); }
  ExpressionNode *getRight() const { return right_.get(); }

  void print(int indent = 0) const override;
  std::string toString() const override;

private:
  std::unique_ptr<ExpressionNode> left_;
  std::unique_ptr<ExpressionNode> right_;
};
// 一元表达式节点
class UnaryExprNode : public ExpressionNode {
public:
  UnaryExprNode(const Token &op, std::unique_ptr<ExpressionNode> operand)
      : ExpressionNode(op, ASTLocation(op.sourceLocation)),
        operand_(std::move(operand)) {}

  DrawASTNodeType getNodeType() const override {
    return DrawASTNodeType::UnaryExpr;
  }

  double value() const override {
    double v = operand_ ? operand_->value() : 0.0;
    if (token_.keyword() == KeywordType::Minus) {
      return -v;
    }
    return v; // Plus
  }

  DrawASTNode *getChild(size_t index) const override {
    return index == 0 ? operand_.get() : nullptr;
  }
  size_t getChildCount() const override { return 1; }

  void print(int indent = 0) const override;
  std::string toString() const override;

private:
  std::unique_ptr<ExpressionNode> operand_;
};
// 函数调用表达式节点
class FuncCallExprNode : public ExpressionNode {
public:
  FuncCallExprNode(const Token &funcToken,
                   std::unique_ptr<ExpressionNode> argument,
                   MathFunc funcPtr = nullptr)
      : ExpressionNode(funcToken, ASTLocation(funcToken.sourceLocation)),
        argument_(std::move(argument)), funcPtr_(funcPtr) {}

  DrawASTNodeType getNodeType() const override {
    return DrawASTNodeType::FuncCallExpr;
  }

  double value() const override {
    if (funcPtr_ && argument_) {
      return funcPtr_(argument_->value());
    }
    return 0.0;
  }

  void setFuncPtr(MathFunc ptr) { funcPtr_ = ptr; }
  MathFunc getFuncPtr() const { return funcPtr_; }

  DrawASTNode *getChild(size_t index) const override {
    return index == 0 ? argument_.get() : nullptr;
  }
  size_t getChildCount() const override { return 1; }

  void print(int indent = 0) const override;
  std::string toString() const override;

private:
  std::unique_ptr<ExpressionNode> argument_;
  MathFunc funcPtr_;
};
// 常量表达式节点
class ConstExprNode : public ExpressionNode {
public:
  ConstExprNode(const Token &token, double val = 0.0)
      : ExpressionNode(token, ASTLocation(token.sourceLocation)),
        constValue_(val) {
    // 如果传入了非零值，直接使用
    // 只有当传入的val为默认值0时才尝试解析字面量
    if (val == 0.0 && token.isLiteral()) {
      std::string litValue = token.literalValue();
      // 只尝试解析数字字符串
      if (!litValue.empty() && (std::isdigit(litValue[0]) ||
                                litValue[0] == '-' || litValue[0] == '.')) {
        try {
          constValue_ = std::stod(litValue);
        } catch (...) {
          constValue_ = 0.0;
        }
      }
    }
  }

  DrawASTNodeType getNodeType() const override {
    return DrawASTNodeType::ConstExpr;
  }

  double value() const override { return constValue_; }

  void setValue(double v) { constValue_ = v; }

  void print(int indent = 0) const override;
  std::string toString() const override;

private:
  double constValue_;
};
// 参数T表达式节点
class ParamExprNode : public ExpressionNode {
public:
  ParamExprNode(const Token &token, double *storage = nullptr)
      : ExpressionNode(token, ASTLocation(token.sourceLocation)),
        storage_(storage) {}

  DrawASTNodeType getNodeType() const override {
    return DrawASTNodeType::ParamExpr;
  }

  double value() const override { return storage_ ? *storage_ : 0.0; }

  void setStorage(double *ptr) { storage_ = ptr; }
  double *getStorage() const { return storage_; }

  void print(int indent = 0) const override;
  std::string toString() const override;

private:
  double *storage_;
};
// 颜色名称表达式节点
class ColorNameExprNode : public ExpressionNode {
public:
  ColorNameExprNode(const Token &token)
      : ExpressionNode(token, ASTLocation(token.sourceLocation)) {}

  DrawASTNodeType getNodeType() const override {
    return DrawASTNodeType::ColorNameExpr;
  }

  double value() const override { return 0.0; }

  // 获取RGB值
  void getRGB(double &r, double &g, double &b) const;

  std::string getColorName() const { return token_.lexeme; }

  void print(int indent = 0) const override;
  std::string toString() const override;
};
// 语句节点基类
class StatementNode : public DrawASTNode {
public:
  StatementNode(const Token &token, const ASTLocation &loc)
      : token_(token), location_(loc) {}

  const Token &getToken() const override { return token_; }
  ASTLocation getLocation() const override { return location_; }

  // 添加表达式子节点
  void addExpression(std::unique_ptr<ExpressionNode> expr) {
    expressions_.push_back(std::move(expr));
  }

  // 获取表达式子节点
  ExpressionNode *getExpression(size_t index) const {
    return index < expressions_.size() ? expressions_[index].get() : nullptr;
  }

  DrawASTNode *getChild(size_t index) const override {
    return getExpression(index);
  }

  size_t getChildCount() const override { return expressions_.size(); }

protected:
  Token token_;
  ASTLocation location_;
  std::vector<std::unique_ptr<ExpressionNode>> expressions_;
};
// Origin语句节点
class OriginStmtNode : public StatementNode {
public:
  OriginStmtNode(const Token &token)
      : StatementNode(token, ASTLocation(token.sourceLocation)) {}

  DrawASTNodeType getNodeType() const override {
    return DrawASTNodeType::OriginStmt;
  }

  // 获取x和y坐标表达式
  ExpressionNode *getXExpr() const { return getExpression(0); }
  ExpressionNode *getYExpr() const { return getExpression(1); }

  double getX() const { return childValue(0); }
  double getY() const { return childValue(1); }

  void print(int indent = 0) const override;
  std::string toString() const override;
};
// Scale语句节点
class ScaleStmtNode : public StatementNode {
public:
  ScaleStmtNode(const Token &token)
      : StatementNode(token, ASTLocation(token.sourceLocation)) {}

  DrawASTNodeType getNodeType() const override {
    return DrawASTNodeType::ScaleStmt;
  }

  double getScaleX() const { return childValue(0); }
  double getScaleY() const { return childValue(1); }

  void print(int indent = 0) const override;
  std::string toString() const override;
};
// Rot语句节点
class RotStmtNode : public StatementNode {
public:
  RotStmtNode(const Token &token)
      : StatementNode(token, ASTLocation(token.sourceLocation)) {}

  DrawASTNodeType getNodeType() const override {
    return DrawASTNodeType::RotStmt;
  }

  double getAngle() const { return childValue(0); }

  void print(int indent = 0) const override;
  std::string toString() const override;
};
// For-Draw语句节点
class ForDrawStmtNode : public StatementNode {
public:
  ForDrawStmtNode(const Token &token)
      : StatementNode(token, ASTLocation(token.sourceLocation)) {}

  DrawASTNodeType getNodeType() const override {
    return DrawASTNodeType::ForDrawStmt;
  }

  // 获取各部分表达式
  // expressions_[0]: start (from)
  // expressions_[1]: end (to)
  // expressions_[2]: step
  // expressions_[3]: x坐标
  // expressions_[4]: y坐标
  ExpressionNode *getStartExpr() const { return getExpression(0); }
  ExpressionNode *getEndExpr() const { return getExpression(1); }
  ExpressionNode *getStepExpr() const { return getExpression(2); }
  ExpressionNode *getXExpr() const { return getExpression(3); }
  ExpressionNode *getYExpr() const { return getExpression(4); }

  double getStart() const { return childValue(0); }
  double getEnd() const { return childValue(1); }
  double getStep() const { return childValue(2); }

  void print(int indent = 0) const override;
  std::string toString() const override;
};
// Color语句节点
class ColorStmtNode : public StatementNode {
public:
  ColorStmtNode(const Token &token)
      : StatementNode(token, ASTLocation(token.sourceLocation)),
        useColorName_(false) {}

  DrawASTNodeType getNodeType() const override {
    return DrawASTNodeType::ColorStmt;
  }

  // 设置使用颜色名称
  void setColorName(std::unique_ptr<ColorNameExprNode> colorName) {
    colorName_ = std::move(colorName);
    useColorName_ = true;
  }

  bool usesColorName() const { return useColorName_; }

  ColorNameExprNode *getColorName() const { return colorName_.get(); }

  // RGB模式
  double getRed() const { return childValue(0); }
  double getGreen() const { return childValue(1); }
  double getBlue() const { return childValue(2); }

  // 获取默认颜色
  static void getDefaultColor(double &r, double &g, double &b) {
    r = 255;
    g = 0;
    b = 0; // 默认红色
  }

  void print(int indent = 0) const override;
  std::string toString() const override;

private:
  bool useColorName_;
  std::unique_ptr<ColorNameExprNode> colorName_;
};
// Size语句节点
class SizeStmtNode : public StatementNode {
public:
  SizeStmtNode(const Token &token)
      : StatementNode(token, ASTLocation(token.sourceLocation)) {}

  DrawASTNodeType getNodeType() const override {
    return DrawASTNodeType::SizeStmt;
  }

  // 获取像素大小
  double getSize() const { return childValue(0); }

  // 可选的第二个维度
  bool hasTwoDimensions() const { return getChildCount() == 2; }
  double getWidth() const { return childValue(0); }
  double getHeight() const { return childValue(1); }

  void print(int indent = 0) const override;
  std::string toString() const override;
};
// 程序节点（语句列表）
class ProgramNode : public DrawASTNode {
public:
  ProgramNode(const Token &token, const std::string &filename = "")
      : token_(token), filename_(filename),
        location_(ASTLocation(token.sourceLocation)) {}

  DrawASTNodeType getNodeType() const override {
    return DrawASTNodeType::Program;
  }
  const Token &getToken() const override { return token_; }
  ASTLocation getLocation() const override { return location_; }

  // 添加语句
  void addStatement(std::unique_ptr<StatementNode> stmt, bool atEnd = true) {
    if (atEnd) {
      statements_.push_back(std::move(stmt));
    } else {
      statements_.insert(statements_.begin(), std::move(stmt));
    }
  }

  // 获取语句
  StatementNode *getStatement(size_t index) const {
    return index < statements_.size() ? statements_[index].get() : nullptr;
  }

  DrawASTNode *getChild(size_t index) const override {
    return getStatement(index);
  }

  size_t getChildCount() const override { return statements_.size(); }

  const std::string &getFilename() const { return filename_; }

  void print(int indent = 0) const override;
  std::string toString() const override;

private:
  Token token_;
  std::string filename_;
  ASTLocation location_;
  std::vector<std::unique_ptr<StatementNode>> statements_;
};
// AST工具类
class DrawASTUtils {
public:
  // 打印AST树
  static void dump(const DrawASTNode *root, int indent = 0);

  // 创建缩进字符串
  static std::string makeIndent(int level);
};
// 颜色名称映射
class ColorMap {
public:
  static ColorMap &getInstance();

  bool isDefined(const std::string &name) const;
  bool getRGB(const std::string &name, double &r, double &g, double &b) const;
  void addColor(const std::string &name, double r, double g, double b);

private:
  ColorMap();

  struct RGB {
    double r, g, b;
  };
  std::unordered_map<std::string, RGB> colors_;
};

} // namespace ast
} // namespace interpreter_exp
