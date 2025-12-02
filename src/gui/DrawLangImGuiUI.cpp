// 文件：DrawLangImGuiUI.cpp
// 内容：基于ImGui的Draw语言UI实现
// 使用GLFW + OpenGL3后端

#include "DrawLangImGuiUI.hpp"
#include <algorithm>
#include <cstring>
#include <filesystem>
#include <iostream>

namespace interpreter_exp {
namespace ui {

// ============================================================================
// GLFW错误回调
// ============================================================================

static void glfw_error_callback(int error, const char *description) {
  std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}

// ============================================================================
// DrawLangImGuiUI 实现
// ============================================================================

DrawLangImGuiUI::DrawLangImGuiUI() {
  // 初始化画布数据
  canvasData_.resize(canvasWidth_ * canvasHeight_ * 4, 255); // 白色背景
}

DrawLangImGuiUI::~DrawLangImGuiUI() { shutdown(); }

bool DrawLangImGuiUI::initialize(int width, int height,
                                 const std::string &title) {
  if (initialized_) {
    return true;
  }

  windowWidth_ = width;
  windowHeight_ = height;
  windowTitle_ = title;

  // 初始化GLFW
  glfwSetErrorCallback(glfw_error_callback);
  if (!glfwInit()) {
    std::cerr << "Failed to initialize GLFW" << std::endl;
    return false;
  }

  // GL 3.0 + GLSL 130
  const char *glsl_version = "#version 130";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

  // 创建窗口
  window_ = glfwCreateWindow(windowWidth_, windowHeight_, windowTitle_.c_str(),
                             nullptr, nullptr);
  if (window_ == nullptr) {
    std::cerr << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return false;
  }

  glfwMakeContextCurrent(window_);
  glfwSwapInterval(1); // 启用垂直同步

  // 初始化ImGui
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

  // 设置ImGui样式
  ImGui::StyleColorsDark();

  // 设置缩放
  float scale =
      ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor());
  ImGuiStyle &style = ImGui::GetStyle();
  style.ScaleAllSizes(scale);

  // 初始化ImGui后端
  ImGui_ImplGlfw_InitForOpenGL(window_, true);
  ImGui_ImplOpenGL3_Init(glsl_version);

  // 创建画布纹理
  glGenTextures(1, &canvasTexture_);
  glBindTexture(GL_TEXTURE_2D, canvasTexture_);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, canvasWidth_, canvasHeight_, 0,
               GL_RGBA, GL_UNSIGNED_BYTE, canvasData_.data());

  initialized_ = true;
  setStatus("Initialized - Ready to load Draw language file");
  showMessage(0, "Draw Language Interpreter initialized successfully.");

  return true;
}

void DrawLangImGuiUI::shutdown() {
  if (!initialized_) {
    return;
  }

  // 删除纹理
  if (canvasTexture_ != 0) {
    glDeleteTextures(1, &canvasTexture_);
    canvasTexture_ = 0;
  }

  // 清理ImGui
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  // 清理GLFW
  if (window_) {
    glfwDestroyWindow(window_);
    window_ = nullptr;
  }
  glfwTerminate();

  initialized_ = false;
}

bool DrawLangImGuiUI::shouldContinue() const {
  return initialized_ && window_ && !glfwWindowShouldClose(window_);
}

void DrawLangImGuiUI::processFrame() {
  if (!initialized_ || !window_) {
    return;
  }

  // 处理事件
  glfwPollEvents();

  // 处理最小化
  if (glfwGetWindowAttrib(window_, GLFW_ICONIFIED)) {
    ImGui_ImplGlfw_Sleep(10);
    return;
  }

  // 开始新帧
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  // 渲染UI
  renderUI();

  // 渲染
  ImGui::Render();
  int display_w, display_h;
  glfwGetFramebufferSize(window_, &display_w, &display_h);
  glViewport(0, 0, display_w, display_h);
  glClearColor(clearColor_.x, clearColor_.y, clearColor_.z, clearColor_.w);
  glClear(GL_COLOR_BUFFER_BIT);
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

  glfwSwapBuffers(window_);

  // 检查是否需要执行解释
  if (needInterpret_ && interpretCallback_) {
    needInterpret_ = false;
    isRunning_ = true;
    setStatus("Running...");
    interpretCallback_(sourceFilePath_);
    isRunning_ = false;
    setStatus("Completed");
  }
}

void DrawLangImGuiUI::run() {
  while (shouldContinue()) {
    processFrame();
  }
}

void DrawLangImGuiUI::renderUI() {
  // 主菜单栏
  if (ImGui::BeginMainMenuBar()) {
    if (ImGui::BeginMenu("File")) {
      if (ImGui::MenuItem("Open...", "Ctrl+O")) {
        showFileDialog_ = true;
      }
      if (ImGui::MenuItem("Clear Canvas")) {
        clearCanvas();
      }
      ImGui::Separator();
      if (ImGui::MenuItem("Exit", "Alt+F4")) {
        glfwSetWindowShouldClose(window_, true);
      }
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Run")) {
      if (ImGui::MenuItem("Execute", "F5", false,
                          !sourceFilePath_.empty() && !isRunning_)) {
        needInterpret_ = true;
      }
      ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
  }

  // 渲染各个面板
  renderControlPanel();
  renderCanvas();
  renderMessageLog();

  // 文件对话框
  if (showFileDialog_) {
    renderFileDialog();
  }
}

void DrawLangImGuiUI::renderCanvas() {
  ImGui::SetNextWindowPos(ImVec2(250, 30), ImGuiCond_FirstUseEver);
  ImGui::SetNextWindowSize(ImVec2(820, 620), ImGuiCond_FirstUseEver);

  ImGui::Begin("Canvas", nullptr, ImGuiWindowFlags_NoCollapse);

  // 更新画布纹理（如果需要）
  if (canvasDirty_) {
    updateCanvasTexture();
    canvasDirty_ = false;
  }

  // 显示画布
  ImVec2 canvasSize((float)canvasWidth_, (float)canvasHeight_);
  ImVec2 availSize = ImGui::GetContentRegionAvail();

  // 计算缩放以适应窗口
  float scaleX = availSize.x / canvasSize.x;
  float scaleY = availSize.y / canvasSize.y;
  float scale = std::min(scaleX, scaleY);

  ImVec2 displaySize(canvasSize.x * scale, canvasSize.y * scale);

  // 居中显示
  ImVec2 cursorPos = ImGui::GetCursorPos();
  cursorPos.x += (availSize.x - displaySize.x) * 0.5f;
  cursorPos.y += (availSize.y - displaySize.y) * 0.5f;
  ImGui::SetCursorPos(cursorPos);

  ImGui::Image((ImTextureID)(intptr_t)canvasTexture_, displaySize);

  ImGui::End();
}

void DrawLangImGuiUI::renderControlPanel() {
  ImGui::SetNextWindowPos(ImVec2(10, 30), ImGuiCond_FirstUseEver);
  ImGui::SetNextWindowSize(ImVec2(230, 300), ImGuiCond_FirstUseEver);

  ImGui::Begin("Control Panel", nullptr, ImGuiWindowFlags_NoCollapse);

  // 文件信息
  ImGui::Text("Source File:");
  if (sourceFilePath_.empty()) {
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "(No file loaded)");
  } else {
    std::filesystem::path p(sourceFilePath_);
    ImGui::TextWrapped("%s", p.filename().string().c_str());
  }

  ImGui::Separator();

  // 控制按钮
  if (ImGui::Button("Open File...", ImVec2(-1, 0))) {
    showFileDialog_ = true;
  }

  if (ImGui::Button("Execute (F5)", ImVec2(-1, 0))) {
    if (!sourceFilePath_.empty() && !isRunning_) {
      needInterpret_ = true;
    }
  }

  if (ImGui::Button("Clear Canvas", ImVec2(-1, 0))) {
    clearCanvas();
  }

  ImGui::Separator();

  // 统计信息
  ImGui::Text("Statistics:");
  {
    std::lock_guard<std::mutex> lock(pixelMutex_);
    ImGui::Text("Pixels drawn: %zu", drawnPixels_.size());
  }
  ImGui::Text("Canvas: %dx%d", canvasWidth_, canvasHeight_);

  ImGui::Separator();

  // 状态
  ImGui::Text("Status:");
  if (isRunning_) {
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "%s",
                       statusText_.c_str());
  } else {
    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "%s",
                       statusText_.c_str());
  }

  ImGui::End();
}

void DrawLangImGuiUI::renderMessageLog() {
  ImGui::SetNextWindowPos(ImVec2(10, 340), ImGuiCond_FirstUseEver);
  ImGui::SetNextWindowSize(ImVec2(460, 500), ImGuiCond_FirstUseEver);

  ImGui::Begin("Message Log", nullptr, ImGuiWindowFlags_NoCollapse);

  if (ImGui::Button("Clear Log")) {
    messages_.clear();
  }
  ImGui::SameLine();
  ImGui::Text("(%zu messages)", messages_.size());

  ImGui::Separator();

  ImGui::BeginChild("LogScrollRegion", ImVec2(0, 0), false,
                    ImGuiWindowFlags_HorizontalScrollbar);

  for (const auto &msg : messages_) {
    if (msg.flag == 1) {
      ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "%s",
                         msg.text.c_str());
    } else {
      ImGui::TextWrapped("%s", msg.text.c_str());
    }
  }

  // 自动滚动到底部
  if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
    ImGui::SetScrollHereY(1.0f);
  }

  ImGui::EndChild();
  ImGui::End();
}

void DrawLangImGuiUI::renderFileDialog() {
  ImGui::OpenPopup("Open Draw Language File");

  ImVec2 center = ImGui::GetMainViewport()->GetCenter();
  ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
  ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);

  if (ImGui::BeginPopupModal("Open Draw Language File", &showFileDialog_,
                             ImGuiWindowFlags_NoResize)) {
    static char pathBuffer[512] = "";

    ImGui::Text("Enter file path:");
    ImGui::InputText("##filepath", pathBuffer, sizeof(pathBuffer));

    ImGui::Separator();

    // 显示一些预设路径
    ImGui::Text("Quick Access (testcase files):");

    const char *testFiles[] = {"../../asset/testcase/draw.txt",
                               "../../asset/testcase/draw2.txt",
                               "../../asset/testcase/TaiJi.txt"};

    for (const char *file : testFiles) {
      if (ImGui::Selectable(file)) {
        strncpy(pathBuffer, file, sizeof(pathBuffer) - 1);
      }
    }

    ImGui::Separator();

    if (ImGui::Button("OK", ImVec2(120, 0))) {
      sourceFilePath_ = pathBuffer;
      if (!sourceFilePath_.empty()) {
        showMessage(0, "Loaded file: " + sourceFilePath_);
        setStatus("File loaded - Press Execute to run");
      }
      showFileDialog_ = false;
      ImGui::CloseCurrentPopup();
    }

    ImGui::SameLine();

    if (ImGui::Button("Cancel", ImVec2(120, 0))) {
      showFileDialog_ = false;
      ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
  }
}

void DrawLangImGuiUI::drawPixel(int x, int y, const PixelAttribute &attr) {
  std::lock_guard<std::mutex> lock(pixelMutex_);

  // 记录像素点
  drawnPixels_.emplace_back(x, y, attr);

  // 绘制到画布数据
  int size = std::max(1, attr.size);
  int halfSize = size / 2;

  for (int dy = -halfSize; dy <= halfSize; ++dy) {
    for (int dx = -halfSize; dx <= halfSize; ++dx) {
      int px = x + dx;
      int py = y + dy;

      // 边界检查
      if (px >= 0 && px < canvasWidth_ && py >= 0 && py < canvasHeight_) {
        int idx = (py * canvasWidth_ + px) * 4;
        canvasData_[idx + 0] = attr.r;
        canvasData_[idx + 1] = attr.g;
        canvasData_[idx + 2] = attr.b;
        canvasData_[idx + 3] = 255;
      }
    }
  }

  canvasDirty_ = true;
}

void DrawLangImGuiUI::clearCanvas() {
  std::lock_guard<std::mutex> lock(pixelMutex_);

  // 清空像素记录
  drawnPixels_.clear();

  // 重置画布为白色
  for (int i = 0; i < canvasWidth_ * canvasHeight_; ++i) {
    canvasData_[i * 4 + 0] = 255;
    canvasData_[i * 4 + 1] = 255;
    canvasData_[i * 4 + 2] = 255;
    canvasData_[i * 4 + 3] = 255;
  }

  canvasDirty_ = true;
  showMessage(0, "Canvas cleared.");
}

void DrawLangImGuiUI::refresh() { canvasDirty_ = true; }

void DrawLangImGuiUI::updateCanvasTexture() {
  std::lock_guard<std::mutex> lock(pixelMutex_);

  glBindTexture(GL_TEXTURE_2D, canvasTexture_);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, canvasWidth_, canvasHeight_, GL_RGBA,
                  GL_UNSIGNED_BYTE, canvasData_.data());
}

void DrawLangImGuiUI::showMessage(int flag, const std::string &msg) {
  messages_.push_back({flag, msg});

  // 限制消息数量
  if (messages_.size() > 1000) {
    messages_.erase(messages_.begin(), messages_.begin() + 100);
  }
}

void DrawLangImGuiUI::setStatus(const std::string &status) {
  statusText_ = status;
}

std::string DrawLangImGuiUI::selectFile() {
  showFileDialog_ = true;
  // 注意：这是一个简化实现，实际的文件对话框需要阻塞等待用户选择
  // 在ImGui中通常使用异步方式处理
  return sourceFilePath_;
}

void DrawLangImGuiUI::setCanvasSize(int width, int height) {
  std::lock_guard<std::mutex> lock(pixelMutex_);

  canvasWidth_ = width;
  canvasHeight_ = height;
  canvasData_.resize(width * height * 4, 255);

  // 重新创建纹理
  if (canvasTexture_ != 0) {
    glBindTexture(GL_TEXTURE_2D, canvasTexture_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, canvasWidth_, canvasHeight_, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, canvasData_.data());
  }

  canvasDirty_ = true;
}

void DrawLangImGuiUI::setBackgroundColor(float r, float g, float b, float a) {
  clearColor_ = ImVec4(r, g, b, a);
}

// ============================================================================
// 工厂函数
// ============================================================================

std::unique_ptr<DrawLangUI> createImGuiUI() {
  return std::make_unique<DrawLangImGuiUI>();
}

} // namespace ui
} // namespace interpreter_exp
