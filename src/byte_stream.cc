#include "byte_stream.hh"

using namespace std;

ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ) {}

void Writer::push( string data )
{
  auto len = std::min( available_capacity(), data.size() );
  if ( len == 0 ) {
    return;
  }
  data.resize( len );
  buffer_.emplace( std::move( data ) );
  if ( buffer_.size() == 1 ) {
    buffer_view_ = buffer_.back();
  }
  num_bytes_pushed_ += len;
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
  if ( buffer_.empty() ) {
    return {};
  } else {
    return buffer_view_;
  }
}

void Reader::pop( uint64_t len )
{
  while ( !buffer_.empty() && len > 0 ) {

    auto view_size = buffer_view_.size();

    if ( len >= view_size ) {
      len -= view_size;
      num_bytes_popped_ += view_size;
      buffer_.pop();
      if ( !buffer_.empty() )
        buffer_view_ = buffer_.front();
    } else {
      buffer_view_.remove_prefix( len );
      num_bytes_popped_ += len;
      len = 0;
    }
  }
}

bool Reader::is_finished() const
{
  return closed_ && ( num_bytes_popped_ == num_bytes_pushed_ );
}

uint64_t Reader::bytes_buffered() const
{
  return num_bytes_pushed_ - num_bytes_popped_;
}

uint64_t Reader::bytes_popped() const
{
  return num_bytes_popped_;
}
