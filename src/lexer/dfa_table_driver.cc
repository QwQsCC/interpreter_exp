#include "dfa_table_driver.hpp"
#include "Token.hpp"
#include <spdlog/spdlog.h>

namespace interpreter_exp {
namespace lexer {

// 基于表转移的DFA
// 构建字符类转移
void TableDrivenDFA::buildCharClassTransitions() {
  // 字母字符类
  addCharClassTransition('a', 'z',
                         [](char c) { return std::isalpha(c) || c == '_'; });
  addCharClassTransition('A', 'Z',
                         [](char c) { return std::isalpha(c) || c == '_'; });

  // 数字字符类
  addCharClassTransition('0', '9', [](char c) { return std::isdigit(c); });

  // 空白字符类
  addCharClassTransition(' ', ' ', [](char c) { return std::isspace(c); });
  addCharClassTransition('\t', '\t', [](char c) { return std::isspace(c); });
  addCharClassTransition('\n', '\n', [](char c) { return std::isspace(c); });

  // 运算符字符类
  addCharClassTransition('+', '+', [](char c) {
    return c == '+' || c == '-' || c == '*' || c == '/' || c == '=' ||
           c == '<' || c == '>' || c == '!' || c == '&' || c == '|';
  });
}

// 添加字符类转移
void TableDrivenDFA::addCharClassTransition(
    char fromChar, char toChar, std::function<bool(char)> condition) {
  // 为所有可能的状态添加字符类转移
  for (int stateId = 0; stateId < static_cast<int>(states.size()); ++stateId) {
    // 查找是否有直接的字符转移
    bool hasDirectTransition = false;
    for (char c = fromChar; c <= toChar; ++c) {
      if (transitionTable[stateId][static_cast<size_t>(c)] != errorStateId) {
        hasDirectTransition = true;
        break;
      }
    }

    if (!hasDirectTransition) {
      charClassTransitions[stateId].push_back(
          {stateId,
           errorStateId, // 默认转到错误状态，具体目标状态在feed中确定
           '\0', true, condition});
    }
  }
}

TableDrivenDFA::TableDrivenDFA()
    : currentState(0), startStateId(0), errorStateId(-1) {
  spdlog::info("TableDrivenDFA is init.");
  // 初始化转移表
  transitionTable.resize(128);
  for (auto &row : transitionTable) {
    row.fill(errorStateId);
  }

  // 构建基础的字符类转移
  buildCharClassTransitions();
}

// 添加状态
int TableDrivenDFA::addState(DFAStateType type, TokenType tokenType,
                             const std::string &description) {
  int id = static_cast<int>(states.size());
  states.push_back({id, type, tokenType, description});

  // 扩展转移表
  transitionTable.resize(states.size());
  transitionTable.back().fill(errorStateId);

  // 设置特殊状态ID
  if (type == DFAStateType::Start) {
    startStateId = id;
  } else if (type == DFAStateType::Error) {
    errorStateId = id;
  }

  return id;
}

// 添加转移（单个字符）
void TableDrivenDFA::addTransition(int fromState, int toState, char inputChar) {
  if (fromState < static_cast<int>(states.size()) &&
      toState < static_cast<int>(states.size())) {
    transitionTable[fromState][static_cast<size_t>(inputChar)] = toState;
  }
}

// 添加转移（字符范围）
void TableDrivenDFA::addTransitionRange(int fromState, int toState,
                                        char startChar, char endChar) {
  for (char c = startChar; c <= endChar; ++c) {
    addTransition(fromState, toState, c);
  }
}

// 添加转移（字符集合）
void TableDrivenDFA::addTransitionSet(int fromState, int toState,
                                      const std::string &charSet) {
  for (char c : charSet) {
    addTransition(fromState, toState, c);
  }
}

// 添加字符类转移
void TableDrivenDFA::addCharClassTransition(
    int fromState, int toState, std::function<bool(char)> condition) {
  charClassTransitions[fromState].push_back(
      {fromState, toState, '\0', true, condition});
}

void TableDrivenDFA::reset() {
  currentState = startStateId;
  processedInput.clear();
  stateStack.clear();
  inputStack.clear();
}

bool TableDrivenDFA::feed(char c) {
  // 保存当前状态以便回溯
  saveState();

  // 记录输入
  processedInput.push_back(c);

  // 首先检查字符类转移
  if (charClassTransitions.find(currentState) != charClassTransitions.end()) {
    for (const auto &trans : charClassTransitions[currentState]) {
      if (trans.matches(c)) {
        currentState = trans.toState;
        return !isError();
      }
    }
  }

  // 检查普通转移表
  if (c >= 0 && c < 128) {
    int nextState = transitionTable[currentState][static_cast<size_t>(c)];
    if (nextState != errorStateId) {
      currentState = nextState;
      return true;
    }
  }

  // 没有找到转移，进入错误状态
  if (errorStateId != -1) {
    currentState = errorStateId;
  }

  return false;
}

const DFAStateInfo &TableDrivenDFA::getCurrentState() const {
  static DFAStateInfo invalidState = {-1, DFAStateType::Error,
                                      TokenType::Invalid, "Invalid State"};
  if (currentState >= 0 && currentState < static_cast<int>(states.size())) {
    return states[currentState];
  }
  return invalidState;
}

bool TableDrivenDFA::isAccepting() const {
  if (currentState >= 0 && currentState < static_cast<int>(states.size())) {
    return states[currentState].type == DFAStateType::Accepting;
  }
  return false;
}

bool TableDrivenDFA::isError() const {
  if (currentState >= 0 && currentState < static_cast<int>(states.size())) {
    return states[currentState].type == DFAStateType::Error;
  }
  return true;
}

TokenType TableDrivenDFA::getAcceptedTokenType() const {
  if (isAccepting() && currentState >= 0 &&
      currentState < static_cast<int>(states.size())) {
    return states[currentState].tokenType;
  }
  return TokenType::Invalid;
}

std::string TableDrivenDFA::getProcessedInput() const { return processedInput; }

void TableDrivenDFA::backtrack() {
  if (!processedInput.empty() && !stateStack.empty()) {
    processedInput.pop_back();
    currentState = stateStack.back();
    stateStack.pop_back();
  }
}

void TableDrivenDFA::saveState() {
  stateStack.push_back(currentState);
  inputStack.push_back(processedInput);
}

void TableDrivenDFA::restoreState() {
  if (!stateStack.empty() && !inputStack.empty()) {
    currentState = stateStack.back();
    processedInput = inputStack.back();
    stateStack.pop_back();
    inputStack.pop_back();
  }
}

void TableDrivenDFA::getStats(size_t &stateCount,
                              size_t &transitionCount) const {
  stateCount = states.size();

  // 计算转移数量
  transitionCount = 0;
  for (const auto &row : transitionTable) {
    for (int state : row) {
      if (state != errorStateId) {
        ++transitionCount;
      }
    }
  }

  // 加上字符类转移
  for (const auto &entry : charClassTransitions) {
    transitionCount += entry.second.size();
  }
}

// 优化DFA（合并等价状态等）
void TableDrivenDFA::optimize() {
  // TODO: 实现DFA优化算法
  // 1. 移除不可达状态
  // 2. 合并等价状态
  // 3. 最小化DFA
}

// 导出DFA到文件
void TableDrivenDFA::exportToFile(const std::string &filename) const {
  // TODO: 实现DFA导出功能
}

// 从文件导入DFA
void TableDrivenDFA::importFromFile(const std::string &filename) {
  // TODO: 实现DFA导入功能
}

TableDrivenDFABuilder::TableDrivenDFABuilder()
    : dfa(std::make_shared<TableDrivenDFA>()) {
  // 初始化基础状态
  dfa->addState(DFAStateType::Start, TokenType::Invalid, "Start State");
  dfa->addState(DFAStateType::Error, TokenType::Invalid, "Error State");
}

void TableDrivenDFABuilder::buildIdentifierDFA() {
  int start = 0;
  int identifierState = dfa->addState(DFAStateType::Accepting,
                                      TokenType::Identifier, "Identifier");

  // 首字符：字母或下划线
  dfa->addCharClassTransition(start, identifierState, [](char c) {
    return std::isalpha(static_cast<unsigned char>(c)) || c == '_';
  });

  // 后续字符：字母、数字或下划线
  dfa->addCharClassTransition(identifierState, identifierState, [](char c) {
    return std::isalnum(static_cast<unsigned char>(c)) || c == '_';
  });
}

void TableDrivenDFABuilder::buildIntegerDFA() {
  int start = 0;
  int integerState = dfa->addState(DFAStateType::Accepting, TokenType::Literal,
                                   "Integer Literal");

  // 数字
  dfa->addCharClassTransition(start, integerState, [](char c) {
    return std::isdigit(static_cast<unsigned char>(c));
  });

  // 更多数字
  dfa->addCharClassTransition(integerState, integerState, [](char c) {
    return std::isdigit(static_cast<unsigned char>(c));
  });
}

void TableDrivenDFABuilder::buildFloatDFA() {
  int start = 0;
  int integerPart = dfa->addState(DFAStateType::Accepting, TokenType::Literal,
                                  "Integer Part");
  int dot = dfa->addState(DFAStateType::Accepting, TokenType::Invalid, "Dot");
  int fractionPart = dfa->addState(DFAStateType::Accepting, TokenType::Literal,
                                   "Float Literal");

  // 整数部分
  dfa->addCharClassTransition(start, integerPart, [](char c) {
    return std::isdigit(static_cast<unsigned char>(c));
  });
  dfa->addCharClassTransition(integerPart, integerPart, [](char c) {
    return std::isdigit(static_cast<unsigned char>(c));
  });

  // 小数点
  dfa->addTransition(integerPart, dot, '.');
  dfa->addTransition(start, dot, '.');

  // 小数部分
  dfa->addCharClassTransition(dot, fractionPart, [](char c) {
    return std::isdigit(static_cast<unsigned char>(c));
  });
  dfa->addCharClassTransition(fractionPart, fractionPart, [](char c) {
    return std::isdigit(static_cast<unsigned char>(c));
  });
}

void TableDrivenDFABuilder::buildOperatorDFA() {
  int start = 0;
  int singleOp = dfa->addState(DFAStateType::Accepting, TokenType::Operator,
                               "Single Operator");
  int doubleOpStart =
      dfa->addState(DFAStateType::Accepting, TokenType::Operator,
                    "Potential Double Operator");

  // 单字符运算符
  std::string singleOps = "+-*/=<>!&|^%~";
  dfa->addTransitionSet(start, singleOp, singleOps);

  // 双字符运算符开始
  dfa->addTransition(start, doubleOpStart, '=');
  dfa->addTransition(start, doubleOpStart, '!');
  dfa->addTransition(start, doubleOpStart, '<');
  dfa->addTransition(start, doubleOpStart, '>');
  dfa->addTransition(start, doubleOpStart, '&');
  dfa->addTransition(start, doubleOpStart, '|');

  // 双字符运算符完成
  int doubleOp = dfa->addState(DFAStateType::Accepting, TokenType::Operator,
                               "Double Operator");
  dfa->addTransition(doubleOpStart, doubleOp, '=');
  dfa->addTransition(doubleOpStart, doubleOp, '<');
  dfa->addTransition(doubleOpStart, doubleOp, '>');
  dfa->addTransition(doubleOpStart, doubleOp, '&');
  dfa->addTransition(doubleOpStart, doubleOp, '|');
}

void TableDrivenDFABuilder::buildStringDFA() {
  int start = 0;
  int stringStart = dfa->addState(DFAStateType::Accepting, TokenType::Invalid,
                                  "String Start Quote");
  int stringContent = dfa->addState(DFAStateType::Accepting, TokenType::Literal,
                                    "String Content");
  int escapeChar = dfa->addState(DFAStateType::Accepting, TokenType::Invalid,
                                 "Escape Character");
  int stringEnd = dfa->addState(DFAStateType::Accepting, TokenType::Literal,
                                "String Literal");

  // 开始引号
  dfa->addTransition(start, stringStart, '"');
  dfa->addTransition(start, stringStart, '\'');

  // 字符串内容（不包含引号、反斜杠、换行）
  dfa->addCharClassTransition(stringStart, stringContent, [](char c) {
    return c != '"' && c != '\'' && c != '\\' && c != '\n';
  });
  dfa->addCharClassTransition(stringContent, stringContent, [](char c) {
    return c != '"' && c != '\'' && c != '\\' && c != '\n';
  });

  // 转义字符
  dfa->addTransition(stringContent, escapeChar, '\\');
  dfa->addCharClassTransition(escapeChar, stringContent,
                              [](char c) { return true; }); // 接受任何转义字符

  // 结束引号
  dfa->addTransition(stringContent, stringEnd, '"');
  dfa->addTransition(stringContent, stringEnd, '\'');
}

void TableDrivenDFABuilder::buildWhitespaceDFA() {
  int start = 0;
  int whitespace = dfa->addState(DFAStateType::Accepting,
                                 TokenType::Invalid, // 空白通常不生成 Token
                                 "Whitespace");

  dfa->addCharClassTransition(start, whitespace, [](char c) {
    return std::isspace(static_cast<unsigned char>(c));
  });
  dfa->addCharClassTransition(whitespace, whitespace, [](char c) {
    return std::isspace(static_cast<unsigned char>(c));
  });
}

std::shared_ptr<TableDrivenDFA> TableDrivenDFABuilder::getDFA() const {
  return dfa;
}

} // namespace lexer
} // namespace interpreter_exp