// 表驱动型DFA的声明

#pragma once
#include "dfa.hpp"
#include <stack>
#include <vector>

namespace interpreter_exp {
namespace lexer {

// 字符类定义（与老实现兼容）
enum CharClass : unsigned int {
  CK_CHAR = 0 << 16,    // 单个任意字符
  CK_LETTER = 1U << 16, // 单个字母[a-zA-Z_]
  CK_DIGIT = 2U << 16,  // 单个数字[0-9]
  CK_NULL = 0x80U << 16 // 空（结束标志）
};

// 状态转移三元组
struct StateTransition {
  unsigned int key; // 查询关键字：(fromState << 24) | (charClass | char)
  int toState;      // 目标状态
};

// 终态信息
struct FinalStateInfo {
  int state;           // 状态编号
  TokenType tokenType; // 该终态识别的Token类型
};

// 宏定义用于构建转移表
#define MK_KEY(from, c) ((unsigned int)((from) << 24) | (c))
#define DEF_TRANS(from, C, to) {MK_KEY(from, C), to}
#define TRANS_TABLE_END MK_KEY(255, CK_NULL)

// 表驱动型DFA实现
class TableDrivenDFA : public AbstractDFA {
public:
  TableDrivenDFA();
  ~TableDrivenDFA() override = default;

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
  // 状态转移函数
  int move(int fromState, char c) const;

  // 判断状态是否为终态
  TokenType stateIsFinal(int state) const;

  // 获取字符类
  static unsigned int getCharClass(char c);

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

  // 静态转移表和终态表
  static const StateTransition transitions_[];
  static const FinalStateInfo finalStates_[];
  static const size_t transitionCount_;
  static const size_t finalStateCount_;
};

// 工厂函数
std::unique_ptr<AbstractDFA> createTableDrivenDFA();

} // namespace lexer
} // namespace interpreter_exp
