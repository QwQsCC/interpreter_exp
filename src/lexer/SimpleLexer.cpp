// 简单词法分析器的实现

#include "SimpleLexer.hpp"
#include "HardCodedDFA.hpp"
#include "TableDrivenDFA.hpp"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdlib>

namespace interpreter_exp {
namespace lexer {


namespace {

// 演示函数
double demo_ayy(double) { return 2019.07 - 2018.10; }

// 预定义的符号表条目
struct PredefSymbol {
  const char *name;
  TokenType type;
  KeywordType keyword;
  double value;
  MathFunc funcPtr;
};

const PredefSymbol predefinedSymbols[] = {
    // 常量
    {"PI", TokenType::Literal, KeywordType::None, 3.1415926535897932, nullptr},
    {"E", TokenType::Literal, KeywordType::None, 2.7182818284590452, nullptr},
    {"XD", TokenType::Literal, KeywordType::None, 10701, nullptr},
    {"WXQ", TokenType::Literal, KeywordType::None, 5.28, nullptr},

    // 参数T
    {"T", TokenType::Keyword, KeywordType::T, 0.0, nullptr},

    // 内置函数
    {"SIN", TokenType::Keyword, KeywordType::Func, 0.0, std::sin},
    {"COS", TokenType::Keyword, KeywordType::Func, 0.0, std::cos},
    {"TAN", TokenType::Keyword, KeywordType::Func, 0.0, std::tan},
    {"LN", TokenType::Keyword, KeywordType::Func, 0.0, std::log},
    {"EXP", TokenType::Keyword, KeywordType::Func, 0.0, std::exp},
    {"SQRT", TokenType::Keyword, KeywordType::Func, 0.0, std::sqrt},
    {"_AYY_", TokenType::Keyword, KeywordType::Func, 0.0, demo_ayy},

    // 关键字
    {"ORIGIN", TokenType::Keyword, KeywordType::Origin, 0.0, nullptr},
    {"SCALE", TokenType::Keyword, KeywordType::Scale, 0.0, nullptr},
    {"ROT", TokenType::Keyword, KeywordType::Rot, 0.0, nullptr},
    {"IS", TokenType::Keyword, KeywordType::Assign, 0.0, nullptr},
    {"FOR", TokenType::Keyword, KeywordType::For, 0.0, nullptr},
    {"FROM", TokenType::Keyword, KeywordType::From, 0.0, nullptr},
    {"TO", TokenType::Keyword, KeywordType::To, 0.0, nullptr},
    {"STEP", TokenType::Keyword, KeywordType::Step, 0.0, nullptr},
    {"DRAW", TokenType::Keyword, KeywordType::Draw, 0.0, nullptr},
    {"COLOR", TokenType::Keyword, KeywordType::Color, 0.0, nullptr},
    {"SIZE", TokenType::Keyword, KeywordType::Size, 0.0, nullptr},

    // 结束标记
    {nullptr, TokenType::Invalid, KeywordType::None, 0.0, nullptr}};

// 操作符到KeywordType的映射
KeywordType getOperatorKeyword(const std::string &op) {
  if (op == "+")
    return KeywordType::Plus;
  if (op == "-")
    return KeywordType::Minus;
  if (op == "*")
    return KeywordType::Mul;
  if (op == "/")
    return KeywordType::Div;
  if (op == "**")
    return KeywordType::Power;
  return KeywordType::None;
}

// 标点到KeywordType的映射
KeywordType getPunctuationKeyword(const std::string &punct) {
  if (punct == "(")
    return KeywordType::L_bracket;
  if (punct == ")")
    return KeywordType::R_bracket;
  if (punct == ";")
    return KeywordType::Semico;
  if (punct == ",")
    return KeywordType::Comma;
  return KeywordType::None;
}

} // anonymous namespace

SimpleLexer::SimpleLexer(std::unique_ptr<InputSource> input,
                         std::unique_ptr<AbstractDFA> dfa)
    : input_(std::move(input)),
      dfa_(dfa ? std::move(dfa) : std::make_unique<TableDrivenDFA>()) {
  initSymbolTable();
}

void SimpleLexer::initSymbolTable() {
  symbolTable_.clear();

  for (const PredefSymbol *p = predefinedSymbols; p->name != nullptr; ++p) {
    SymbolEntry entry(p->type, p->keyword, p->name, p->value, p->funcPtr);
    symbolTable_[p->name] = entry;
  }

  // 添加SIZE的别名
  auto it = symbolTable_.find("SIZE");
  if (it != symbolTable_.end()) {
    symbolTable_["PIXELSIZE"] = it->second;
    symbolTable_["PIXSIZE"] = it->second;
    symbolTable_["PIX"] = it->second;
  }
}

void SimpleLexer::setInput(std::unique_ptr<InputSource> input) {
  input_ = std::move(input);
  hasMore_ = true;
  dfa_->reset();
}

void SimpleLexer::reset() {
  hasMore_ = true;
  if (dfa_) {
    dfa_->reset();
  }
  while (!stateStack_.empty()) {
    stateStack_.pop();
  }
  currentStateName_.clear();
}

bool SimpleLexer::hasMoreTokens() const {
  return hasMore_ && input_ && !input_->eof();
}

char SimpleLexer::getChar() {
  if (!input_)
    return '\0';
  return input_->nextChar();
}

void SimpleLexer::ungetChar(char c) {
  if (!input_ || c == '\0' || c == '\n')
    return;
  input_->ungetChar();
}

bool SimpleLexer::isSpace(char c) const {
  if (c < 0 || c > 0x7e)
    return false;
  return std::isspace(static_cast<unsigned char>(c));
}

char SimpleLexer::preProcess() {
  char c;
  while (true) {
    c = getChar();
    if (c == '\0') {
      return '\0'; // EOF
    }
    if (!isSpace(c)) {
      break;
    }
  }
  return c;
}

std::string SimpleLexer::scanMove(char firstChar) {
  dfa_->reset();
  std::string lexeme;
  char c = firstChar;

  while (c != '\0') {
    if (dfa_->feed(c)) {
      lexeme += c;
      c = getChar();
    } else {
      // 无法继续转移
      if (lexeme.empty()) {
        // 第一个字符就无法识别
        lexeme += c;
      } else {
        // 回退当前字符
        ungetChar(c);
      }
      break;
    }
  }

  return lexeme;
}

void SimpleLexer::skipToEndOfLine() {
  char c;
  while (true) {
    c = getChar();
    if (c == '\n' || c == '\r' || c == '\0') {
      break;
    }
  }
}

std::string SimpleLexer::toUpper(const std::string &str) {
  std::string result;
  result.reserve(str.size());
  for (char c : str) {
    result += static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
  }
  return result;
}

std::unique_ptr<Token>
SimpleLexer::postProcess(const std::string &lexeme,
                         const SourceLocation &location) {
  TokenType dfaTokenType = dfa_->getAcceptedTokenType();

  switch (dfaTokenType) {
  case TokenType::Identifier: {
    // 查符号表
    std::string upperLexeme = toUpper(lexeme);
    auto it = symbolTable_.find(upperLexeme);
    if (it != symbolTable_.end()) {
      const SymbolEntry &entry = it->second;
      if (entry.type == TokenType::Literal) {
        // 命名常量
        return std::make_unique<Token>(
            Token::makeLiteral(LiteralType::Float, lexeme, location));
      } else if (entry.keyword == KeywordType::Func) {
        // 函数
        return std::make_unique<Token>(
            Token::makeKeyword(KeywordType::Func, lexeme, location));
      } else if (entry.keyword != KeywordType::None) {
        // 其他关键字
        return std::make_unique<Token>(
            Token::makeKeyword(entry.keyword, lexeme, location));
      }
    }
    // 普通标识符
    return std::make_unique<Token>(TokenType::Identifier, lexeme, location);
  }

  case TokenType::Literal: {
    // 数值字面量
    bool isFloat = (lexeme.find('.') != std::string::npos) ||
                   (lexeme.find('e') != std::string::npos) ||
                   (lexeme.find('E') != std::string::npos);
    LiteralType litType = isFloat ? LiteralType::Float : LiteralType::Integer;
    return std::make_unique<Token>(
        Token::makeLiteral(litType, lexeme, location));
  }

  case TokenType::Operator: {
    KeywordType kw = getOperatorKeyword(lexeme);
    // 创建Operator类型的Token，保存KeywordType
    auto token = std::make_unique<Token>(TokenType::Operator, lexeme, location);
    token->payload = kw;
    return token;
  }

  case TokenType::Punctuation: {
    KeywordType kw = getPunctuationKeyword(lexeme);
    // 创建Punctuation类型的Token，保存KeywordType
    auto token =
        std::make_unique<Token>(TokenType::Punctuation, lexeme, location);
    token->payload = kw;
    return token;
  }

  case TokenType::Comment: {
    // 跳过注释内容到行尾
    skipToEndOfLine();
    // 返回nullptr表示需要继续获取下一个token
    return nullptr;
  }

  default: {
    // 错误或无效token
    return std::make_unique<Token>(
        Token::makeError(ErrorType::UnknownCharacter, lexeme, location,
                         "Unknown token: " + lexeme));
  }
  }
}

std::unique_ptr<Token> SimpleLexer::nextToken() {
  if (!input_) {
    hasMore_ = false;
    return std::make_unique<Token>(TokenType::Eof, "", SourceLocation());
  }

  while (true) {
    // 预处理：跳过空白符
    char firstChar = preProcess();
    if (firstChar == '\0') {
      hasMore_ = false;
      return std::make_unique<Token>(TokenType::Eof, "",
                                     input_->getCurrentLocation());
    }

    // 记录位置
    SourceLocation location = input_->getCurrentLocation();
    // 调整列号（因为已经读取了firstChar）
    if (location.column > 0) {
      location.column--;
    }

    // DFA扫描
    std::string lexeme = scanMove(firstChar);

    // 后处理
    auto token = postProcess(lexeme, location);

    if (token != nullptr) {
      return token;
    }
    // token为nullptr表示是注释，继续获取下一个token
  }
}

std::vector<std::unique_ptr<Token>> SimpleLexer::tokenizeAll() {
  std::vector<std::unique_ptr<Token>> tokens;

  while (hasMoreTokens()) {
    auto token = nextToken();
    if (token->type == TokenType::Eof) {
      tokens.push_back(std::move(token));
      break;
    }
    tokens.push_back(std::move(token));
  }

  return tokens;
}

void SimpleLexer::pushState() {
  if (input_) {
    stateStack_.push({input_->getCurrentLocation()});
  }
}

void SimpleLexer::popState() {
  if (!stateStack_.empty()) {
    stateStack_.pop();
  }
}

void SimpleLexer::setState(const std::string &state) {
  currentStateName_ = state;
}

void SimpleLexer::setErrorHandler(
    std::function<void(const std::string &, const SourceLocation &)> handler) {
  errorHandler_ = std::move(handler);
}

void SimpleLexer::registerTokenType(std::shared_ptr<TokenType> tokenType) {
  // 保留扩展接口
}

std::shared_ptr<TokenType>
SimpleLexer::getTokenType(const std::string &name) const {
  // 保留扩展接口
  return nullptr;
}

void SimpleLexer::skipWhitespace() {
  while (input_ && !input_->eof()) {
    char c = input_->peekChar();
    if (!isSpace(c))
      break;
    input_->nextChar();
  }
}

bool SimpleLexer::match(const std::string &pattern) {
  for (char expected : pattern) {
    if (!input_ || input_->eof())
      return false;
    char c = input_->nextChar();
    if (c != expected) {
      input_->ungetChar();
      return false;
    }
  }
  return true;
}

std::string SimpleLexer::consumeWhile(std::function<bool(char)> predicate) {
  std::string result;
  while (input_ && !input_->eof()) {
    char c = input_->peekChar();
    if (!predicate(c))
      break;
    result += input_->nextChar();
  }
  return result;
}

void SimpleLexer::addSymbol(const std::string &name, const SymbolEntry &entry) {
  symbolTable_[toUpper(name)] = entry;
}

const SymbolEntry *SimpleLexer::lookupSymbol(const std::string &name) const {
  auto it = symbolTable_.find(toUpper(name));
  if (it != symbolTable_.end()) {
    return &(it->second);
  }
  return nullptr;
}

MathFunc SimpleLexer::getFunction(const std::string &name) const {
  const SymbolEntry *entry = lookupSymbol(name);
  if (entry && entry->funcPtr) {
    return entry->funcPtr;
  }
  return nullptr;
}

std::unique_ptr<AbstractDFA> createDFA(DFAType type) {
  switch (type) {
  case DFAType::TableDriven:
    return createTableDrivenDFA();
  case DFAType::HardCoded:
    return createHardCodedDFA();
  default:
    return createTableDrivenDFA();
  }
}

std::unique_ptr<SimpleLexer>
createLexerFromString(const std::string &source, DFAType dfaType,
                      const std::string &sourceId) {
  auto input = std::make_unique<StringInputSource>(source, sourceId);
  auto dfa = createDFA(dfaType);
  return std::make_unique<SimpleLexer>(std::move(input), std::move(dfa));
}

std::unique_ptr<SimpleLexer> createLexerFromFile(const std::string &filename,
                                                 DFAType dfaType) {
  auto input = std::make_unique<FileInputSource>(filename);
  auto dfa = createDFA(dfaType);
  return std::make_unique<SimpleLexer>(std::move(input), std::move(dfa));
}

} // namespace lexer
} // namespace interpreter_exp