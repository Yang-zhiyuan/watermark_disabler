#include <utility>
#include "mem_scan.hpp"
#include "mem_iter.hpp"
#include "mem_util.hpp"

template <typename ...Args>
__forceinline constexpr void output_to_console( const char* str, Args&&... args )
{
	DbgPrintEx( 77, 0, str, std::forward<Args>( args )... );
}

__forceinline constexpr void output_appended( const char* str )
{
	output_to_console( "[!] watermark_disabler: %s\n", str );
}

NTSTATUS driver_entry( )
{
	output_appended( "loaded" );

	const auto win32kfull_info = impl::search_for_module( "win32kfull.sys" );

	if ( !win32kfull_info )
	{
		output_appended( "failed to find the win32kfull.sys module" );
		return STATUS_UNSUCCESSFUL;
	}

	output_to_console( "[!] watermark_disabler: win32kfull.sys: 0x%p\n", win32kfull_info->image_base );

	/* gpsi = global pointer server info, a struct used to hold information about the system that is used by official windows kernel modules */
	static constexpr const char gpsi_signature[] = "48 8B 0D ? ? ? ? 48 8B 05 ? ? ? ? 0F BA 30 0C";
	const auto gpsi_instruction = FIND_SIGNATURE( static_cast< std::uint8_t* >( win32kfull_info->image_base ), gpsi_signature );

	if ( !gpsi_instruction )
	{
		output_appended( "failed to find gpsi, signature outdated?" );
		return STATUS_UNSUCCESSFUL;
	}

	output_to_console( "[!] watermark_disabler: mov gpsi instruction: 0x%p\n", gpsi_instruction );

	const auto gpsi = *reinterpret_cast< std::uint64_t* >( impl::resolve_mov<std::uint64_t>( gpsi_instruction ) );

	output_to_console( "[!] watermark_disabler: gpsi address: 0x%p\n", gpsi );

	/* std::uint32_t gpsi + 0x874: should render watermark? */
	*reinterpret_cast< std::uint32_t* >( gpsi + 0x874 ) = 0ui32;

	output_appended( "watermark disabled!" );

	return STATUS_SUCCESS;
}