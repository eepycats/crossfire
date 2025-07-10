#pragma once
#include <cstdint>

namespace ssdk {
    namespace cvar {
        enum flags : int32_t {
            FCVAR_NONE                       = 0,
            
            FCVAR_UNREGISTERED              = (1 << 0),
            FCVAR_DEVELOPMENTONLY           = (1 << 1),
            FCVAR_GAMEDLL                   = (1 << 2),
            FCVAR_CLIENTDLL                 = (1 << 3),
            FCVAR_HIDDEN                    = (1 << 4),

            FCVAR_PROTECTED                 = (1 << 5),
            FCVAR_SPONLY                    = (1 << 6),
            FCVAR_ARCHIVE                   = (1 << 7),
            FCVAR_NOTIFY                    = (1 << 8),
            FCVAR_USERINFO                  = (1 << 9),

            FCVAR_PRINTABLEONLY             = (1 << 10),
            FCVAR_GAMEDLL_FOR_REMOTE_CLIENTS = (1 << 10),  // Alias of FCVAR_PRINTABLEONLY

            FCVAR_UNLOGGED                  = (1 << 11),
            FCVAR_NEVER_AS_STRING           = (1 << 12),

            FCVAR_REPLICATED                = (1 << 13),
            FCVAR_CHEAT                     = (1 << 14),
            FCVAR_SS                        = (1 << 15),
            FCVAR_DEMO                      = (1 << 16),
            FCVAR_DONTRECORD                = (1 << 17),
            FCVAR_SS_ADDED                  = (1 << 18),
            FCVAR_RELEASE                   = (1 << 19),
            FCVAR_RELOAD_MATERIALS          = (1 << 20),
            FCVAR_RELOAD_TEXTURES           = (1 << 21),

            FCVAR_NOT_CONNECTED             = (1 << 22),
            FCVAR_MATERIAL_SYSTEM_THREAD    = (1 << 23),
            FCVAR_ARCHIVE_GAMECONSOLE       = (1 << 24),

            FCVAR_ACCESSIBLE_FROM_THREADS   = (1 << 25),

            // 26, 27, 31 are reserved/available, not defined

            FCVAR_SERVER_CAN_EXECUTE        = (1 << 28),
            FCVAR_SERVER_CANNOT_QUERY       = (1 << 29),
            FCVAR_CLIENTCMD_CAN_EXECUTE     = (1 << 30),
        };

        struct ConCommandBase // sizeof=0x18
        {                                       // XREF: ConVar/r ConCommand/r
            int (**_vptr_ConCommandBase)(...);  // XREF: `global constructor keyed to'cl_bounded_cvars.cpp+188/w
                                                // `global constructor keyed to'cl_bounded_cvars.cpp+20A/w ...
            ConCommandBase *m_pNext;
            bool m_bRegistered;
            // padding byte
            // padding byte
            // padding byte
            const char *m_pszName;
            const char *m_pszHelpString;
            int m_nFlags;                       // XREF: CLog::Open(void)+81/r
                                                // CBaseServer::GetPassword(void)+17/r ...
        };
        struct ICVarIteratorInternal
        {
        public:
            virtual void		SetFirst( void ) = 0;
            virtual void		Next( void )  = 0;
            virtual	bool		IsValid( void ) = 0;
            virtual ConCommandBase *Get( void ) = 0;
        };
        //38        

    }
}