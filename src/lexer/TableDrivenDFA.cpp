// 表驱动型DFA的实现

#include "TableDrivenDFA.hpp"
#include <cctype>

namespace interpreter_exp {
namespace lexer {


// 转移表定义


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

const StateTransition TableDrivenDFA::transitions_[] = {
    // 从状态0开始的转移
    DEF_TRANS(0, CK_LETTER, 1), // 字母 -> 标识符
    DEF_TRANS(0, CK_DIGIT, 2),  // 数字 -> 整数
    DEF_TRANS(0, '*', 4),       // * -> MUL
    DEF_TRANS(0, '/', 6),       // / -> DIV
    DEF_TRANS(0, '+', 8),       // + -> PLUS
    DEF_TRANS(0, '-', 7),       // - -> MINUS
    DEF_TRANS(0, ',', 9),       // , -> COMMA
    DEF_TRANS(0, ';', 10),      // ; -> SEMICO
    DEF_TRANS(0, '(', 11),      // ( -> L_BRACKET
    DEF_TRANS(0, ')', 12),      // ) -> R_BRACKET

    // 状态1：标识符
    DEF_TRANS(1, CK_LETTER, 1), // 继续读取字母
    DEF_TRANS(1, CK_DIGIT, 1),  // 标识符中可以有数字

    // 状态2：整数
    DEF_TRANS(2, CK_DIGIT, 2), // 继续读取数字
    DEF_TRANS(2, '.', 3),      // 遇到小数点 -> 小数部分
    DEF_TRANS(2, 'e', 14),     // 科学计数法
    DEF_TRANS(2, 'E', 14),

    // 状态3：小数部分
    DEF_TRANS(3, CK_DIGIT, 3), // 继续读取数字
    DEF_TRANS(3, 'e', 14),     // 科学计数法
    DEF_TRANS(3, 'E', 14),

    // 状态4：*
    DEF_TRANS(4, '*', 5), // ** -> POWER

    // 状态6：/
    DEF_TRANS(6, '/', 13), // // -> COMMENT

    // 状态7：-
    DEF_TRANS(7, '-', 13), // -- -> COMMENT

    // 状态14：科学计数法e/E后
    DEF_TRANS(14, '+', 15),      // e+
    DEF_TRANS(14, '-', 15),      // e-
    DEF_TRANS(14, CK_DIGIT, 16), // e后直接是数字

    // 状态15：科学计数法e±后
    DEF_TRANS(15, CK_DIGIT, 16), // e±后必须是数字

    // 状态16：科学计数法指数部分
    DEF_TRANS(16, CK_DIGIT, 16), // 继续读取指数

    // 结束标志
    {TRANS_TABLE_END, 255}};

const size_t TableDrivenDFA::transitionCount_ =
    sizeof(transitions_) / sizeof(transitions_[0]) - 1; // 不计结束标志


// 终态表定义


const FinalStateInfo TableDrivenDFA::finalStates_[] = {
    {1, TokenType::Identifier},   // 标识符
    {2, TokenType::Literal},      // 整数
    {3, TokenType::Literal},      // 小数
    {4, TokenType::Operator},     // *
    {5, TokenType::Operator},     // **
    {6, TokenType::Operator},     // /
    {7, TokenType::Operator},     // -
    {8, TokenType::Operator},     // +
    {9, TokenType::Punctuation},  // ,
    {10, TokenType::Punctuation}, // ;
    {11, TokenType::Punctuation}, // (
    {12, TokenType::Punctuation}, // )
    {13, TokenType::Comment},     // 注释
    {16, TokenType::Literal},     // 科学计数法
    // 注意：状态14、15不是终态，是科学计数法的中间状态
    {-1, TokenType::Invalid} // 结束标志
};

const size_t TableDrivenDFA::finalStateCount_ =
    sizeof(finalStates_) / sizeof(finalStates_[0]) - 1; // 不计结束标志


// TableDrivenDFA 实现


TableDrivenDFA::TableDrivenDFA() { reset(); }

void TableDrivenDFA::reset() {
  currentState_ = 0; // 初态
  processedInput_.clear();
  updateStateInfo();
  // 清空保存栈
  while (!savedStates_.empty()) {
    savedStates_.pop();
  }
}

unsigned int TableDrivenDFA::getCharClass(char c) {
  if (('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_') {
    return CK_LETTER;
  } else if ('0' <= c && c <= '9') {
    return CK_DIGIT;
  }
  return CK_CHAR;
}

int TableDrivenDFA::move(int fromState, char c) const {
  unsigned int charClass = getCharClass(c);

  // 构建查询关键字
  unsigned int key;
  if (charClass == CK_CHAR) {
    // 普通字符，直接使用字符值
    key = MK_KEY(fromState, c);
  } else {
    // 字符类，使用类标识
    key = MK_KEY(fromState, charClass);
  }

  // 在转移表中查找
  for (size_t i = 0; i < transitionCount_; ++i) {
    if (transitions_[i].key == key) {
      return transitions_[i].toState;
    }
  }

  // 对于字母和数字，如果字符类查询失败，尝试作为普通字符查询
  // 这用于处理特殊字符如 'e', 'E' 在科学计数法中的情况
  if (charClass != CK_CHAR) {
    key = MK_KEY(fromState, static_cast<unsigned int>(c));
    for (size_t i = 0; i < transitionCount_; ++i) {
      if (transitions_[i].key == key) {
        return transitions_[i].toState;
      }
    }
  }

  return -1; // 无转移
}

TokenType TableDrivenDFA::stateIsFinal(int state) const {
  for (size_t i = 0; i < finalStateCount_; ++i) {
    if (finalStates_[i].state == state) {
      return finalStates_[i].tokenType;
    }
  }
  return TokenType::Invalid;
}

void TableDrivenDFA::updateStateInfo() {
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

bool TableDrivenDFA::feed(char c) {
  if (currentState_ < 0) {
    return false; // 已经在错误状态
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

const DFAStateInfo &TableDrivenDFA::getCurrentState() const {
  return currentStateInfo_;
}

bool TableDrivenDFA::isAccepting() const {
  return currentStateInfo_.type == DFAStateType::Accepting;
}

bool TableDrivenDFA::isError() const {
  return currentState_ < 0 || currentStateInfo_.type == DFAStateType::Error;
}

TokenType TableDrivenDFA::getAcceptedTokenType() const {
  if (isAccepting()) {
    return currentStateInfo_.tokenType;
  }
  return TokenType::Invalid;
}

std::string TableDrivenDFA::getProcessedInput() const {
  return processedInput_;
}

void TableDrivenDFA::backtrack() {
  if (!processedInput_.empty()) {
    processedInput_.pop_back();
    // 注意：回退后需要重新计算当前状态
    // 简单实现：重新从头扫描
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

void TableDrivenDFA::saveState() {
  savedStates_.push({currentState_, processedInput_});
}

void TableDrivenDFA::restoreState() {
  if (!savedStates_.empty()) {
    const auto &saved = savedStates_.top();
    currentState_ = saved.state;
    processedInput_ = saved.input;
    savedStates_.pop();
    updateStateInfo();
  }
}

void TableDrivenDFA::getStats(size_t &states, size_t &transitions) const {
  states = 17; // 状态0-16
  transitions = transitionCount_;
}

std::unique_ptr<AbstractDFA> createTableDrivenDFA() {
  return std::make_unique<TableDrivenDFA>();
}

} // namespace lexer
} // namespace interpreter_exp
