
#pragma once
#include <srcsdk/net.hpp>
#include <srcsdk/tier1.hpp>

namespace ssdk{

    namespace netmsg {
        using namespace ssdk::net;
        using namespace ssdk::tier1;

        struct SVC_UserMessage : CNetMessage // sizeof=0x64
        {                                       // XREF: MsgData/r
            void *m_pMessageHandler;
            int m_nMsgType;                     // XREF: CVEngineServer::Message_CheckMessageLength(void)+6C/r
                                                // CVEngineServer::UserMessageBegin(IRecipientFilter *,int,char const*)+83/w
            int m_nLength;
            bf_read m_DataIn;                   // XREF: `global constructor keyed to'vengineserver_impl.cpp+12B/w
                                                // `global constructor keyed to'vengineserver_impl.cpp+132/w ...
            bf_write  m_DataOut;                 // XREF: `global constructor keyed to'vengineserver_impl.cpp+150/o
                                                // CVEngineServer::Message_CheckMessageLength(void)+A7/r
            CUtlString m_sDebugName;            // XREF: `global constructor keyed to'vengineserver_impl.cpp+15E/o
                                                // `global constructor keyed to'vengineserver_impl.cpp+2FB/w ...
        };
        static_assert( sizeof(SVC_UserMessage) == 0x64 );

        struct SVC_ServerInfo : CNetMessage // sizeof=0x564
        {                                       // XREF: _ZN11CBaseClient14SendServerInfoEv/r
                                                // _ZN17CHLTVDemoRecorder15WriteServerInfoEv/r ...
            IServerMessageHandler *m_pMessageHandler;
            int m_nProtocol;
            int m_nServerCount;
            bool m_bIsDedicated;
            bool m_bIsHLTV;
            char m_cOS;
            // padding byte
            CRC32_t m_nMapCRC;
            CRC32_t m_nClientCRC;
            CRC32_t m_nStringTableCRC;
            int m_nMaxClients;
            int m_nMaxClasses;
            int m_nPlayerSlot;                  // XREF: CBaseClient::SendServerInfo(void)+256/w
            float m_fTickInterval;
            const char *m_szGameDir;
            const char *m_szMapName;
            const char *m_szMapGroupName;
            const char *m_szSkyName;
            const char *m_szHostName;
            uint32_t m_publicIP;                  // XREF: CBaseClient::SendServerInfo(void)+702/r
            char m_szGameDirBuffer[260];
            char m_szMapNameBuffer[260];
            char m_szMapGroupNameBuffer[260];
            char m_szSkyNameBuffer[260];
            char m_szHostNameBuffer[260];
        };
        static_assert(sizeof(SVC_ServerInfo) == 0x564 );


    }
}
