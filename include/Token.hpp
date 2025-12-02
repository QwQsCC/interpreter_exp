#pragma once

#include <cctype>
#include <cstring>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace interpreter_exp {

// 基础类别几乎不会变化所以使用枚举类
enum class TokenType {
  Keyword,     // 关键字
  Identifier,  // 标识符
  Literal,     // 字面量
  Operator,    // 操作符
  Punctuation, // 分隔符
  Comment,     // 注释
  Eof,         // 结束符
  Invalid      // 无效
};

// 利用宏定义来配置关键字
#define KEYWORD_LIST                                                           \
  KEYWORD(If, "if")                                                            \
  KEYWORD(Else, "else")                                                        \
  KEYWORD(While, "while")                                                      \
  KEYWORD(For, "for")                                                          \
  KEYWORD(From, "from")                                                        \
  KEYWORD(To, "to")                                                            \
  KEYWORD(Step, "step")                                                        \
  KEYWORD(Draw, "draw")                                                        \
  KEYWORD(T, "t")                                                              \
  KEYWORD(Return, "return")                                                    \
                                                                               \
  KEYWORD(Color, "color")                                                      \
  KEYWORD(Scale, "scale")                                                      \
  KEYWORD(Rot, "rot")                                                          \
  KEYWORD(Origin, "origin")                                                    \
  KEYWORD(Size, "size")                                                        \
  KEYWORD(L_bracket, "(")                                                      \
  KEYWORD(R_bracket, ")")                                                      \
  KEYWORD(Semico, ";")                                                         \
  KEYWORD(Comma, ",")                                                          \
  KEYWORD(Assign, "==")                                                        \
  KEYWORD(Plus, "+")                                                           \
  KEYWORD(Minus, "-")                                                          \
  KEYWORD(Mul, "*")                                                            \
  KEYWORD(Div, "/")                                                            \
  KEYWORD(Power, "^")                                                          \
                                                                               \
  KEYWORD(Int, "int")                                                          \
  KEYWORD(Float, "float")                                                      \
  KEYWORD(Double, "double")                                                    \
  KEYWORD(Bool, "bool")                                                        \
  KEYWORD(Func, "func")                                                        \
                                                                               \
  KEYWORD(Error_token, "error_token")

// 关键字类型
enum class KeywordType {
#define KEYWORD(name, str) name,
  KEYWORD_LIST
#undef KEYWORD
      None // None不在KEYWORD_LIST中，单独定义
};

// 关键字map
static const std::unordered_map<std::string, KeywordType> g_keywordMap = {
#define KEYWORD(name, str) {str, KeywordType::name},
    KEYWORD_LIST
#undef KEYWORD
};

enum class LiteralType { Integer, Float, String, Boolean };

enum class ErrorType {
  UnknownCharacter,
  InvalidNumberFormat,
  UnterminatedString,
  UnexpectedEndOfFile,
  Other
};

// 源代码位置类
struct SourceLocation {
  std::string filename;
  size_t line = 1;
  size_t column = 1;
  size_t position = 0;

  std::string toString() const {
    return filename + ":" + std::to_string(line) + ":" + std::to_string(column);
  }

  SourceLocation() : filename(""), line(1), column(1), position(0) {}

  SourceLocation(std::string filename_, size_t line_, size_t column_,
                 size_t position_)
      : filename(filename_), line(line_), column(column_), position(position_) {
  }
};

struct Token {
  TokenType type;
  std::string lexeme;
  SourceLocation sourceLocation;

  // 使用variant方便保存每种Token的类型和信息
  std::variant<std::monostate,                      // 对应 Identifier
               KeywordType,                         // 对应 Keyword
               std::pair<LiteralType, std::string>, // 对应 Literal
               char,                             // 对应 Operator/Punctuation
               std::pair<ErrorType, std::string> // 对应 Invalid
               >
      payload;

  Token()
      : type(TokenType::Invalid), lexeme(""), sourceLocation(SourceLocation()),
        payload(std::monostate{}) {}

  Token(TokenType t, std::string lex, SourceLocation sl)
      : type(t), lexeme(lex), sourceLocation(sl), payload(std::monostate{}) {}

  // 关键字Token的构造
  static Token makeKeyword(KeywordType kw, std::string lex, SourceLocation sl) {
    Token tok(TokenType::Keyword, lex, sl);
    tok.payload = kw;
    return tok;
  }

  // 字面量Token的构造
  static Token makeLiteral(LiteralType lt, std::string lex, SourceLocation sl) {
    Token tok(TokenType::Literal, lex, sl);
    tok.payload = std::make_pair(lt, std::string(lex));
    return tok;
  }

  // 运算符/标点Token的构造
  static Token makeOperator(char op, std::string lex, SourceLocation sl) {
    Token tok(TokenType::Operator, lex, sl);
    tok.payload = op;
    return tok;
  }

  // 专用于错误的构造（新增）
  static Token makeError(ErrorType et, std::string lex, SourceLocation sl,
                         const std::string &msg) {
    Token tok(TokenType::Invalid, lex, sl);
    tok.payload = std::make_pair(et, msg);
    return tok;
  }

  // 类型安全的访问方法
  template <typename T> const T &get() const { return std::get<T>(payload); }

  // 检查是否为关键字
  bool isKeyword() const { return type == TokenType::Keyword; }

  // 获取关键字类型 - 也支持从Operator和Punctuation获取
  KeywordType keyword() const {
    if (type == TokenType::Keyword) {
      return get<KeywordType>();
    } else if (type == TokenType::Operator || type == TokenType::Punctuation) {
      // 对于运算符和标点符号，payload存储的也是KeywordType
      if (std::holds_alternative<KeywordType>(payload)) {
        return get<KeywordType>();
      }
    }
    return KeywordType::None;
  }

  // 检查是否为字面量
  bool isLiteral() const { return type == TokenType::Literal; }

  // 获取字面量类型
  LiteralType literalType() const {
    return isLiteral() ? get<std::pair<LiteralType, std::string>>().first
                       : LiteralType::Integer;
  }

  // 获取字面量值
  std::string literalValue() const {
    return isLiteral() ? get<std::pair<LiteralType, std::string>>().second : "";
  }

  // 检查是否为运算符
  bool isOperator() const { return type == TokenType::Operator; }

  // 获取运算符字符
  char operatorChar() const { return isOperator() ? get<char>() : '\0'; }

  // 检查是否为错误
  bool isError() const { return type == TokenType::Invalid; }

  // 获取错误类型
  ErrorType errorType() const {
    return isError() ? get<std::pair<ErrorType, std::string>>().first
                     : ErrorType::Other;
  }

  // 获取错误信息
  std::string errorMessage() const {
    return isError() ? get<std::pair<ErrorType, std::string>>().second : "";
  }

  // 用于Token类型判定的运算符
  bool operator==(TokenType type) { return this->type == type; }
};

} // namespace interpreter_exp
