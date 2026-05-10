#include "tcp_receiver.hh"
#include "debug.hh"
#include "wrapping_integers.hh"
#include <cstdint>
#include <optional>

using namespace std;

void TCPReceiver::receive( TCPSenderMessage message )
{
  if ( message.RST ) {
    reassembler_.reader().set_error();
    return;
  }

  if ( message.SYN && !isn_.has_value() ) {
    isn_ = message.seqno;
  }

  if ( !isn_.has_value() ) {
    return;
  }

  auto checkpoint = reassembler_.writer().bytes_pushed() + 1;
  auto abs_seqno = message.seqno.unwrap( isn_.value(), checkpoint );
  auto first_index = abs_seqno - ( message.SYN ? 0 : 1 );

  reassembler_.insert( first_index, message.payload, message.FIN );
}

TCPReceiverMessage TCPReceiver::send() const
{
  std::optional<Wrap32> ackno;

  if ( isn_.has_value() ) {
    uint64_t abs_ack = reassembler_.writer().bytes_pushed() + 1;
    abs_ack += reassembler_.writer().is_closed();
    ackno = Wrap32::wrap( abs_ack, isn_.value() );
  }

  uint16_t windows_size = std::min<uint64_t>( reassembler_.writer().available_capacity(), UINT16_MAX );

  return { ackno, windows_size, reassembler_.reader().has_error() };
}
