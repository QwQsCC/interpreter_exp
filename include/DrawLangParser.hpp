
// Draw语言语法分析器的声明
// 实现递归下降语法分析

#pragma once

#include "DrawLangAST.hpp"
#include "ErrorLog.hpp"
#include "SimpleLexer.hpp"
#include "Token.hpp"
#include <functional>
#include <memory>
#include <stack>
#include <string>
#include <vector>

namespace interpreter_exp {
namespace parser {

// 语法分析器配置
struct DrawParserConfig {
  bool traceParsing = false;     // 是否跟踪解析过程
  bool recoverFromErrors = true; // 是否从错误中恢复
  bool enableWarnings = true;    // 是否启用警告
  size_t maxErrors = 100;        // 最大错误数
};

// 语法错误信息
struct DrawParseError {
  std::string message;
  SourceLocation location;

  std::string toString() const {
    return "[" + std::to_string(location.line) + ":" +
           std::to_string(location.column) + "] " + message;
  }
};

// Draw语言语法分析器
class DrawLangParser {
public:
  // 构造函数：需要一个词法分析器
  explicit DrawLangParser(lexer::SimpleLexer *lexer);
  ~DrawLangParser();

  // 语法分析入口
  // 返回完整程序的AST
  std::unique_ptr<ast::ProgramNode> parse();

  // 获取源文件名
  const std::string &getFilename() const { return filename_; }
  void setFilename(const std::string &name) { filename_ = name; }

  // 错误处理
  bool hasErrors() const { return !errors_.empty(); }
  const std::vector<DrawParseError> &getErrors() const { return errors_; }
  void clearErrors() { errors_.clear(); }

  // 配置
  void setConfig(const DrawParserConfig &config) { config_ = config; }
  const DrawParserConfig &getConfig() const { return config_; }

  // T值存储管理（用于语义分析）
  double *getTStorage() const { return tStorage_; }
  void setTStorage(double *ptr) { tStorage_ = ptr; }

protected:
  // 递归下降解析方法
  void program();
  std::unique_ptr<ast::StatementNode> statement();
  std::unique_ptr<ast::OriginStmtNode> originStatement();
  std::unique_ptr<ast::ScaleStmtNode> scaleStatement();
  std::unique_ptr<ast::RotStmtNode> rotStatement();
  std::unique_ptr<ast::ForDrawStmtNode> forStatement();
  std::unique_ptr<ast::ColorStmtNode> colorStatement();
  std::unique_ptr<ast::SizeStmtNode> sizeStatement();

  // 表达式解析
  std::unique_ptr<ast::ExpressionNode> expression();
  std::unique_ptr<ast::ExpressionNode> term();
  std::unique_ptr<ast::ExpressionNode> factor();
  std::unique_ptr<ast::ExpressionNode> component();
  std::unique_ptr<ast::ExpressionNode> atom();

  // Token操作
  Token fetchToken();
  void matchToken(KeywordType expected);
  bool checkToken(KeywordType type) const;
  bool checkTokenType(TokenType type) const;

  // 错误处理
  void syntaxError(const std::string &message);
  void syntaxError(int errorCase);
  void addError(const std::string &message, const SourceLocation &loc);

  // 表达式节点构造
  std::unique_ptr<ast::ExpressionNode> makeExprNode(const Token &token);
  std::unique_ptr<ast::ExpressionNode>
  makeExprNode(const Token &op, std::unique_ptr<ast::ExpressionNode> left,
               std::unique_ptr<ast::ExpressionNode> right);
  std::unique_ptr<ast::ExpressionNode>
  makeFuncNode(const Token &funcToken,
               std::unique_ptr<ast::ExpressionNode> arg);

  // 调试输出
  void enter(const char *ruleName);
  void leave(const char *ruleName);
  void printAST(ast::DrawASTNode *root, int indent = 0);

private:
  lexer::SimpleLexer *lexer_; // 词法分析器
  std::string filename_;      // 源文件名

  Token currentToken_; // 当前Token
  Token lastToken_;    // 上一个成功匹配的Token

  std::unique_ptr<ast::ProgramNode> astRoot_; // AST根节点

  double *tStorage_;           // T值存储（默认空间）
  double defaultTValue_ = 0.0; // 默认T值

  std::vector<DrawParseError> errors_; // 错误列表
  DrawParserConfig config_;            // 配置

  int indent_ = 0; // 调试缩进
};

} // namespace parser
} // namespace interpreter_exp
