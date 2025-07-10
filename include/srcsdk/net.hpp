#pragma once
#include <cstdint>
#include <Windows.h>

namespace ssdk {
	namespace net {
		struct netadr_s
		{
			enum netadrtype_t : std::int32_t
			{
				NA_NULL = 0,
				NA_LOOPBACK,
				NA_BROADCAST,
				NA_IP,
			};
			netadrtype_t	type;
			unsigned char	ip[4];
			unsigned short	port;
		};
		using netadr_t = netadr_s;

		struct bf_read {
			char pad[0x24];
		};

		struct netpacket_s // sizeof=0x50
		{                                       // XREF: netpacket_t/r
			netadr_t from;                      // XREF: CHLTVServer::ReadCompeleteDemoFile(void)+1A4/r
												// CHLTVServer::ReadCompeleteDemoFile(void)+415/r
			int source;
			double received;
			unsigned char *data;
			bf_read message;
			int size;
			int wiresize;
			bool stream;
			// padding byte
			// padding byte
			// padding byte
			netpacket_s *pNext;
		};
		using netpacket_t = netpacket_s;
		static_assert(sizeof(netpacket_s) == 0x50 );

		struct INetMessage // sizeof=0x4
		{                                       // XREF: CNetMessage/r
												// CNetMessage_0/r
			int (**_vptr_INetMessage)(...);     // XREF: CBaseClient::DisconnectSplitScreenUser(CBaseClient*)+46/w
												// CBaseClient::ProcessSplitPlayerConnect(CLC_SplitPlayerConnect *)+6A/w ...
		};

		using INetChannel = void;
		using CRC32_t = unsigned;

		struct CNetMessage : INetMessage // sizeof=0xC
		{                                       // XREF: SVC_Prefetch/r
												// SVC_ClassInfo/r ...
			bool m_bReliable;                   // XREF: CBaseClient::DisconnectSplitScreenUser(CBaseClient*)+36/w
												// CBaseClient::ProcessSplitPlayerConnect(CLC_SplitPlayerConnect *)+5D/w ...
			// padding byte
			// padding byte
			// padding byte
			INetChannel *m_NetChannel;          // XREF: CBaseClient::DisconnectSplitScreenUser(CBaseClient*)+3E/w
												// CBaseClient::ProcessSplitPlayerConnect(CLC_SplitPlayerConnect *)+62/w ...
		};
		static_assert(sizeof(CNetMessage) == 0xC );
		struct INetMessageHandler // sizeof=0x4
		{                                       // XREF: IServerMessageHandler/r
												// IClientMessageHandler/r ...
			int (**_vptr_INetMessageHandler)(...);
		};


		struct IClientMessageHandler : INetMessageHandler // sizeof=0x4
		{                                       // XREF: CBaseClient/r
		};
		struct IServerMessageHandler : INetMessageHandler // sizeof=0x4
		{                                       // XREF: CBaseClientState/r
		};

		static_assert(sizeof(INetMessageHandler) == 0x4 );
		static_assert(sizeof(INetMessageHandler) == 0x4 );

		struct bf_write // sizeof=0x18
		{                                       // XREF: CCachedReply/r
												// SVC_EntityMessage/r ...
			unsigned char *m_pData;           // XREF: CServerMsg_CheckReservation::SendMsg(netadr_s &,int,uint)+B3/r
												// CServerMsg_Ping::SendMsg(netadr_s &,int,uint)+D1/r ...
			int m_nDataBytes;                   // XREF: CBaseServer::ProcessConnectionlessPacket(netpacket_s *)+4A3/w
												// CBaseServer::ProcessConnectionlessPacket(netpacket_s *):loc_B60F7/w ...
			int m_nDataBits;                    // XREF: CBaseClientState::SendConnectPacket(netadr_s const&,int,int,ulong long,bool)+2A6/r
												// CBaseServer::ReplyReservationCheckRequest(netadr_s &,bf_read &)+135/r ...
			int m_iCurBit;                      // XREF: CBaseClient::SendSnapshot(CClientFrame *)+1E6/r
												// CBaseClient::SendSnapshot(CClientFrame *)+23F/r ...
			bool m_bOverflow;                   // XREF: CBaseClient::SendSnapshot(CClientFrame *):loc_9CB44/r
												// CBaseClient::SendServerInfo(void)+56F/r ...
			bool m_bAssertOnOverflow;
			// padding byte
			// padding byte
			const char *m_pDebugName;           // XREF: SV_CreateBaseline(void)+5F/w
		};
		static_assert(sizeof(bf_write) == 0x18);
		struct ISteamSocketMgr {
			enum ESteamCnxType : int32_t
			{                                       // XREF: _ZN16CBaseClientState14CheckForResendEb/r
				ESCT_NEVER = 0x0,
				ESCT_ASBACKUP = 0x1,
				ESCT_ALWAYS = 0x2,
				ESCT_MAXTYPE = 0x3,
			};

			virtual void Init();
			virtual void Shutdown();
			virtual ESteamCnxType GetCnxType();
			virtual void OpenSocket(int s, int nModule, int nSetPort, int nDefaultPort, const char *pName, int nProtocol, bool bTryAny);
			virtual void CloseSocket(int nModule);
			virtual int sendto(int s, const char *buf, int len, int flags, const sockaddr *to, int tolen);
			virtual int recvfrom(int s, char *buf, int len, int flags, sockaddr *from, int *fromlen);
			virtual uint64_t GetSteamIDForRemote(netadr_t* const remote);
		};
	}	
}