#include "byte_stream.hh"

using namespace std;

ByteStream::ByteStream( uint64_t capacity )
  : capacity_( capacity ), 
    error_( false ), 
    closed_( false ), 
    num_bytes_pushed_( 0 ), 
    num_bytes_popped_( 0 ),
    buffer_(),       // 显式初始化队列
    buffer_view_()  // 显式初始化视图队列
{}

void Writer::push( string data )
{
  auto len = std::min(available_capacity(), data.size());
  if (len == 0) {
    return ;
  }
  
  buffer_.push(data.substr(0, len));
  buffer_view_.emplace(buffer_.back().c_str(), len);
  num_bytes_pushed_ += len;
  /*std::cerr << "[PUSH] 尝试写入长度: " << data.size() 
          << ", 实际写入长度: " << len 
          << ", 当前 buffer_view 大小: " << buffer_view_.size() << "\n";*/
}

void Writer::close()
{
  closed_ = true;
}

bool Writer::is_closed() const
{
  return closed_; 
}

uint64_t Writer::available_capacity() const
{
  return capacity_ - reader().bytes_buffered(); 
}

uint64_t Writer::bytes_pushed() const
{
  return num_bytes_pushed_;
}

string_view Reader::peek() const
{
  if (buffer_view_.empty()) {
    return {};
  }
  else {
    return buffer_view_.front();
  }
}

void Reader::pop( uint64_t len )
{
  while (!buffer_.empty() && len > 0) {

    auto view_size = buffer_view_.front().size();

    if (len >= view_size) {
      len -= view_size;
      num_bytes_popped_ += view_size;
      buffer_.pop();
      buffer_view_.pop();
    }
    else {
      buffer_view_.front().remove_prefix(len);
      num_bytes_popped_ += len;
      len = 0;
    }
  }
}

bool Reader::is_finished() const
{
  return (num_bytes_popped_ == num_bytes_pushed_) && (closed_ == true);
}

uint64_t Reader::bytes_buffered() const
{
  return num_bytes_pushed_ - num_bytes_popped_;
}

uint64_t Reader::bytes_popped() const
{
  return num_bytes_popped_;
}

