#pragma once
#include <array>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include "dfa.hpp"

namespace interpreter_exp {
namespace lexer {

// 基于表驱动的 DFA 实现
class TableDrivenDFA : public AbstractDFA {
private:
  // 状态表
  std::vector<DFAStateInfo> states;

  // 转移表：[state][char] -> nextState（仅支持 ASCII 0-127）
  std::vector<std::array<int, 128>> transitionTable;

  // 字符类转移表：每个状态可能有多个基于条件的转移
  std::unordered_map<int, std::vector<DFATransition>> charClassTransitions;

  // 当前状态
  int currentState;

  // 已处理的输入
  std::string processedInput;

  // 状态栈（用于回溯）
  std::vector<int> stateStack;
  std::vector<std::string> inputStack;

  // 起始状态ID 和 错误状态ID
  int startStateId;
  int errorStateId;

  // 构建默认字符类转移（字母、数字、空白、运算符等）
  void buildCharClassTransitions();

  // 添加单个字符类转移（内部辅助）
  void addCharClassTransition(char fromChar, char toChar,
                              std::function<bool(char)> condition);

public:
  TableDrivenDFA();

  // 添加状态
  int addState(DFAStateType type, TokenType tokenType = TokenType::Invalid,
               const std::string &description = "");

  // 添加转移（单个字符）
  void addTransition(int fromState, int toState, char inputChar);

  // 添加转移（字符范围）
  void addTransitionRange(int fromState, int toState, char startChar,
                          char endChar);

  // 添加转移（字符集合）
  void addTransitionSet(int fromState, int toState, const std::string &charSet);

  // 添加自定义字符类转移（带条件函数）
  void addCharClassTransition(int fromState, int toState,
                              std::function<bool(char)> condition);

  // AbstractDFA 接口实现
  void reset() override;
  bool feed(char c) override;
  const DFAStateInfo &getCurrentState() const override;
  bool isAccepting() const override;
  bool isError() const override;
  TokenType getAcceptedTokenType() const override;
  std::string getProcessedInput() const override;
  void backtrack() override;
  void saveState() override;
  void restoreState() override;
  void getStats(size_t &stateCount, size_t &transitionCount) const override;

  // 高级功能（可选实现）
  void optimize();
  void exportToFile(const std::string &filename) const;
  void importFromFile(const std::string &filename);
};

// DFA构建器：提供高级接口来构建特定类型的DFA
class TableDrivenDFABuilder {
private:
  std::shared_ptr<TableDrivenDFA> dfa;

public:
  TableDrivenDFABuilder();

  // 构建各种类型的 DFA
  void buildIdentifierDFA();
  void buildIntegerDFA();
  void buildFloatDFA();
  void buildOperatorDFA();
  void buildStringDFA();
  void buildWhitespaceDFA();

  // 获取构建完成的 DFA
  std::shared_ptr<TableDrivenDFA> getDFA() const;
};

} // namespace lexer
} // namespace interpreter_exp