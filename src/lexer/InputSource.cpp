// 输入源抽象类的实现

#include "lexer.hpp"

namespace interpreter_exp {
namespace lexer {

// ============================================================================
// StringInputSource 实现
// ============================================================================

char StringInputSource::nextChar() {
  if (position_ >= source_.length()) {
    return '\0'; // EOF
  }

  char c = source_[position_++];

  // 更新位置信息
  if (c == '\n') {
    location_.line++;
    location_.column = 1;
  } else {
    location_.column++;
  }
  location_.position = position_;

  return c;
}

char StringInputSource::peekChar() const {
  if (position_ >= source_.length()) {
    return '\0'; // EOF
  }
  return source_[position_];
}

void StringInputSource::ungetChar() {
  if (position_ > 0) {
    position_--;

    // 回退位置信息
    char c = source_[position_];
    if (c == '\n') {
      // 需要找到上一行的列号
      location_.line--;
      // 计算上一行的列号
      size_t lineStart = position_;
      while (lineStart > 0 && source_[lineStart - 1] != '\n') {
        lineStart--;
      }
      location_.column = position_ - lineStart + 1;
    } else {
      location_.column--;
    }
    location_.position = position_;
  }
}

// ============================================================================
// FileInputSource 实现
// ============================================================================

FileInputSource::FileInputSource(const std::string &filePath,
                                 const std::string &sourceId)
    : filePath_(filePath), sourceId_(sourceId.empty() ? filePath : sourceId) {
  location_.filename = sourceId_;
  location_.line = 1;
  location_.column = 1;
  location_.position = 0;

  file_.open(filePath, std::ios::in);
  if (!file_.is_open()) {
    throw std::runtime_error("Failed to open file: " + filePath);
  }
}

FileInputSource::~FileInputSource() {
  if (file_.is_open()) {
    file_.close();
  }
}

char FileInputSource::nextChar() {
  if (!file_.is_open() || file_.eof()) {
    return '\0';
  }

  // 保存当前位置到历史记录（用于ungetChar）
  history_.push_back(location_);

  int c = file_.get();
  if (c == EOF) {
    history_.pop_back(); // 回滚历史
    return '\0';
  }

  // 更新位置信息
  if (c == '\n') {
    location_.line++;
    location_.column = 1;
  } else {
    location_.column++;
  }
  location_.position++;

  return static_cast<char>(c);
}

char FileInputSource::peekChar() const {
  if (!file_.is_open() || file_.eof()) {
    return '\0';
  }

  int c = file_.peek();
  if (c == EOF) {
    return '\0';
  }
  return static_cast<char>(c);
}

void FileInputSource::ungetChar() {
  if (!file_.is_open() || history_.empty()) {
    return;
  }

  // 回退文件流位置
  file_.unget();

  // 恢复位置信息
  location_ = history_.back();
  history_.pop_back();
}

bool FileInputSource::eof() const {
  if (!file_.is_open()) {
    return true;
  }
  // 检查是否已到文件末尾
  return file_.peek() == EOF;
}

// ============================================================================
// 工厂函数
// ============================================================================

std::unique_ptr<InputSource>
createStringInputSource(const std::string &source,
                        const std::string &sourceId) {
  return std::make_unique<StringInputSource>(source, sourceId);
}

std::unique_ptr<InputSource>
createFileInputSource(const std::string &filename) {
  return std::make_unique<FileInputSource>(filename);
}

} // namespace lexer
} // namespace interpreter_exp
