#pragma once

namespace ssdk{
    namespace tier1 {
        template< class T, class I = int >
        struct CUtlMemory
        {                                       // XREF: CUtlVector<unsigned char,CUtlMemory<unsigned char,int> >/r
                                                // CUtlBuffer/r ...
            T *m_pMemory;         // XREF: Base_CmdKeyValues::WriteToBuffer(bf_write &)+12F/r
                                                // Base_CmdKeyValues::WriteToBuffer(bf_write &)+14B/r ...
            int m_nAllocationCount;             // XREF: Base_CmdKeyValues::ReadFromBuffer(bf_read &):loc_2131E/w
                                                // SVC_Menu::ReadFromBuffer(bf_read &):loc_291CA/w ...
            int m_nGrowSize;                    // XREF: Base_CmdKeyValues::WriteToBuffer(bf_write &):loc_1E742/r
                                                // SVC_Menu::WriteToBuffer(bf_write &):loc_1EC5D/r ...
        };

        struct CUtlBinaryBlock // sizeof=0x10
        {                                       // XREF: CUtlString/r
            CUtlMemory<unsigned char> m_Memory;
                                                // XREF: CAddressList::AddRemote(char const*,char const*)+10E/r
                                                // CAddressList::AddRemote(char const*,char const*)+115/r ...
            int m_nActualLength;                // XREF: CAddressList::AddRemote(char const*,char const*)+107/w
                                                // CAddressList::AddRemote(char const*,char const*):loc_A5D88/w ...
        };
        static_assert(sizeof(CUtlBinaryBlock) == 0x10 );

        struct CUtlString {
            const char *Get()
            {
                const char *result; // eax

                result = "";
                if ( this->m_Storage.m_nActualLength )
                    return (const char *)this->m_Storage.m_Memory.m_pMemory;
                return result;
            }
            CUtlBinaryBlock m_Storage;     
        };
        static_assert( sizeof(CUtlString) == 0x10);
    }
}
