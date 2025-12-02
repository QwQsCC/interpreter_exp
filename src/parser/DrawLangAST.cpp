// Draw语言专用AST节点的实现

#include "DrawLangAST.hpp"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <iostream>
#include <sstream>

namespace interpreter_exp {
namespace ast {

double BinaryExprNode::value() const {
  double l = leftValue();
  double r = rightValue();

  KeywordType op = token_.keyword();
  switch (op) {
  case KeywordType::Plus:
    return l + r;
  case KeywordType::Minus:
    return l - r;
  case KeywordType::Mul:
    return l * r;
  case KeywordType::Div:
    return (r != 0.0) ? l / r : 0.0;
  case KeywordType::Power:
    return std::pow(l, r);
  default:
    return 0.0;
  }
}

void BinaryExprNode::print(int indent) const {
  std::cout << DrawASTUtils::makeIndent(indent) << token_.lexeme << std::endl;
  if (left_)
    left_->print(indent + 2);
  if (right_)
    right_->print(indent + 2);
}

std::string BinaryExprNode::toString() const {
  return "(" + (left_ ? left_->toString() : "") + " " + token_.lexeme + " " +
         (right_ ? right_->toString() : "") + ")";
}

void UnaryExprNode::print(int indent) const {
  std::cout << DrawASTUtils::makeIndent(indent) << token_.lexeme << std::endl;
  if (operand_)
    operand_->print(indent + 2);
}

std::string UnaryExprNode::toString() const {
  return token_.lexeme + (operand_ ? operand_->toString() : "");
}

void FuncCallExprNode::print(int indent) const {
  std::cout << DrawASTUtils::makeIndent(indent) << token_.lexeme << "()"
            << std::endl;
  if (argument_)
    argument_->print(indent + 2);
}

std::string FuncCallExprNode::toString() const {
  return token_.lexeme + "(" + (argument_ ? argument_->toString() : "") + ")";
}

void ConstExprNode::print(int indent) const {
  std::cout << DrawASTUtils::makeIndent(indent) << constValue_ << std::endl;
}

std::string ConstExprNode::toString() const {
  std::ostringstream oss;
  oss << constValue_;
  return oss.str();
}

void ParamExprNode::print(int indent) const {
  std::cout << DrawASTUtils::makeIndent(indent) << "T" << std::endl;
}

std::string ParamExprNode::toString() const { return "T"; }

void ColorNameExprNode::getRGB(double &r, double &g, double &b) const {
  std::string name = token_.lexeme;
  // 转大写
  std::transform(name.begin(), name.end(), name.begin(), ::toupper);

  if (!ColorMap::getInstance().getRGB(name, r, g, b)) {
    // 默认红色
    r = 255;
    g = 0;
    b = 0;
  }
}

void ColorNameExprNode::print(int indent) const {
  std::cout << DrawASTUtils::makeIndent(indent) << "COLOR:" << token_.lexeme
            << std::endl;
}

std::string ColorNameExprNode::toString() const { return token_.lexeme; }

void OriginStmtNode::print(int indent) const {
  std::cout << DrawASTUtils::makeIndent(indent) << "ORIGIN" << std::endl;
  for (size_t i = 0; i < expressions_.size(); ++i) {
    if (expressions_[i])
      expressions_[i]->print(indent + 2);
  }
}

std::string OriginStmtNode::toString() const {
  return "origin is (" + (getXExpr() ? getXExpr()->toString() : "") + ", " +
         (getYExpr() ? getYExpr()->toString() : "") + ")";
}

void ScaleStmtNode::print(int indent) const {
  std::cout << DrawASTUtils::makeIndent(indent) << "SCALE" << std::endl;
  for (size_t i = 0; i < expressions_.size(); ++i) {
    if (expressions_[i])
      expressions_[i]->print(indent + 2);
  }
}

std::string ScaleStmtNode::toString() const {
  return "scale is (" + std::to_string(getScaleX()) + ", " +
         std::to_string(getScaleY()) + ")";
}

void RotStmtNode::print(int indent) const {
  std::cout << DrawASTUtils::makeIndent(indent) << "ROT" << std::endl;
  if (expressions_.size() > 0 && expressions_[0]) {
    expressions_[0]->print(indent + 2);
  }
}

std::string RotStmtNode::toString() const {
  return "rot is " + std::to_string(getAngle());
}

void ForDrawStmtNode::print(int indent) const {
  std::cout << DrawASTUtils::makeIndent(indent) << "FOR-DRAW" << std::endl;
  std::cout << DrawASTUtils::makeIndent(indent + 2) << "[start]" << std::endl;
  if (getStartExpr())
    getStartExpr()->print(indent + 4);
  std::cout << DrawASTUtils::makeIndent(indent + 2) << "[end]" << std::endl;
  if (getEndExpr())
    getEndExpr()->print(indent + 4);
  std::cout << DrawASTUtils::makeIndent(indent + 2) << "[step]" << std::endl;
  if (getStepExpr())
    getStepExpr()->print(indent + 4);
  std::cout << DrawASTUtils::makeIndent(indent + 2) << "[x]" << std::endl;
  if (getXExpr())
    getXExpr()->print(indent + 4);
  std::cout << DrawASTUtils::makeIndent(indent + 2) << "[y]" << std::endl;
  if (getYExpr())
    getYExpr()->print(indent + 4);
}

std::string ForDrawStmtNode::toString() const {
  return "for t from " + (getStartExpr() ? getStartExpr()->toString() : "") +
         " to " + (getEndExpr() ? getEndExpr()->toString() : "") + " step " +
         (getStepExpr() ? getStepExpr()->toString() : "") + " draw (" +
         (getXExpr() ? getXExpr()->toString() : "") + ", " +
         (getYExpr() ? getYExpr()->toString() : "") + ")";
}

void ColorStmtNode::print(int indent) const {
  std::cout << DrawASTUtils::makeIndent(indent) << "COLOR" << std::endl;
  if (useColorName_ && colorName_) {
    colorName_->print(indent + 2);
  } else {
    for (size_t i = 0; i < expressions_.size(); ++i) {
      if (expressions_[i])
        expressions_[i]->print(indent + 2);
    }
  }
}

std::string ColorStmtNode::toString() const {
  if (useColorName_ && colorName_) {
    return "color is " + colorName_->toString();
  }
  return "color is (" + std::to_string(static_cast<int>(getRed())) + ", " +
         std::to_string(static_cast<int>(getGreen())) + ", " +
         std::to_string(static_cast<int>(getBlue())) + ")";
}

void SizeStmtNode::print(int indent) const {
  std::cout << DrawASTUtils::makeIndent(indent) << "SIZE" << std::endl;
  for (size_t i = 0; i < expressions_.size(); ++i) {
    if (expressions_[i])
      expressions_[i]->print(indent + 2);
  }
}

std::string SizeStmtNode::toString() const {
  if (hasTwoDimensions()) {
    return "size is (" + std::to_string(getWidth()) + ", " +
           std::to_string(getHeight()) + ")";
  }
  return "size is " + std::to_string(getSize());
}

void ProgramNode::print(int indent) const {
  std::cout << DrawASTUtils::makeIndent(indent) << "PROGRAM: " << filename_
            << std::endl;
  for (size_t i = 0; i < statements_.size(); ++i) {
    if (statements_[i])
      statements_[i]->print(indent + 2);
  }
}

std::string ProgramNode::toString() const {
  std::ostringstream oss;
  oss << "Program: " << filename_ << "\n";
  for (size_t i = 0; i < statements_.size(); ++i) {
    if (statements_[i]) {
      oss << "  " << statements_[i]->toString() << ";\n";
    }
  }
  return oss.str();
}

void DrawASTUtils::dump(const DrawASTNode *root, int indent) {
  if (root) {
    root->print(indent);
  }
}

std::string DrawASTUtils::makeIndent(int level) {
  return std::string(level, ' ');
}

ColorMap &ColorMap::getInstance() {
  static ColorMap instance;
  return instance;
}

ColorMap::ColorMap() {
  // 初始化预定义颜色
  addColor("RED", 255, 0, 0);
  addColor("GREEN", 0, 255, 0);
  addColor("BLUE", 0, 0, 255);
  addColor("BLACK", 0, 0, 0);
  addColor("WHITE", 255, 255, 255);
  addColor("YELLOW", 255, 255, 0);
  addColor("CYAN", 0, 255, 255);
  addColor("MAGENTA", 255, 0, 255);
  addColor("GRAY", 128, 128, 128);
  addColor("GREY", 128, 128, 128);
  addColor("ORANGE", 255, 165, 0);
  addColor("PINK", 255, 192, 203);
  addColor("PURPLE", 128, 0, 128);
  addColor("BROWN", 139, 69, 19);
}

bool ColorMap::isDefined(const std::string &name) const {
  std::string upper = name;
  std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
  return colors_.find(upper) != colors_.end();
}

bool ColorMap::getRGB(const std::string &name, double &r, double &g,
                      double &b) const {
  std::string upper = name;
  std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);

  auto it = colors_.find(upper);
  if (it != colors_.end()) {
    r = it->second.r;
    g = it->second.g;
    b = it->second.b;
    return true;
  }
  return false;
}

void ColorMap::addColor(const std::string &name, double r, double g, double b) {
  std::string upper = name;
  std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
  colors_[upper] = {r, g, b};
}

} // namespace ast
} // namespace interpreter_exp
