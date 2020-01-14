#pragma once
#include "mem_defs.hpp"

namespace impl
{
	template <typename T>
	__forceinline T follow_call( const std::uint8_t* address )
	{
		/* + 1 is the address of the calle, + 5 is the size of a call instruction */
		return reinterpret_cast< T >( address + *reinterpret_cast< std::int32_t* >( address + 1 ) + 5 );
	}

	template <typename T>
	__forceinline T resolve_mov( std::uint8_t* address )
	{
		/* + 3 is the address of the source, + 7 is the size of a mov instruction */
		return reinterpret_cast< T >( address + *reinterpret_cast< std::int32_t* >( address + 3 ) + 7 );
	}
}