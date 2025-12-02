// Draw语言词法分析器的声明
// 为保持与DrawLangParser/DrawLangSemantic的命名一致性，对SimpleLexer进行封装

#pragma once

#include "SimpleLexer.hpp"
#include "lexer.hpp"
#include <memory>
#include <string>

namespace interpreter_exp {
namespace lexer {

// DrawLangLexer - Draw语言词法分析器
// 封装SimpleLexer，提供一致的命名风格

class DrawLangLexer : public SimpleLexer {
public:
  // 从字符串创建
  explicit DrawLangLexer(const std::string &source,
                         const std::string &sourceId = "string");

  // 从InputSource创建
  explicit DrawLangLexer(std::unique_ptr<InputSource> input,
                         std::unique_ptr<AbstractDFA> dfa = nullptr);

  ~DrawLangLexer() override = default;

  // 获取源文件路径/标识
  const std::string &getSourceId() const { return sourceId_; }

private:
  std::string sourceId_;
};

// DFA类型选择

enum class DrawLangDFAType {
  TableDriven, // 表驱动型（默认）
  HardCoded    // 硬编码型
};

// 工厂函数

// 从字符串创建Draw语言词法分析器
std::unique_ptr<DrawLangLexer> createDrawLangLexerFromString(
    const std::string &source,
    DrawLangDFAType dfaType = DrawLangDFAType::TableDriven,
    const std::string &sourceId = "string");

// 从文件创建Draw语言词法分析器
std::unique_ptr<DrawLangLexer> createDrawLangLexerFromFile(
    const std::string &filename,
    DrawLangDFAType dfaType = DrawLangDFAType::TableDriven);

} // namespace lexer
} // namespace interpreter_exp
