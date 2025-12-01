#include "dfa_table_driver.hpp"
#include "lexer.hpp"
#include <spdlog/spdlog.h>

int main() {
  interpreter_exp::lexer::TableDrivenDFA table_drivenDFA;
  spdlog::info("Lexer example");
  return 0;
}