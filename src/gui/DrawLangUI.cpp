// 文件：DrawLangUI.cpp
// 内容：Draw语言UI模块的基础实现
// UIManager单例实现

#include "DrawLangUI.hpp"

namespace interpreter_exp {
namespace ui {

// ============================================================================
// DrawLangUIManager 实现
// ============================================================================

DrawLangUIManager &DrawLangUIManager::getInstance() {
  static DrawLangUIManager instance;
  return instance;
}

void DrawLangUIManager::setUI(DrawLangUI *ui) { currentUI_ = ui; }

DrawLangUI *DrawLangUIManager::getUI() const { return currentUI_; }

void DrawLangUIManager::drawPixel(int x, int y, const PixelAttribute &attr) {
  if (currentUI_) {
    currentUI_->drawPixel(x, y, attr);
  }
}

void DrawLangUIManager::showMessage(int flag, const std::string &msg) {
  if (currentUI_) {
    currentUI_->showMessage(flag, msg);
  }
}

void DrawLangUIManager::clearCanvas() {
  if (currentUI_) {
    currentUI_->clearCanvas();
  }
}

void DrawLangUIManager::refresh() {
  if (currentUI_) {
    currentUI_->refresh();
  }
}

} // namespace ui
} // namespace interpreter_exp
