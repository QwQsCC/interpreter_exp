#pragma once
#include "Token.hpp"
#include <functional>
#include <memory>
#include <vector>

namespace interpreter_exp {

namespace lexer {

// 输入源抽象 - 支持多种输入类型
class InputSource {
public:
  virtual ~InputSource() = default;

  // 读取下一个字符
  virtual char nextChar() = 0;

  // 预览下一个字符（不移动指针）
  virtual char peekChar() const = 0;

  // 回退一个字符
  virtual void ungetChar() = 0;

  // 获取当前位置
  virtual SourceLocation getCurrentLocation() const = 0;

  // 是否到达文件末尾
  virtual bool eof() const = 0;

  // 获取源标识
  virtual std::string getSourceId() const = 0;
};

// 具体实现：字符串输入源
class StringInputSource : public InputSource {
private:
  std::string source_;
  size_t position_ = 0;
  SourceLocation location_;
  std::string sourceId_;

public:
  StringInputSource(const std::string &source,
                    const std::string &sourceId = "string")
      : source_(source), sourceId_(sourceId) {
    location_.filename = sourceId;
  }

  char nextChar() override;
  char peekChar() const override;
  void ungetChar() override;
  SourceLocation getCurrentLocation() const override { return location_; }
  bool eof() const override { return position_ >= source_.length(); }
  std::string getSourceId() const override { return sourceId_; }
};

// 文件输入源
// class FileInputSource : public InputSource
// {
//     // 实现文件输入
// };

// 词法分析器抽象基类
class Lexer {
public:
  virtual ~Lexer() = default;

  // 主要接口方法
  virtual std::unique_ptr<Token> nextToken() = 0;
  virtual bool hasMoreTokens() const = 0;

  // 配置方法
  virtual void setInput(std::unique_ptr<InputSource> input) = 0;
  virtual void reset() = 0;

  // 状态管理
  virtual void pushState() = 0;                        // 保存当前状态
  virtual void popState() = 0;                         // 恢复之前状态
  virtual void setState(const std::string &state) = 0; // 设置特定状态

  // 错误处理
  virtual void setErrorHandler(
      std::function<void(const std::string &, const SourceLocation &)>
          handler) = 0;

  // Token类型注册
  virtual void registerTokenType(std::shared_ptr<TokenType> tokenType) = 0;
  virtual std::shared_ptr<TokenType>
  getTokenType(const std::string &name) const = 0;

  // 工具方法
  virtual std::vector<std::unique_ptr<Token>> tokenizeAll() = 0;

protected:
  // 供子类使用的保护方法
  virtual void skipWhitespace() = 0;
  virtual bool match(const std::string &pattern) = 0;
  virtual std::string consumeWhile(std::function<bool(char)> predicate) = 0;
};

} // namespace lexer
} // namespace interpreter_exp