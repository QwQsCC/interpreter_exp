// 直接编码型DFA的实现

#include "HardCodedDFA.hpp"

namespace interpreter_exp {
namespace lexer {


HardCodedDFA::HardCodedDFA() { reset(); }

void HardCodedDFA::reset() {
  currentState_ = 0; // 初态
  processedInput_.clear();
  updateStateInfo();
  // 清空保存栈
  while (!savedStates_.empty()) {
    savedStates_.pop();
  }
}

// 状态转移函数（直接编码）
// 返回值：>=0 表示新状态，-1 表示无转移
int HardCodedDFA::move(int state, char ch) const {
  switch (state) {
  case 0: // 初态
    if (('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z') || ch == '_')
      return 1; // 标识符
    else if ('0' <= ch && ch <= '9')
      return 2; // 整数
    else if (ch == '*')
      return 4;
    else if (ch == '/')
      return 6;
    else if (ch == '+')
      return 8;
    else if (ch == '-')
      return 7;
    else if (ch == ',')
      return 9;
    else if (ch == ';')
      return 10;
    else if (ch == '(')
      return 11;
    else if (ch == ')')
      return 12;
    break;

  case 1: // 标识符
    if (('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z') || ch == '_' ||
        ('0' <= ch && ch <= '9'))
      return 1;
    break;

  case 2: // 整数部分
    if (ch == '.')
      return 3; // 小数部分
    else if ('0' <= ch && ch <= '9')
      return 2;
    else if (ch == 'e' || ch == 'E')
      return 14; // 科学计数法
    break;

  case 3: // 小数部分
    if ('0' <= ch && ch <= '9')
      return 3;
    else if (ch == 'e' || ch == 'E')
      return 14; // 科学计数法
    break;

  case 4: // *
    if (ch == '*')
      return 5; // **
    break;

  case 6: // /
    if (ch == '/')
      return 13; // //注释
    break;

  case 7: // -
    if (ch == '-')
      return 13; // --注释
    break;

  case 14: // 科学计数法e/E后
    if (ch == '+' || ch == '-')
      return 15;
    else if ('0' <= ch && ch <= '9')
      return 16;
    break;

  case 15: // 科学计数法e±后
    if ('0' <= ch && ch <= '9')
      return 16;
    break;

  case 16: // 科学计数法指数部分
    if ('0' <= ch && ch <= '9')
      return 16;
    break;

  // 终态，无法继续转移
  case 5:  // **
  case 8:  // +
  case 9:  // ,
  case 10: // ;
  case 11: // (
  case 12: // )
  case 13: // 注释
    break;
  }
  return -1; // 无转移
}

// 判断状态是否为终态
TokenType HardCodedDFA::stateIsFinal(int state) const {
  switch (state) {
  case 1:
    return TokenType::Identifier; // 标识符
  case 2:
    return TokenType::Literal; // 整数
  case 3:
    return TokenType::Literal; // 小数
  case 4:
    return TokenType::Operator; // *
  case 5:
    return TokenType::Operator; // **
  case 6:
    return TokenType::Operator; // /
  case 7:
    return TokenType::Operator; // -
  case 8:
    return TokenType::Operator; // +
  case 9:
    return TokenType::Punctuation; // ,
  case 10:
    return TokenType::Punctuation; // ;
  case 11:
    return TokenType::Punctuation; // (
  case 12:
    return TokenType::Punctuation; // )
  case 13:
    return TokenType::Comment; // 注释
  case 16:
    return TokenType::Literal; // 科学计数法
  // 状态14、15是中间态，不是终态
  default:
    return TokenType::Invalid;
  }
}

void HardCodedDFA::updateStateInfo() {
  currentStateInfo_.id = currentState_;

  if (currentState_ == 0) {
    currentStateInfo_.type = DFAStateType::Start;
    currentStateInfo_.tokenType = TokenType::Invalid;
    currentStateInfo_.description = "Start state";
  } else {
    TokenType tokenType = stateIsFinal(currentState_);
    if (tokenType != TokenType::Invalid) {
      currentStateInfo_.type = DFAStateType::Accepting;
      currentStateInfo_.tokenType = tokenType;
      currentStateInfo_.description =
          "Accepting state " + std::to_string(currentState_);
    } else if (currentState_ < 0) {
      currentStateInfo_.type = DFAStateType::Error;
      currentStateInfo_.tokenType = TokenType::Invalid;
      currentStateInfo_.description = "Error state";
    } else {
      currentStateInfo_.type = DFAStateType::Rejecting;
      currentStateInfo_.tokenType = TokenType::Invalid;
      currentStateInfo_.description =
          "Intermediate state " + std::to_string(currentState_);
    }
  }
}

bool HardCodedDFA::feed(char c) {
  if (currentState_ < 0) {
    return false; // 已在错误状态
  }

  int nextState = move(currentState_, c);
  if (nextState < 0) {
    return false; // 无有效转移
  }

  currentState_ = nextState;
  processedInput_ += c;
  updateStateInfo();
  return true;
}

const DFAStateInfo &HardCodedDFA::getCurrentState() const {
  return currentStateInfo_;
}

bool HardCodedDFA::isAccepting() const {
  return currentStateInfo_.type == DFAStateType::Accepting;
}

bool HardCodedDFA::isError() const {
  return currentState_ < 0 || currentStateInfo_.type == DFAStateType::Error;
}

TokenType HardCodedDFA::getAcceptedTokenType() const {
  if (isAccepting()) {
    return currentStateInfo_.tokenType;
  }
  return TokenType::Invalid;
}

std::string HardCodedDFA::getProcessedInput() const { return processedInput_; }

void HardCodedDFA::backtrack() {
  if (!processedInput_.empty()) {
    processedInput_.pop_back();
    // 重新从头扫描计算当前状态
    int state = 0;
    for (char c : processedInput_) {
      state = move(state, c);
      if (state < 0)
        break;
    }
    currentState_ = state;
    updateStateInfo();
  }
}

void HardCodedDFA::saveState() {
  savedStates_.push({currentState_, processedInput_});
}

void HardCodedDFA::restoreState() {
  if (!savedStates_.empty()) {
    const auto &saved = savedStates_.top();
    currentState_ = saved.state;
    processedInput_ = saved.input;
    savedStates_.pop();
    updateStateInfo();
  }
}

void HardCodedDFA::getStats(size_t &states, size_t &transitions) const {
  states = 17;      // 状态0-16
  transitions = 28; // 大约28个转移（硬编码）
}

// 工厂函数
std::unique_ptr<AbstractDFA> createHardCodedDFA() {
  return std::make_unique<HardCodedDFA>();
}

} // namespace lexer
} // namespace interpreter_exp
