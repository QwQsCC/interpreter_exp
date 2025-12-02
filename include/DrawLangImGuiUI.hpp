// 基于ImGui的Draw语言UI实现
// 使用GLFW + OpenGL3后端

#pragma once

#include "DrawLangUI.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <mutex>
#include <string>
#include <vector>

namespace interpreter_exp {
namespace ui {

// ============================================================================
// 基于ImGui的Draw语言UI实现
// ============================================================================

class DrawLangImGuiUI : public DrawLangUI {
public:
  DrawLangImGuiUI();
  ~DrawLangImGuiUI() override;

  // ========================================================================
  // 生命周期管理
  // ========================================================================

  bool
  initialize(int width = 800, int height = 600,
             const std::string &title = "Draw Language Interpreter") override;
  void shutdown() override;

  // ========================================================================
  // 主循环
  // ========================================================================

  bool shouldContinue() const override;
  void processFrame() override;
  void run() override;

  // ========================================================================
  // 绘图接口
  // ========================================================================

  void drawPixel(int x, int y, const PixelAttribute &attr) override;
  void clearCanvas() override;
  void refresh() override;

  // ========================================================================
  // 消息接口
  // ========================================================================

  void showMessage(int flag, const std::string &msg) override;
  void setStatus(const std::string &status) override;

  // ========================================================================
  // 文件选择
  // ========================================================================

  std::string selectFile() override;

  // ========================================================================
  // 画布信息
  // ========================================================================

  int getCanvasWidth() const override { return canvasWidth_; }
  int getCanvasHeight() const override { return canvasHeight_; }

  // ========================================================================
  // 配置
  // ========================================================================

  void setCanvasSize(int width, int height);
  void setBackgroundColor(float r, float g, float b, float a = 1.0f);

  // 是否由UI调用解释器
  bool callInterpreterByUI() const override { return true; }

private:
  // ========================================================================
  // 内部方法
  // ========================================================================

  void renderUI();
  void renderCanvas();
  void renderControlPanel();
  void renderMessageLog();
  void renderFileDialog();

  // 将像素数据转换为纹理
  void updateCanvasTexture();

  // ========================================================================
  // GLFW/OpenGL资源
  // ========================================================================

  GLFWwindow *window_ = nullptr;
  GLuint canvasTexture_ = 0;

  // ========================================================================
  // 画布数据
  // ========================================================================

  int canvasWidth_ = 800;
  int canvasHeight_ = 600;
  std::vector<unsigned char> canvasData_; // RGBA格式
  std::vector<DrawnPixel> drawnPixels_;   // 绘制的像素点记录
  bool canvasDirty_ = true;               // 画布是否需要更新纹理

  // ========================================================================
  // UI状态
  // ========================================================================

  int windowWidth_ = 1280;
  int windowHeight_ = 800;
  std::string windowTitle_ = "Draw Language Interpreter";
  ImVec4 clearColor_ = ImVec4(0.15f, 0.15f, 0.15f, 1.0f);
  ImVec4 canvasBgColor_ = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

  // ========================================================================
  // 消息日志
  // ========================================================================

  struct LogMessage {
    int flag; // 0=普通, 1=错误
    std::string text;
  };
  std::vector<LogMessage> messages_;
  std::string statusText_ = "Ready";

  // ========================================================================
  // 文件对话框状态
  // ========================================================================

  bool showFileDialog_ = false;
  std::string fileDialogPath_;
  std::string selectedFilePath_;
  bool fileSelected_ = false;

  // ========================================================================
  // 执行控制
  // ========================================================================

  bool needInterpret_ = false;
  bool isRunning_ = false;

  // ========================================================================
  // 线程安全
  // ========================================================================

  mutable std::mutex pixelMutex_;

  // ========================================================================
  // 初始化状态
  // ========================================================================

  bool initialized_ = false;
};

// ============================================================================
// 工厂函数
// ============================================================================

std::unique_ptr<DrawLangUI> createImGuiUI();

} // namespace ui
} // namespace interpreter_exp
