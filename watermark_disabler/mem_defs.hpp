#pragma once
#include <cstdint>
#include <ntifs.h>

namespace nt
{
    enum system_info_class
    {
        system_module_information = 11ui32,
    };

    struct rtl_module_info
    {
        HANDLE section;
        PVOID mapped_base;
        PVOID image_base;
        ULONG image_size;
        ULONG image_flags;
        USHORT load_order_idx;
        USHORT init_order_idx;
        USHORT load_count;
        USHORT file_name_offset;
        UCHAR full_path[ 256 ];
    };

    struct rtl_modules
    {
        ULONG count;
        rtl_module_info modules[ 1 ];
    };

    struct image_optional_header
    {
        char pad_0[ 0x38 ];
        std::uint32_t size_of_image;
    };

    struct image_nt_headers
    {
        char pad_0[ 0x18 ];
        image_optional_header optional_header;
    };

    struct image_dos_header
    {
        char pad_0[ 0x3c ];
        std::int32_t e_lfanew;
    };
}