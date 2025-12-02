// Draw语言词法分析器的实现
// 封装SimpleLexer，提供一致的命名风格

#include "DrawLangLexer.hpp"
#include "HardCodedDFA.hpp"
#include "TableDrivenDFA.hpp"

namespace interpreter_exp {
namespace lexer {

DrawLangLexer::DrawLangLexer(const std::string &source,
                             const std::string &sourceId)
    : SimpleLexer(std::make_unique<StringInputSource>(source, sourceId)),
      sourceId_(sourceId) {}

DrawLangLexer::DrawLangLexer(std::unique_ptr<InputSource> input,
                             std::unique_ptr<AbstractDFA> dfa)
    : SimpleLexer(std::move(input), std::move(dfa)), sourceId_("unknown") {}

namespace {

std::unique_ptr<AbstractDFA> createDFAForDrawLang(DrawLangDFAType type) {
  switch (type) {
  case DrawLangDFAType::TableDriven:
    return createTableDrivenDFA();
  case DrawLangDFAType::HardCoded:
    return createHardCodedDFA();
  default:
    return createTableDrivenDFA();
  }
}

} // anonymous namespace

std::unique_ptr<DrawLangLexer>
createDrawLangLexerFromString(const std::string &source,
                              DrawLangDFAType dfaType,
                              const std::string &sourceId) {

  auto input = std::make_unique<StringInputSource>(source, sourceId);
  auto dfa = createDFAForDrawLang(dfaType);
  auto lexer =
      std::make_unique<DrawLangLexer>(std::move(input), std::move(dfa));
  return lexer;
}

std::unique_ptr<DrawLangLexer>
createDrawLangLexerFromFile(const std::string &filename,
                            DrawLangDFAType dfaType) {

  auto input = std::make_unique<FileInputSource>(filename);
  auto dfa = createDFAForDrawLang(dfaType);
  auto lexer =
      std::make_unique<DrawLangLexer>(std::move(input), std::move(dfa));
  return lexer;
}

} // namespace lexer
} // namespace interpreter_exp
