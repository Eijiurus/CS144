#include "wrapping_integers.hh"
#include "debug.hh"
#include <cstdint>

using namespace std;

Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{
  return Wrap32( static_cast<uint32_t>( n ) + zero_point.raw_value_ );
}

uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{
  uint64_t MOD = ( 1ULL << 32 );

  uint64_t offset = static_cast<uint32_t>( raw_value_ - zero_point.raw_value_ );
  uint64_t base = checkpoint / MOD * MOD;
  auto candidate = base + offset;

  if ( candidate > checkpoint && candidate >= MOD && candidate - checkpoint > MOD / 2 ) {
    candidate -= MOD;
  } else if ( checkpoint > candidate && checkpoint - candidate > MOD / 2 ) {
    candidate += MOD;
  }

  return candidate;
}
