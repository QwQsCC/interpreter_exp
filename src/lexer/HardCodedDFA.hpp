// 直接编码型DFA的声明

#pragma once
#include "dfa.hpp"
#include <stack>
#include <string>

namespace interpreter_exp {
namespace lexer {

// 硬编码DFA实现
// 状态说明：
// 0:  初态
// 1:  标识符 ID
// 2:  整数部分 CONST_ID
// 3:  小数部分 CONST_ID
// 4:  MUL (*)
// 5:  POWER (**)
// 6:  DIV (/)
// 7:  MINUS (-)
// 8:  PLUS (+)
// 9:  COMMA (,)
// 10: SEMICO (;)
// 11: L_BRACKET (()
// 12: R_BRACKET ())
// 13: COMMENT (// 或 --)
// 14: 科学计数法中间态 (遇到e/E)
// 15: 科学计数法中间态 (遇到e/E后的+/-)
// 16: 科学计数法指数部分 CONST_ID
class HardCodedDFA : public AbstractDFA {
public:
  HardCodedDFA();
  ~HardCodedDFA() override = default;

  // AbstractDFA接口实现
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
  void getStats(size_t &states, size_t &transitions) const override;

  // 调试辅助
  int getCurrentStateId() const { return currentState_; }

private:
  // 状态转移函数（硬编码实现）
  int move(int fromState, char c) const;

  // 判断状态是否为终态
  TokenType stateIsFinal(int state) const;

  // 更新当前状态信息
  void updateStateInfo();

private:
  int currentState_;              // 当前状态
  std::string processedInput_;    // 已处理的输入
  DFAStateInfo currentStateInfo_; // 当前状态信息

  // 状态保存栈
  struct SavedState {
    int state;
    std::string input;
  };
  std::stack<SavedState> savedStates_;
};

// 工厂函数
std::unique_ptr<AbstractDFA> createHardCodedDFA();

} // namespace lexer
} // namespace interpreter_exp
