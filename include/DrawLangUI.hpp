// 文件：DrawLangUI.hpp
// 内容：Draw语言UI模块的接口声明
// 基于旧module_UI接口设计，保持风格一致

#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace interpreter_exp {
namespace ui {

// ============================================================================
// 像素属性
// ============================================================================

struct PixelAttribute {
  unsigned char r = 255; // 红色分量
  unsigned char g = 0;   // 绿色分量
  unsigned char b = 0;   // 蓝色分量
  int size = 1;          // 像素大小

  PixelAttribute() = default;

  PixelAttribute(unsigned char red, unsigned char green, unsigned char blue,
                 int sz = 1)
      : r(red), g(green), b(blue), size(sz) {}

  void setColor(unsigned char red, unsigned char green, unsigned char blue) {
    r = red;
    g = green;
    b = blue;
  }

  void setSize(int sz) { size = sz; }
};

// ============================================================================
// 绘制的像素点数据
// ============================================================================

struct DrawnPixel {
  int x;
  int y;
  PixelAttribute attr;

  DrawnPixel() : x(0), y(0) {}
  DrawnPixel(int px, int py, const PixelAttribute &a) : x(px), y(py), attr(a) {}
};

// ============================================================================
// UI抽象基类
// ============================================================================

class DrawLangUI {
public:
  virtual ~DrawLangUI() = default;

  // ========================================================================
  // 生命周期管理
  // ========================================================================

  // 初始化UI（创建窗口等）
  virtual bool
  initialize(int width = 800, int height = 600,
             const std::string &title = "Draw Language Interpreter") = 0;

  // 关闭UI
  virtual void shutdown() = 0;

  // 准备绘图（设置文件路径等）
  virtual int prepare(const std::string &filePath) {
    sourceFilePath_ = filePath;
    return 0;
  }

  // ========================================================================
  // 主循环
  // ========================================================================

  // UI是否应该继续运行
  virtual bool shouldContinue() const = 0;

  // 处理一帧（事件处理、渲染等）
  virtual void processFrame() = 0;

  // 运行主循环（阻塞直到窗口关闭）
  virtual void run() = 0;

  // ========================================================================
  // 绘图接口
  // ========================================================================

  // 绘制一个像素点
  virtual void drawPixel(int x, int y, const PixelAttribute &attr) = 0;

  // 清除画布
  virtual void clearCanvas() = 0;

  // 刷新显示
  virtual void refresh() = 0;

  // ========================================================================
  // 消息接口
  // ========================================================================

  // 显示消息（flag: 0=普通信息, 1=错误信息）
  virtual void showMessage(int flag, const std::string &msg) = 0;

  // 显示状态信息
  virtual void setStatus(const std::string &status) = 0;

  // ========================================================================
  // 文件选择
  // ========================================================================

  // 选择源文件
  virtual std::string selectFile() = 0;

  // 获取当前源文件路径
  const std::string &getSourceFilePath() const { return sourceFilePath_; }

  // ========================================================================
  // 解释器回调
  // ========================================================================

  // 是否由UI调用解释器（用于异步模式）
  virtual bool callInterpreterByUI() const { return false; }

  // 设置解释执行回调
  using InterpretCallback = std::function<void(const std::string &)>;
  void setInterpretCallback(InterpretCallback callback) {
    interpretCallback_ = std::move(callback);
  }

  // ========================================================================
  // 画布信息
  // ========================================================================

  virtual int getCanvasWidth() const = 0;
  virtual int getCanvasHeight() const = 0;

protected:
  std::string sourceFilePath_;
  InterpretCallback interpretCallback_;
};

// ============================================================================
// UI包装类（全局访问点）
// ============================================================================

class DrawLangUIManager {
public:
  // 获取单例实例
  static DrawLangUIManager &getInstance();

  // 设置当前UI对象
  void setUI(DrawLangUI *ui);

  // 获取当前UI对象
  DrawLangUI *getUI() const;

  // 便捷绘图接口
  void drawPixel(int x, int y, const PixelAttribute &attr);
  void showMessage(int flag, const std::string &msg);
  void clearCanvas();
  void refresh();

  // 禁止拷贝
  DrawLangUIManager(const DrawLangUIManager &) = delete;
  DrawLangUIManager &operator=(const DrawLangUIManager &) = delete;

private:
  DrawLangUIManager() = default;
  DrawLangUI *currentUI_ = nullptr;
};

// 便捷的全局函数
inline DrawLangUIManager &getUIManager() {
  return DrawLangUIManager::getInstance();
}

} // namespace ui
} // namespace interpreter_exp
