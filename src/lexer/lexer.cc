#include "lexer.hpp"
#include "Token.hpp"
#include "dfa.hpp"
#include "dfa_table_driver.hpp"
#include <sstream>

namespace interpreter_exp {
namespace lexer {

// StringInputSource 实现
char StringInputSource::nextChar() {
  if (eof())
    return '\0';

  char c = source_[position_];
  position_++;

  // 更新位置信息
  location_.position++;
  location_.column++;

  if (c == '\n') {
    location_.line++;
    location_.column = 1;
  }

  return c;
}

char StringInputSource::peekChar() const {
  if (eof())
    return '\0';
  return source_[position_];
}

void StringInputSource::ungetChar() {
  if (position_ > 0) {
    position_--;
    location_.position--;
    location_.column--;

    // 如果回退的是换行符，需要特殊处理
    if (source_[position_] == '\n' && location_.line > 1) {
      location_.line--;
      // 需要计算上一行的列数
      size_t temp = position_;
      while (temp > 0 && source_[temp - 1] != '\n') {
        temp--;
      }
      location_.column = position_ - temp + 1;
    }
  }
}

// 使用DFA的词法分析器实现
class DFACompilerLexer : public Lexer {
private:
  std::unique_ptr<InputSource> input_;
  std::shared_ptr<TableDrivenDFA> dfa_;
  std::function<void(const std::string &, const SourceLocation &)>
      errorHandler_;

  // DFA状态栈
  struct DFAState {
    int stateId;
    std::string processedInput;
    SourceLocation location;
  };
  std::vector<DFAState> dfaStateStack_;

  // Token类型注册
  std::unordered_map<std::string, std::shared_ptr<TokenType>> tokenTypes_;

public:
  DFACompilerLexer() {
    // 使用DFABuilder构建词法分析器的DFA
    TableDrivenDFABuilder builder;
    builder.buildIdentifierDFA();
    builder.buildIntegerDFA();
    builder.buildFloatDFA();
    builder.buildOperatorDFA();
    builder.buildStringDFA();
    builder.buildWhitespaceDFA();

    dfa_ = builder.getDFA();

    // 注册Token类型
    registerDefaultTokenTypes();
  }

  void registerDefaultTokenTypes() {
    // 注册基础Token类型
    auto keywordType = std::make_shared<TokenType>(TokenType::Keyword);
    auto identifierType = std::make_shared<TokenType>(TokenType::Identifier);
    auto literalType = std::make_shared<TokenType>(TokenType::Literal);
    auto operatorType = std::make_shared<TokenType>(TokenType::Operator);
    auto punctuationType = std::make_shared<TokenType>(TokenType::Punctuation);

    tokenTypes_["keyword"] = keywordType;
    tokenTypes_["identifier"] = identifierType;
    tokenTypes_["literal"] = literalType;
    tokenTypes_["operator"] = operatorType;
    tokenTypes_["punctuation"] = punctuationType;
  }

  std::unique_ptr<Token> nextToken() override {
    if (!input_ || input_->eof()) {
      return std::make_unique<Token>(TokenType::Eof, "",
                                     input_->getCurrentLocation());
    }

    // 跳过空白字符
    skipWhitespace();

    if (input_->eof()) {
      return std::make_unique<Token>(TokenType::Eof, "",
                                     input_->getCurrentLocation());
    }

    // 重置DFA
    dfa_->reset();

    // 保存起始位置
    SourceLocation startLoc = input_->getCurrentLocation();

    // 使用DFA识别Token
    std::string lexeme;
    SourceLocation lastAcceptingLoc = startLoc;
    std::string lastAcceptingLexeme;
    TokenType lastAcceptingTokenType = TokenType::Invalid;

    while (!input_->eof()) {
      // 保存DFA状态
      dfa_->saveState();

      char c = input_->peekChar();

      // 尝试喂给DFA
      if (!dfa_->feed(c)) {
        // DFA无法接受这个字符，回溯到上次接受状态
        input_->ungetChar(); // 回退当前字符
        dfa_->restoreState();

        if (!lastAcceptingLexeme.empty()) {
          // 有接受状态，创建Token
          return createTokenFromDFA(lastAcceptingTokenType, lastAcceptingLexeme,
                                    lastAcceptingLoc);
        } else {
          // 没有接受状态，报告错误
          return handleError(c, startLoc);
        }
      }

      // 消费字符
      input_->nextChar();
      lexeme.push_back(c);

      // 如果DFA当前是接受状态，记录下来
      if (dfa_->isAccepting()) {
        lastAcceptingLoc = input_->getCurrentLocation();
        lastAcceptingLexeme = lexeme;
        lastAcceptingTokenType = dfa_->getAcceptedTokenType();
      }
    }

    // 处理文件结束
    if (!lastAcceptingLexeme.empty()) {
      return createTokenFromDFA(lastAcceptingTokenType, lastAcceptingLexeme,
                                lastAcceptingLoc);
    }

    return std::make_unique<Token>(TokenType::Eof, "",
                                   input_->getCurrentLocation());
  }

  std::unique_ptr<Token> createTokenFromDFA(TokenType type,
                                            const std::string &lexeme,
                                            const SourceLocation &loc) {
    // 检查是否为关键字
    auto it = g_keywordMap.find(lexeme);
    if (it != g_keywordMap.end()) {
      return std::make_unique<Token>(
          Token::makeKeyword(it->second, lexeme, loc));
    }

    // 根据Token类型创建不同的Token
    switch (type) {
    case TokenType::Identifier:
      return std::make_unique<Token>(Token(TokenType::Identifier, lexeme, loc));

    case TokenType::Literal:
      // 需要进一步判断字面量类型
      return createLiteralToken(lexeme, loc);

    case TokenType::Operator:
      return std::make_unique<Token>(
          Token::makeOperator(lexeme[0], lexeme, loc));

    case TokenType::Punctuation:
      return std::make_unique<Token>(
          Token::makeOperator(lexeme[0], lexeme, loc));

    default:
      return std::make_unique<Token>(Token(TokenType::Invalid, lexeme, loc));
    }
  }

  std::unique_ptr<Token> createLiteralToken(const std::string &lexeme,
                                            const SourceLocation &loc) {
    // 判断字面量类型
    if (lexeme.find('.') != std::string::npos ||
        lexeme.find('e') != std::string::npos ||
        lexeme.find('E') != std::string::npos) {
      // 浮点数
      return std::make_unique<Token>(
          Token::makeLiteral(LiteralType::Float, lexeme, loc));
    } else if (lexeme == "true" || lexeme == "false") {
      // 布尔值
      return std::make_unique<Token>(
          Token::makeLiteral(LiteralType::Boolean, lexeme, loc));
    } else if (lexeme.front() == '"' || lexeme.front() == '\'') {
      // 字符串
      return std::make_unique<Token>(
          Token::makeLiteral(LiteralType::String, lexeme, loc));
    } else {
      // 整数
      return std::make_unique<Token>(
          Token::makeLiteral(LiteralType::Integer, lexeme, loc));
    }
  }

  std::unique_ptr<Token> handleError(char c, const SourceLocation &loc) {
    std::string errorMsg = "Unexpected character: '";
    errorMsg += c;
    errorMsg += "'";

    if (errorHandler_) {
      errorHandler_(errorMsg, loc);
    }

    // 跳过错误字符
    input_->nextChar();

    return std::make_unique<Token>(Token::makeError(
        ErrorType::UnknownCharacter, std::string(1, c), loc, errorMsg));
  }

  bool hasMoreTokens() const override { return input_ && !input_->eof(); }

  void setInput(std::unique_ptr<InputSource> input) override {
    input_ = std::move(input);
  }

  void reset() override {
    if (input_) {
      // 重置输入源需要重新创建
      // 这里简化处理
    }
    dfa_->reset();
  }

  void pushState() override {
    // 保存当前状态
    DFAState state;
    state.stateId = dfa_->getCurrentState().id;
    state.processedInput = dfa_->getProcessedInput();
    state.location = input_->getCurrentLocation();
    dfaStateStack_.push_back(state);
  }

  void popState() override {
    if (!dfaStateStack_.empty()) {
      DFAState state = dfaStateStack_.back();
      dfaStateStack_.pop_back();

      // 恢复状态
      dfa_->reset();
      // 需要重新处理输入，这里简化处理
    }
  }

  void setState(const std::string &state) override {
    // 设置DFA状态，这里简化处理
    // 实际实现中可能需要根据状态名切换到不同的DFA
  }

  void setErrorHandler(
      std::function<void(const std::string &, const SourceLocation &)> handler)
      override {
    errorHandler_ = handler;
  }

  // TODO: 这里有些问题，需要处理一下
  //   void registerTokenType(std::shared_ptr<TokenType> tokenType) override {
  //     // 这里简化处理，实际可能需要更多的管理
  //     tokenTypes_[tokenType->name()] = tokenType;
  //   }

  std::shared_ptr<TokenType>
  getTokenType(const std::string &name) const override {
    auto it = tokenTypes_.find(name);
    if (it != tokenTypes_.end()) {
      return it->second;
    }
    return nullptr;
  }

  std::vector<std::unique_ptr<Token>> tokenizeAll() override {
    std::vector<std::unique_ptr<Token>> tokens;

    while (hasMoreTokens()) {
      auto token = nextToken();
      if (token->type == TokenType::Eof) {
        tokens.push_back(std::move(token));
        break;
      }
      if (token->type != TokenType::Invalid &&
          token->type != TokenType::Comment) { // 忽略注释
        tokens.push_back(std::move(token));
      }
    }

    return tokens;
  }

protected:
  void skipWhitespace() override {
    while (!input_->eof()) {
      char c = input_->peekChar();
      if (!std::isspace(c)) {
        break;
      }
      input_->nextChar();
    }
  }

  bool match(const std::string &pattern) override {
    // 检查接下来的字符是否匹配模式
    pushState();

    for (char expected : pattern) {
      if (input_->eof() || input_->nextChar() != expected) {
        popState();
        return false;
      }
    }

    popState();
    return true;
  }

  std::string consumeWhile(std::function<bool(char)> predicate) override {
    std::string result;

    while (!input_->eof()) {
      char c = input_->peekChar();
      if (!predicate(c)) {
        break;
      }
      result.push_back(input_->nextChar());
    }

    return result;
  }
};

// 创建词法分析器的工厂函数
std::unique_ptr<Lexer> createDFALexer() {
  return std::make_unique<DFACompilerLexer>();
}

} // namespace lexer
} // namespace interpreter_exp