#pragma once
#include "mem_defs.hpp"
#include <array>

namespace impl
{
	namespace masking
	{
		/* credits to: DefCon42 */

		constexpr std::uint8_t char_to_hex( char c )
		{
			return c <= '9' ? c - '0' : ( c | ' ' ) - 'a' + 10;
		}

		template<typename T, size_t N, size_t... Idx>
		constexpr std::array<T, N + 1> add_to_front( std::array<T, N> arr, T val, std::index_sequence<Idx...> )
		{
			return std::array<T, N + 1> { val, arr[ Idx ]... };
		}

		template<std::uint32_t C, std::uint32_t N,
			const char* Pattern,
			class Enable = void>
			struct ida_mask_helper_t
		{
			using next_mask_helper = ida_mask_helper_t<C + 2, N, Pattern>;
			constexpr static auto length = next_mask_helper::length + 1;

			constexpr static auto pattern = add_to_front<std::uint8_t>( next_mask_helper::pattern, char_to_hex( Pattern[ C ] ) * 16 + char_to_hex( Pattern[ C + 1 ] ), std::make_index_sequence<length - 1>( ) );
			constexpr static auto wildcard = add_to_front( next_mask_helper::wildcard, false, std::make_index_sequence<length - 1>( ) );
		};

		template<
			std::uint32_t C, std::uint32_t N,
			const char* Pattern>
			struct ida_mask_helper_t<C, N, Pattern, std::enable_if_t<C >= N>>
		{
			constexpr static auto length = 1;

			constexpr static std::array<std::uint8_t, 1> pattern{ 0 };
			constexpr static std::array<bool, 1> wildcard{ true };
		};

		template<
			std::uint32_t C, std::uint32_t N,
			const char* Pattern>
			struct ida_mask_helper_t<C, N, Pattern, std::enable_if_t<Pattern[ C ] == '?'>>
		{
			using next_mask_helper = ida_mask_helper_t<C + 1, N, Pattern>;
			constexpr static auto length = next_mask_helper::length + 1;

			constexpr static auto pattern = add_to_front( next_mask_helper::pattern, static_cast< std::uint8_t >( 0 ), std::make_index_sequence<length - 1>( ) );
			constexpr static auto wildcard = add_to_front( next_mask_helper::wildcard, true, std::make_index_sequence<length - 1>( ) );
		};

		template<
			std::uint32_t C, std::uint32_t N,
			const char* Pattern>
			struct ida_mask_helper_t<C, N, Pattern, std::enable_if_t<Pattern[ C ] == ' '>>
		{
			using next_mask_helper = ida_mask_helper_t<C + 1, N, Pattern>;
			constexpr static auto length = next_mask_helper::length;

			constexpr static auto pattern = next_mask_helper::pattern;
			constexpr static auto wildcard = next_mask_helper::wildcard;
		};

		template<std::uint32_t N, const char Pattern[ N ]>
		struct ida_mask_t
		{
			using value = ida_mask_helper_t<0, N - 1, Pattern>;
		};
	}

	template <typename Mask>
	constexpr std::uint8_t* find_signature( std::uint8_t* module_base )
	{
		const auto module_size = reinterpret_cast< nt::image_nt_headers* >( module_base + reinterpret_cast< nt::image_dos_header* >( module_base )->e_lfanew )->optional_header.size_of_image;

		auto signature = Mask::pattern;
		auto signature_len = Mask::length;
		auto wildcards = Mask::wildcard;

		for ( auto i = 0ul; i < module_size - signature_len; i++ )
		{
			auto found = true;

			for ( auto j = 0; j < signature_len; j++ )
			{
				if ( !wildcards[ j ] && signature[ j ] != module_base[ i + j ] )
				{
					found = false;
					break;
				}
			}

			if ( found )
				return module_base + i;
		}

		return nullptr;
	}
}

#define FIND_SIGNATURE( module_start, signature ) impl::find_signature<impl::masking::ida_mask_t<sizeof(signature), signature>::value>( module_start )