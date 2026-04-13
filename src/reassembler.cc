#include "reassembler.hh"
#include "debug.hh"
#include <algorithm>

using namespace std;

void Reassembler::handle_truncation( uint64_t& first_index, string& data, bool is_last_substring )
{
  if ( is_last_substring ) {
    total_size_ = first_index + data.size();
  }

  auto first_unpopped_index = output_.reader().bytes_popped();
  auto first_unacceptable_index = first_unpopped_index + initial_capacity_;
  std::string_view data_view { data };

  if ( data.size() + first_index <= first_unassembled_index_ || first_index >= first_unacceptable_index ) {
    data.resize( 0 );
    return;
  }

  if ( first_index < first_unassembled_index_ ) {
    data_view.remove_prefix( first_unassembled_index_ - first_index );
    first_index = first_unassembled_index_;
  }
  data = std::string( data_view );
  if ( first_index + data.size() >= first_unacceptable_index ) {
    data.resize( first_unacceptable_index - first_index );
  }
}

void Reassembler::merge_into_map( uint64_t first_index, string data )
{
  auto L = pending_substrings_.upper_bound( first_index );

  if ( L != pending_substrings_.begin() ) {
    L = std::prev( L );

    if ( L->first <= first_index && L->second.size() + L->first >= first_index + data.size() ) {
      return;
    }

    if ( L->first <= first_index && L->second.size() + L->first >= first_index ) {
      auto L_data = L->second;
      L_data.resize( first_index - L->first );
      L_data.append( data );
      data = std::move( L_data );
      first_index = L->first;
      L = pending_substrings_.erase( L );
    } else {
      L++;
    }
  }

  while ( L != pending_substrings_.end() && L->first <= first_index + data.size() ) {
    auto L_data = L->second;
    if ( first_index + data.size() >= L->first + L_data.size() ) {
      L = pending_substrings_.erase( L );
    } else {
      data.resize( L->first - first_index );
      data.append( L_data );
      L = pending_substrings_.erase( L );
    }
  }
  pending_substrings_.emplace( first_index, data );
}

void Reassembler::push_to_output()
{
  while ( !pending_substrings_.empty() && pending_substrings_.begin()->first == first_unassembled_index_ ) {
    output_.writer().push( pending_substrings_.begin()->second );
    first_unassembled_index_ += pending_substrings_.begin()->second.size();
    pending_substrings_.erase( pending_substrings_.begin() );
  }

  if ( output_.writer().bytes_pushed() == total_size_ ) {
    output_.writer().close();
  }
}

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring )
{
  handle_truncation( first_index, data, is_last_substring );
  if ( data.empty() && !is_last_substring ) {
    return;
  }

  merge_into_map( first_index, data );

  push_to_output();

  return;
}

// How many bytes are stored in the Reassembler itself?
// This function is for testing only; don't add extra state to support it.
uint64_t Reassembler::count_bytes_pending() const
{
  uint64_t pending_size = 0;
  for ( const auto& [index, payload] : pending_substrings_ ) {
    pending_size += payload.size();
  }
  return pending_size;
}