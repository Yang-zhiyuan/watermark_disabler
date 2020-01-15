#pragma once
#include "mem_util.hpp"
#include "mem_iter.hpp"
#include "util_raii.hpp"
#include <intrin.h>

namespace impl
{
	std::uint8_t* search_for_signature( const nt::rtl_module_info* module, const char* signature, const char* signature_mask )
	{
		if ( !module )
			return nullptr;

		const auto module_start = reinterpret_cast< std::uint8_t* >( module->image_base );
		const auto module_size = module_start + module->image_size;

		/* iterate the entire module */
		for ( auto segment = module_start; segment < module_size; segment++ )
		{
			if ( [ & ]( const std::uint8_t* bytes ) -> bool
				 {
					 auto sig_as_bytes = reinterpret_cast< std::uint8_t* >( const_cast< char* >( signature ) );

						 /* iterate through validity of the mask, mask sz is essentially equal to the byte sequence specified in signature */
						 for ( ; *signature_mask; ++signature_mask, ++bytes, ++sig_as_bytes )
						 {
							 /* if the signature misk is 'x' ( a valid byte, not an always match / wildcard ), and the current byte is not equal to the byte in the sig, then break */
							 if ( *signature_mask == 'x' && *bytes != *sig_as_bytes )
								 return false;
						 }

					 return ( *signature_mask ) == 0;
				 }( segment )
					 )
				return segment;
		}

		return nullptr;
	}

	extern "C" NTSYSAPI PCHAR NTAPI PsGetProcessImageFileName( PEPROCESS );

	PEPROCESS search_for_process( const char* process_name )
	{
		const auto kernel_module_info = search_for_module( "ntoskrnl.exe" );

		if ( !kernel_module_info )
			return nullptr;

		/* we are scanning for a conditional jump, that jumps to a call to the unexported function that we want, so we follow the jump, then follow the call to get to the function. */
		const auto conditional_instruction = search_for_signature( kernel_module_info, "\x79\xdc\xe9", "xxx" );

		if ( !conditional_instruction )
			return nullptr;
		
		const auto call_instruction = follow_conditional_jump( conditional_instruction );

		if ( !call_instruction )
			return nullptr;

		const auto PsGetNextProcess = follow_call< PEPROCESS( __stdcall* )( PEPROCESS ) >( call_instruction );

		if ( !PsGetNextProcess )
			return nullptr;

		PEPROCESS previous_process = PsGetNextProcess( nullptr );

		while ( previous_process )
		{
			if ( !std::strcmp( PsGetProcessImageFileName( previous_process ), process_name ) )
				return previous_process;

			previous_process = PsGetNextProcess( previous_process );
		}

		return nullptr;
	}
}