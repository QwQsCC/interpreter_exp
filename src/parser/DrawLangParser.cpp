// Draw语言语法分析器的实现
// 实现递归下降语法分析

#include "DrawLangParser.hpp"
#include <cmath>
#include <iostream>

namespace interpreter_exp {
namespace parser {

using namespace ast;
using namespace lexer;
using namespace errlog;

DrawLangParser::DrawLangParser(SimpleLexer *lexer)
    : lexer_(lexer), tStorage_(&defaultTValue_) {
  if (lexer_) {
    // 获取源文件名（如果有）
    // filename_ = lexer_->getSourceId();
  }
}

DrawLangParser::~DrawLangParser() = default;

std::unique_ptr<ProgramNode> DrawLangParser::parse() {
  // 创建虚拟根Token
  Token rootToken;
  rootToken.type = TokenType::Keyword;
  rootToken.lexeme = filename_;
  rootToken.sourceLocation.line = 0;
  rootToken.sourceLocation.column = 0;

  astRoot_ = std::make_unique<ProgramNode>(rootToken, filename_);

  // 获取第一个Token
  fetchToken();

  // 开始递归下降解析
  program();

  // 打印完整AST（如果启用跟踪）
  if (config_.traceParsing) {
    ErrLog::logPrint("\n// Complete AST:\n");
    printAST(astRoot_.get());
    ErrLog::logPrint("\n// END of AST.\n");
  }

  // 返回AST，即使是空的也返回有效的ProgramNode
  return std::move(astRoot_);
}

// program 的递归子程序
// 语法: program -> { statement SEMICO }

void DrawLangParser::program() {
  enter("program");

  while (currentToken_.type != TokenType::Eof) {
    auto stmt = statement();
    if (stmt) {
      // 检查是否到达文件末尾
      if (currentToken_.type == TokenType::Eof) {
        // 丢弃未完成的语句
      } else {
        astRoot_->addStatement(std::move(stmt));
      }
    }

    // 匹配分号
    matchToken(KeywordType::Semico);
  }

  leave("program");
}

// statement 的递归子程序
// 语法: statement -> origin_stmt | scale_stmt | rot_stmt | for_stmt |
// color_stmt | size_stmt

std::unique_ptr<StatementNode> DrawLangParser::statement() {
  enter("statement");
  std::unique_ptr<StatementNode> root = nullptr;

  // 根据当前Token类型选择语句类型
  if (currentToken_.type == TokenType::Keyword) {
    switch (currentToken_.keyword()) {
    case KeywordType::Origin:
      root = originStatement();
      break;
    case KeywordType::Scale:
      root = scaleStatement();
      break;
    case KeywordType::Rot:
      root = rotStatement();
      break;
    case KeywordType::For:
      root = forStatement();
      break;
    case KeywordType::Color:
      root = colorStatement();
      break;
    case KeywordType::Size:
      root = sizeStatement();
      break;
    default:
      // 检查是否是SIZE的别名（PIXSIZE, PIXELSIZE, PIX）
      {
        std::string upper = currentToken_.lexeme;
        std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
        if (upper == "SIZE" || upper == "PIXSIZE" || upper == "PIXELSIZE" ||
            upper == "PIX") {
          root = sizeStatement();
        }
      }
      break;
    }
  }

  leave("statement");
  return root;
}

// origin_statement 的递归子程序
// 语法: ORIGIN IS L_BRACKET expression COMMA expression R_BRACKET

std::unique_ptr<OriginStmtNode> DrawLangParser::originStatement() {
  enter("origin_statement");

  matchToken(KeywordType::Origin);
  auto root = std::make_unique<OriginStmtNode>(lastToken_);

  matchToken(KeywordType::Assign); // IS
  matchToken(KeywordType::L_bracket);

  auto xExpr = expression();
  root->addExpression(std::move(xExpr));

  if (config_.traceParsing) {
    printAST(root->getExpression(0));
  }

  matchToken(KeywordType::Comma);

  auto yExpr = expression();
  root->addExpression(std::move(yExpr));

  if (config_.traceParsing) {
    printAST(root->getExpression(1));
  }

  matchToken(KeywordType::R_bracket);

  leave("origin_statement");
  return root;
}

// scale_statement 的递归子程序
// 语法: SCALE IS L_BRACKET expression COMMA expression R_BRACKET

std::unique_ptr<ScaleStmtNode> DrawLangParser::scaleStatement() {
  enter("scale_statement");

  matchToken(KeywordType::Scale);
  auto root = std::make_unique<ScaleStmtNode>(lastToken_);

  matchToken(KeywordType::Assign); // IS
  matchToken(KeywordType::L_bracket);

  auto sxExpr = expression();
  root->addExpression(std::move(sxExpr));

  matchToken(KeywordType::Comma);

  auto syExpr = expression();
  root->addExpression(std::move(syExpr));

  matchToken(KeywordType::R_bracket);

  leave("scale_statement");
  return root;
}

// rot_statement 的递归子程序
// 语法: ROT IS expression

std::unique_ptr<RotStmtNode> DrawLangParser::rotStatement() {
  enter("rot_statement");

  matchToken(KeywordType::Rot);
  auto root = std::make_unique<RotStmtNode>(lastToken_);

  matchToken(KeywordType::Assign); // IS

  auto angleExpr = expression();
  root->addExpression(std::move(angleExpr));

  leave("rot_statement");
  return root;
}

// for_statement 的递归子程序
// 语法: FOR T FROM expression TO expression STEP expression
//       DRAW L_BRACKET expression COMMA expression R_BRACKET

std::unique_ptr<ForDrawStmtNode> DrawLangParser::forStatement() {
  enter("for_statement");

  matchToken(KeywordType::For);
  auto root = std::make_unique<ForDrawStmtNode>(lastToken_);

  matchToken(KeywordType::T);
  matchToken(KeywordType::From);

  auto startExpr = expression();
  root->addExpression(std::move(startExpr));

  matchToken(KeywordType::To);

  auto endExpr = expression();
  root->addExpression(std::move(endExpr));

  matchToken(KeywordType::Step);

  auto stepExpr = expression();
  root->addExpression(std::move(stepExpr));

  matchToken(KeywordType::Draw);
  matchToken(KeywordType::L_bracket);

  auto xExpr = expression();
  root->addExpression(std::move(xExpr));

  matchToken(KeywordType::Comma);

  auto yExpr = expression();
  root->addExpression(std::move(yExpr));

  matchToken(KeywordType::R_bracket);

  leave("for_statement");
  return root;
}

// color_statement 的递归子程序
// 语法: COLOR IS L_BRACKET expr COMMA expr COMMA expr R_BRACKET
//     | COLOR IS ID

std::unique_ptr<ColorStmtNode> DrawLangParser::colorStatement() {
  enter("color_statement");

  matchToken(KeywordType::Color);
  auto root = std::make_unique<ColorStmtNode>(lastToken_);

  matchToken(KeywordType::Assign); // IS

  if (checkToken(KeywordType::L_bracket)) {
    // RGB模式: color is (r, g, b)
    matchToken(KeywordType::L_bracket);

    auto rExpr = expression();
    root->addExpression(std::move(rExpr));

    matchToken(KeywordType::Comma);

    auto gExpr = expression();
    root->addExpression(std::move(gExpr));

    matchToken(KeywordType::Comma);

    auto bExpr = expression();
    root->addExpression(std::move(bExpr));

    matchToken(KeywordType::R_bracket);
  } else {
    // 颜色名称模式: color is RED
    // 这里假设当前token是标识符
    auto colorName = std::make_unique<ColorNameExprNode>(currentToken_);
    root->setColorName(std::move(colorName));
    fetchToken(); // 消耗颜色名称token
  }

  leave("color_statement");
  return root;
}

// size_statement 的递归子程序
// 语法: SIZE IS expression
//     | SIZE IS L_BRACKET expression COMMA expression R_BRACKET

std::unique_ptr<SizeStmtNode> DrawLangParser::sizeStatement() {
  enter("size_statement");

  // 消耗SIZE关键字
  Token sizeToken = currentToken_;
  fetchToken();
  auto root = std::make_unique<SizeStmtNode>(sizeToken);

  matchToken(KeywordType::Assign); // IS

  if (checkToken(KeywordType::L_bracket)) {
    // 两维模式: size is (w, h)
    matchToken(KeywordType::L_bracket);

    auto wExpr = expression();
    root->addExpression(std::move(wExpr));

    matchToken(KeywordType::Comma);

    auto hExpr = expression();
    root->addExpression(std::move(hExpr));

    matchToken(KeywordType::R_bracket);
  } else {
    // 单值模式: size is s
    auto sExpr = expression();
    root->addExpression(std::move(sExpr));
  }

  leave("size_statement");
  return root;
}

// expression 的递归子程序
// 语法: expression -> term { (PLUS | MINUS) term }

std::unique_ptr<ExpressionNode> DrawLangParser::expression() {
  enter("expression");

  auto left = term();

  while (checkToken(KeywordType::Plus) || checkToken(KeywordType::Minus)) {
    Token op = currentToken_;
    matchToken(currentToken_.keyword());

    auto right = term();
    left = makeExprNode(op, std::move(left), std::move(right));
  }

  leave("expression");
  return left;
}

// term 的递归子程序
// 语法: term -> factor { (MUL | DIV) factor }

std::unique_ptr<ExpressionNode> DrawLangParser::term() {
  auto left = factor();

  while (checkToken(KeywordType::Mul) || checkToken(KeywordType::Div)) {
    Token op = currentToken_;
    matchToken(currentToken_.keyword());

    auto right = factor();
    left = makeExprNode(op, std::move(left), std::move(right));
  }

  return left;
}

// factor 的递归子程序
// 语法: factor -> [PLUS | MINUS] component

std::unique_ptr<ExpressionNode> DrawLangParser::factor() {
  if (checkToken(KeywordType::Plus)) {
    // 一元加：创建UnaryExprNode
    Token op = currentToken_;
    matchToken(KeywordType::Plus);
    auto operand = factor();
    return std::make_unique<UnaryExprNode>(op, std::move(operand));
  } else if (checkToken(KeywordType::Minus)) {
    // 一元减：创建UnaryExprNode
    Token op = currentToken_;
    matchToken(KeywordType::Minus);
    auto operand = factor();
    return std::make_unique<UnaryExprNode>(op, std::move(operand));
  }

  return component();
}

// component 的递归子程序
// 语法: component -> atom [POWER component]  (右结合)

std::unique_ptr<ExpressionNode> DrawLangParser::component() {
  auto left = atom();

  if (checkToken(KeywordType::Power)) {
    Token op = currentToken_;
    matchToken(KeywordType::Power);

    auto right = component(); // 递归以实现右结合
    left = makeExprNode(op, std::move(left), std::move(right));
  }

  return left;
}

// atom 的递归子程序
// 语法: atom -> CONST_ID | T | FUNC L_BRACKET expression R_BRACKET | L_BRACKET
// expression R_BRACKET

std::unique_ptr<ExpressionNode> DrawLangParser::atom() {
  std::unique_ptr<ExpressionNode> root = nullptr;

  if (currentToken_.type == TokenType::Literal) {
    // 常量
    root = makeExprNode(currentToken_);
    fetchToken();
  } else if (checkToken(KeywordType::T)) {
    // 参数T
    root = std::make_unique<ParamExprNode>(currentToken_, tStorage_);
    fetchToken();
  } else if (checkToken(KeywordType::Func)) {
    // 函数调用
    Token funcToken = currentToken_;
    fetchToken();

    matchToken(KeywordType::L_bracket);
    auto arg = expression();
    matchToken(KeywordType::R_bracket);

    root = makeFuncNode(funcToken, std::move(arg));
  } else if (checkToken(KeywordType::L_bracket)) {
    // 括号表达式
    matchToken(KeywordType::L_bracket);
    root = expression();
    matchToken(KeywordType::R_bracket);
  } else if (currentToken_.type == TokenType::Identifier) {
    // 可能是常量名（如PI, E）或函数名
    // 首先检查是否是已知常量
    std::string upper = currentToken_.lexeme;
    std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);

    // 检查是否是函数调用
    Token identToken = currentToken_;
    fetchToken();

    if (checkToken(KeywordType::L_bracket)) {
      // 函数调用
      matchToken(KeywordType::L_bracket);
      auto arg = expression();
      matchToken(KeywordType::R_bracket);
      root = makeFuncNode(identToken, std::move(arg));
    } else {
      // 常量
      root = makeExprNode(identToken);
    }
  } else {
    syntaxError(2);
    // 返回一个零常量节点以继续解析
    Token zeroToken;
    zeroToken.type = TokenType::Literal;
    zeroToken.lexeme = "0";
    root = std::make_unique<ConstExprNode>(zeroToken, 0.0);
  }

  return root;
}

// Token操作

Token DrawLangParser::fetchToken() {
  if (!lexer_) {
    currentToken_.type = TokenType::Eof;
    return currentToken_;
  }

  while (true) {
    auto tokenPtr = lexer_->nextToken();
    if (!tokenPtr) {
      currentToken_.type = TokenType::Eof;
      break;
    }

    currentToken_ = *tokenPtr;

    // 跳过注释
    if (currentToken_.type == TokenType::Comment) {
      continue;
    }

    // 处理无效token
    if (currentToken_.type == TokenType::Invalid) {
      ErrLog::logPrint("Discard invalid token \"{}\" at [{}:{}]\n",
                       currentToken_.lexeme, currentToken_.sourceLocation.line,
                       currentToken_.sourceLocation.column);
      syntaxError(1);
      continue;
    }

    break;
  }

  return currentToken_;
}

void DrawLangParser::matchToken(KeywordType expected) {
  lastToken_ = currentToken_;

  // 支持Keyword, Operator, Punctuation类型的token
  bool matches = false;
  if (currentToken_.type == TokenType::Keyword ||
      currentToken_.type == TokenType::Operator ||
      currentToken_.type == TokenType::Punctuation) {
    matches = (currentToken_.keyword() == expected);
  }

  if (matches) {
    // 匹配成功
    if (config_.traceParsing) {
      for (int i = 0; i < indent_; ++i) {
        ErrLog::logPrint("  ");
      }
      ErrLog::logPrint("match token {}\n", currentToken_.lexeme);
    }
    fetchToken();
  } else {
    // 匹配失败，尝试恢复
    if (config_.recoverFromErrors) {
      while (true) {
        syntaxError(2);
        ErrLog::logPrint("Discard mismatched token \"{}\" at [{}:{}]\n",
                         currentToken_.lexeme,
                         currentToken_.sourceLocation.line,
                         currentToken_.sourceLocation.column);

        fetchToken();

        bool matchesAfterDiscard = false;
        if (currentToken_.type == TokenType::Keyword ||
            currentToken_.type == TokenType::Operator ||
            currentToken_.type == TokenType::Punctuation) {
          matchesAfterDiscard = (currentToken_.keyword() == expected);
        }

        if (matchesAfterDiscard) {
          if (config_.traceParsing) {
            ErrLog::logPrint("*** match token {} after discard\n",
                             currentToken_.lexeme);
          }
          lastToken_ = currentToken_;
          fetchToken();
          break;
        } else if (currentToken_.type == TokenType::Eof) {
          break;
        }
      }
    } else {
      syntaxError(2);
    }
  }
}

bool DrawLangParser::checkToken(KeywordType type) const {
  // 支持Keyword, Operator, Punctuation类型的token
  if (currentToken_.type == TokenType::Keyword ||
      currentToken_.type == TokenType::Operator ||
      currentToken_.type == TokenType::Punctuation) {
    return currentToken_.keyword() == type;
  }
  return false;
}

bool DrawLangParser::checkTokenType(TokenType type) const {
  return currentToken_.type == type;
}

// 错误处理

void DrawLangParser::syntaxError(const std::string &message) {
  addError(message, currentToken_.sourceLocation);
}

void DrawLangParser::syntaxError(int errorCase) {
  switch (errorCase) {
  case 1:
    addError("Lexical error: invalid character/text",
             currentToken_.sourceLocation);
    break;
  case 2:
    addError("Syntax error: unexpected token '" + currentToken_.lexeme + "'",
             currentToken_.sourceLocation);
    break;
  default:
    addError("Unknown error", currentToken_.sourceLocation);
    break;
  }
}

void DrawLangParser::addError(const std::string &message,
                              const SourceLocation &loc) {
  errors_.push_back({message, loc});
  ErrorLog::getInstance().errorAt(loc, message);
}

// 表达式节点构造

std::unique_ptr<ExpressionNode>
DrawLangParser::makeExprNode(const Token &token) {
  if (token.type == TokenType::Literal) {
    double value = 0.0;
    std::string literalStr = token.literalValue();

    // 首先检查是否是预定义常量
    std::string upper = token.lexeme;
    std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);

    if (upper == "PI") {
      value = 3.1415926535897932;
    } else if (upper == "E") {
      value = 2.7182818284590452;
    } else if (upper == "XD") {
      value = 10701;
    } else if (upper == "WXQ") {
      value = 5.28;
    } else {
      // 尝试解析数字字符串
      try {
        value = std::stod(literalStr);
      } catch (...) {
        value = 0.0;
      }
    }
    return std::make_unique<ConstExprNode>(token, value);
  } else if (token.isKeyword() && token.keyword() == KeywordType::T) {
    return std::make_unique<ParamExprNode>(token, tStorage_);
  } else {
    // 尝试从符号表查找
    // 这里简化处理，假设词法分析器已经处理了常量查找
    if (token.type == TokenType::Identifier) {
      // 可能是PI, E等常量
      std::string upper = token.lexeme;
      std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);

      double value = 0.0;
      if (upper == "PI") {
        value = 3.1415926535897932;
      } else if (upper == "E") {
        value = 2.7182818284590452;
      }
      return std::make_unique<ConstExprNode>(token, value);
    }
    return std::make_unique<ConstExprNode>(token, 0.0);
  }
}

std::unique_ptr<ExpressionNode>
DrawLangParser::makeExprNode(const Token &op,
                             std::unique_ptr<ExpressionNode> left,
                             std::unique_ptr<ExpressionNode> right) {
  return std::make_unique<BinaryExprNode>(op, std::move(left),
                                          std::move(right));
}

std::unique_ptr<ExpressionNode>
DrawLangParser::makeFuncNode(const Token &funcToken,
                             std::unique_ptr<ExpressionNode> arg) {

  // 查找函数指针
  MathFunc funcPtr = nullptr;
  std::string upper = funcToken.lexeme;
  std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);

  if (upper == "SIN")
    funcPtr = std::sin;
  else if (upper == "COS")
    funcPtr = std::cos;
  else if (upper == "TAN")
    funcPtr = std::tan;
  else if (upper == "LN")
    funcPtr = std::log;
  else if (upper == "EXP")
    funcPtr = std::exp;
  else if (upper == "SQRT")
    funcPtr = std::sqrt;
  else if (upper == "ABS")
    funcPtr = std::fabs;
  else if (upper == "ASIN")
    funcPtr = std::asin;
  else if (upper == "ACOS")
    funcPtr = std::acos;
  else if (upper == "ATAN")
    funcPtr = std::atan;
  else if (upper == "LOG")
    funcPtr = std::log10;
  else if (upper == "CEIL")
    funcPtr = std::ceil;
  else if (upper == "FLOOR")
    funcPtr = std::floor;

  return std::make_unique<FuncCallExprNode>(funcToken, std::move(arg), funcPtr);
}

// 调试输出

void DrawLangParser::enter(const char *ruleName) {
  if (config_.traceParsing) {
    for (int i = 0; i < indent_; ++i) {
      ErrLog::logPrint("  ");
    }
    ErrLog::logPrint("enter in {}\n", ruleName);
    indent_ += 2;
  }
}

void DrawLangParser::leave(const char *ruleName) {
  if (config_.traceParsing) {
    indent_ -= 2;
    for (int i = 0; i < indent_; ++i) {
      ErrLog::logPrint("  ");
    }
    ErrLog::logPrint("exit from {}\n", ruleName);
  }
}

void DrawLangParser::printAST(DrawASTNode *root, int indent) {
  if (root) {
    root->print(indent + indent_);
  }
}

} // namespace parser
} // namespace interpreter_exp
