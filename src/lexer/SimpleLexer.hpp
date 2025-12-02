// 简单词法分析器的声明

#pragma once
#include "TableDrivenDFA.hpp"
#include "dfa.hpp"
#include "lexer.hpp"
#include <cmath>
#include <functional>
#include <memory>
#include <stack>
#include <unordered_map>


namespace interpreter_exp {
namespace lexer {

// 函数指针类型
using MathFunc = double (*)(double);

// 符号表条目
struct SymbolEntry {
  TokenType type;      // Token类型
  KeywordType keyword; // 如果是关键字，具体是哪个
  std::string lexeme;  // 词素
  double value;        // 如果是常量，其值
  MathFunc funcPtr;    // 如果是函数，其指针

  SymbolEntry()
      : type(TokenType::Invalid), keyword(KeywordType::None), value(0.0),
        funcPtr(nullptr) {}

  SymbolEntry(TokenType t, KeywordType kw, const std::string &lex,
              double v = 0.0, MathFunc f = nullptr)
      : type(t), keyword(kw), lexeme(lex), value(v), funcPtr(f) {}
};

// 简单词法分析器实现
class SimpleLexer : public Lexer {
public:
  // 构造函数：传入InputSource和AbstractDFA（DFA默认为TableDrivenDFA）
  SimpleLexer(std::unique_ptr<InputSource> input,
              std::unique_ptr<AbstractDFA> dfa = nullptr);

  ~SimpleLexer() override = default;

  // Lexer接口实现
  std::unique_ptr<Token> nextToken() override;
  bool hasMoreTokens() const override;
  void setInput(std::unique_ptr<InputSource> input) override;
  void reset() override;
  void pushState() override;
  void popState() override;
  void setState(const std::string &state) override;
  void setErrorHandler(
      std::function<void(const std::string &, const SourceLocation &)> handler)
      override;
  void registerTokenType(std::shared_ptr<TokenType> tokenType) override;
  std::shared_ptr<TokenType>
  getTokenType(const std::string &name) const override;
  std::vector<std::unique_ptr<Token>> tokenizeAll() override;

  // 符号表操作
  void addSymbol(const std::string &name, const SymbolEntry &entry);
  const SymbolEntry *lookupSymbol(const std::string &name) const;

  // 获取函数指针（用于解释器）
  MathFunc getFunction(const std::string &name) const;

protected:
  void skipWhitespace() override;
  bool match(const std::string &pattern) override;
  std::string consumeWhile(std::function<bool(char)> predicate) override;

private:
  // 内部辅助方法
  void initSymbolTable();
  char getChar();
  void ungetChar(char c);
  bool isSpace(char c) const;

  // 预处理：跳过空白符，返回第一个非空字符
  char preProcess();

  // DFA扫描：从firstChar开始识别token
  std::string scanMove(char firstChar);

  // 后处理：根据DFA结果确定最终的token类型
  std::unique_ptr<Token> postProcess(const std::string &lexeme,
                                     const SourceLocation &location);

  // 处理注释：跳过到行尾
  void skipToEndOfLine();

  // 字符串转大写
  static std::string toUpper(const std::string &str);

private:
  std::unique_ptr<InputSource> input_;
  std::unique_ptr<AbstractDFA> dfa_;
  std::unordered_map<std::string, SymbolEntry> symbolTable_;
  std::function<void(const std::string &, const SourceLocation &)>
      errorHandler_;

  // 状态栈（用于pushState/popState）
  struct LexerState {
    SourceLocation location;
  };
  std::stack<LexerState> stateStack_;

  // 当前状态名（用于条件词法分析）
  std::string currentStateName_;

  // 标记是否还有更多tokens
  bool hasMore_ = true;
};

// DFA类型枚举
enum class DFAType {
  TableDriven, // 表驱动型
  HardCoded    // 硬编码型
};

// 工厂函数

// 创建DFA
std::unique_ptr<AbstractDFA> createDFA(DFAType type);

// 创建词法分析器（从字符串）
std::unique_ptr<SimpleLexer>
createLexerFromString(const std::string &source,
                      DFAType dfaType = DFAType::TableDriven,
                      const std::string &sourceId = "string");

// 创建词法分析器（从文件）
std::unique_ptr<SimpleLexer>
createLexerFromFile(const std::string &filename,
                    DFAType dfaType = DFAType::TableDriven);

} // namespace lexer
} // namespace interpreter_exp
