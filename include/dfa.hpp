#pragma once
#include "Token.hpp"
#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

namespace interpreter_exp {
namespace lexer {

// DFA状态类型
enum class DFAStateType {
  Start,     // 起始状态
  Accepting, // 接受状态
  Rejecting, // 拒绝状态
  Error      // 错误状态
};

// DFA状态信息
struct DFAStateInfo {
  int id;                  // 状态ID
  DFAStateType type;       // 状态类型
  TokenType tokenType;     // 接受的Token类型（如果为接受状态）
  std::string description; // 状态描述
};

// DFA转移信息
struct DFATransition {
  int fromState;                       // 源状态
  int toState;                         // 目标状态
  char inputChar;                      // 输入字符
  bool isCharClass = false;            // 是否为字符类
  std::function<bool(char)> condition; // 转移条件（用于字符类）

  // 检查是否匹配
  bool matches(char c) const {
    if (isCharClass) {
      return condition(c);
    }
    return inputChar == c;
  }
};

// 抽象DFA接口
class AbstractDFA {
public:
  virtual ~AbstractDFA() = default;

  // 重置DFA到初始状态
  virtual void reset() = 0;

  // 处理输入字符
  virtual bool feed(char c) = 0;

  // 获取当前状态
  virtual const DFAStateInfo &getCurrentState() const = 0;

  // 检查当前是否为接受状态
  virtual bool isAccepting() const = 0;

  // 检查是否在错误状态
  virtual bool isError() const = 0;

  // 获取当前接受的Token类型（如果是接受状态）
  virtual TokenType getAcceptedTokenType() const = 0;

  // 获取已处理的输入
  virtual std::string getProcessedInput() const = 0;

  // 回退一个字符（用于回溯）
  virtual void backtrack() = 0;

  // 保存当前状态
  virtual void saveState() = 0;

  // 恢复保存的状态
  virtual void restoreState() = 0;

  // 获取DFA统计信息
  virtual void getStats(size_t &states, size_t &transitions) const = 0;
};
} // namespace lexer
} // namespace interpreter_exp