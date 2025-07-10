

#include <algorithm>
#include <bit>
#include <cstdint>
#include <mem.hpp>
#include <safetyhook.hpp>
#include <init.hpp>
#include <array>
#include <lru.hpp>
#include <asmjit/x86.h>
#include <global.hpp>
#include <srcsdk/sdk.hpp>

using namespace ssdk::net;
using namespace ssdk::netmsg;
using namespace mem;
using namespace global;

struct ip_address{
	uint8_t octets[4];
	uint16_t port;
	bool operator==(const ip_address& other) const {
		return std::memcmp(this, &other, sizeof(ip_address)) == 0;
	}
};

namespace std {
    template <>
    struct hash<ip_address> {
        size_t operator()(const ip_address& ip) const noexcept {
            const char* data = reinterpret_cast<const char*>(&ip);
            return std::hash<std::string_view>{}(std::string_view(data, sizeof(ip_address)));
        }
    };
}

LRUCache<ip_address,bool> address_cache(4096);


ip_address v4_to_ip_addr(const sockaddr* sa) {
    if (sa->sa_family == AF_INET) {
        const sockaddr_in* sa_in = reinterpret_cast<const sockaddr_in*>(sa);
		ip_address addr;
        uint32_t ip_net_order = sa_in->sin_addr.s_addr;
        uint8_t octets[4];
        addr.octets[0] = (ip_net_order      ) & 0xFF;
        addr.octets[1] = (ip_net_order >> 8 ) & 0xFF;
        addr.octets[2] = (ip_net_order >> 16) & 0xFF;
        addr.octets[3] = (ip_net_order >> 24) & 0xFF;
        addr.port = std::byteswap(sa_in->sin_port);
		return addr;
    }
    else {
        throw std::exception("what the fuck, not v4?");
    }
}

//
bool g_is_next_xbox = false;

ISteamSocketMgr* g_SteamSocketMgr = reinterpret_cast<ISteamSocketMgr*>(rva<"engine">(0x6AA338));


static std::array<uint8_t, 5> connect_hdr = {0xff,0xff,0xff,0xff,0x71};
int __fastcall hooked_recvfrom(void* ecx, void* edx, int s, char *buf, int len, int flags, sockaddr *from, int *fromlen) {
	auto ret = g_SteamSocketMgr->recvfrom(s, buf, len, flags, from, fromlen);

	if (ret <= 0){
		return ret;
	}

	auto addr = v4_to_ip_addr(from);

	if (ret >= 7){
		uint8_t* xb_data = reinterpret_cast<uint8_t*>(buf) + 2;
		std::span<uint8_t> xb_span{xb_data,5};

		uint8_t* data = reinterpret_cast<uint8_t*>(buf);
		std::span<uint8_t> data_span{data,5};

		if (std::equal(xb_span.begin(),xb_span.end(), connect_hdr.begin()) ) {
			ConColorMsg({218,177,218, 255}, "[net] xbox connect from %i.%i.%i.%i:%i\n", +addr.octets[0],+addr.octets[1],+addr.octets[2],+addr.octets[3],+addr.port);
			address_cache.put(addr, true);	
		} else if (std::equal(data_span.begin(), data_span.end(), connect_hdr.begin())) {
			ConColorMsg({218,177,218, 255}, "[net] regular/ps3 connect from %i.%i.%i.%i:%i\n", +addr.octets[0],+addr.octets[1],+addr.octets[2],+addr.octets[3],+addr.port);
			address_cache.put(addr, false);				
		}
	}
	bool is_xbox = false;

	bool has_key = address_cache.get(addr, is_xbox);
	if (!has_key){
		ConColorMsg({255,0,0,255}, "hooked_recvfrom: no address in cache: %i.%i.%i.%i:%i\n",
					+addr.octets[0], +addr.octets[1], +addr.octets[2], +addr.octets[3], +addr.port);			
		g_is_next_xbox = false;
		return ret;
	} else {
		address_cache.touch(addr);
		g_is_next_xbox = is_xbox;
		return ret;
	}
}



bool* g_is_xbox = reinterpret_cast<bool*>(rva<"engine">(0x6AA461));
safetyhook::InlineHook g_NET_SendTo;

safetyhook::MidHook g_NET_ReceiveDatagram_mid;
void NET_RecvDgram_mid(safetyhook::Context& ctx){
	if (!g_is_next_xbox) return;
	//.text:1009BD5F 8B 4F 18                    mov     ecx, [edi+18h]
	auto shi = ctx.edi + 0x18;
	auto abysmal = reinterpret_cast<uint16_t**>(shi);
	**abysmal = std::byteswap(**abysmal); 
}


DWORD __cdecl NET_SendToImpl(SOCKET s, const char *buf, int len, const sockaddr *to, int tolen, int iGameDataLength)
{
	bool is_xbox = false;
	auto addr = v4_to_ip_addr(to);
	bool has_address = address_cache.get(addr, is_xbox);
	if ( !has_address ){
	ConColorMsg({255,0,0,255}, "NET_SendToImpl: no address in cache: %i.%i.%i.%i:%i\n",
				+addr.octets[0], +addr.octets[1], +addr.octets[2], +addr.octets[3], +addr.port);		
		return g_SteamSocketMgr->sendto(s, buf, len, 0, to, tolen);
	} else {
		address_cache.touch(addr);
		//ConColorMsg({218,177,218, 255}, "[net] sendto %i.%i.%i.%i:%i, xbox: %s\n", +addr.octets[0],+addr.octets[1],+addr.octets[2],+addr.octets[3],+addr.port, is_xbox ? "true" : "false");

		if (!is_xbox){
			return g_SteamSocketMgr->sendto(s, buf, len, 0, to, tolen);
		} else {
			uint16_t xbox_hdr = 0;
			xbox_hdr = std::byteswap(static_cast<uint16_t>(len));
			std::vector<char> data(len+2);
			std::copy_n(reinterpret_cast<char*>(&xbox_hdr),sizeof(xbox_hdr), data.begin());
			std::copy_n(buf,len,data.begin()+2);
			return g_SteamSocketMgr->sendto(s, data.data(), data.size(), 0, to, tolen);
		}
	}
	
}

struct my_socket_mgr : public ISteamSocketMgr{
	virtual void Init() {
		g_SteamSocketMgr->Init();
	};
	virtual void Shutdown() {
		g_SteamSocketMgr->Shutdown();
	};
	virtual ESteamCnxType GetCnxType() {
		return g_SteamSocketMgr->GetCnxType();
	};
	virtual void OpenSocket(int s, int nModule, int nSetPort, int nDefaultPort, const char *pName, int nProtocol, bool bTryAny) {
		return g_SteamSocketMgr->OpenSocket(s, nModule, nSetPort, nDefaultPort, pName, nProtocol, bTryAny);
	};
	virtual void CloseSocket(int nModule) {
		g_SteamSocketMgr->CloseSocket(nModule);
	};
	virtual int sendto(int s, const char *buf, int len, int flags, const sockaddr *to, int tolen){
		return g_SteamSocketMgr->sendto( s, buf, len, flags, to, tolen);
	};
	virtual int recvfrom(int s, char *buf, int len, int flags, sockaddr *from, int *fromlen){
		return hooked_recvfrom(this, nullptr, s, buf, len, flags, from, fromlen);
	}
	virtual uint64_t GetSteamIDForRemote(netadr_t* const remote){
		return g_SteamSocketMgr->GetSteamIDForRemote(remote);
	};
} g_MySocketMgr;

ISteamSocketMgr* g_pMySocketMgr = &g_MySocketMgr;
ISteamSocketMgr** g_ppMySocketmgr = &g_pMySocketMgr;

ip_address ip_from_netadr_t(netadr_t *const adr){
	ip_address ret;
	ret.octets[0] = adr->ip[0];
	ret.octets[1] = adr->ip[1];
	ret.octets[2] = adr->ip[2];
	ret.octets[3] = adr->ip[3];
	ret.port = std::byteswap(adr->port);
	return ret;
}


safetyhook::InlineHook g_GetHostVersion;
uint32_t GetHostVersion(netadr_s* adr){
	bool is_xbox = false;
	auto addr = ip_from_netadr_t(adr);
	bool found = address_cache.get(addr, is_xbox);
	if (!found){
		ConColorMsg({255,0,0,255}, "GetHostVersion: no address in cache: %i.%i.%i.%i:%i\n",
			 +addr.octets[0], +addr.octets[1], +addr.octets[2], +addr.octets[3], +addr.port);
		return 10040;
	}
	if (is_xbox){
		ConColorMsg({218,177,218, 255}, "[net] sending xbox version protocolid\n");
		return 10040;
	} else {
		ConColorMsg({218,177,218, 255}, "[net] sending ps3 version protocolid\n");
		return 10041;
	}
}


uint32_t GetHostVersion_client(){
	return 10041;
}

safetyhook::InlineHook g_CheckProtocol;
bool __fastcall CheckProtocol(void* thisptr, void* edx, netadr_t *const adr, int nProtocol)
{
	bool is_xbox = false;
	auto addr = ip_from_netadr_t(adr);
	bool has_address = address_cache.get(addr, is_xbox);
	if (!has_address){
		ConColorMsg({255,0,0,255}, "CheckProtocol: no address in cache: %i.%i.%i.%i:%i\n",
			 +addr.octets[0], +addr.octets[1], +addr.octets[2], +addr.octets[3], +addr.port);
		mem::vcall<51, void>(thisptr, adr, "500 Internal Server Error");
		return false;
	}
	if (is_xbox) {
		if (nProtocol != 10040){
			ConColorMsg({255,0,0,255}, "CheckProtocol: bad protocol for xbox host %i.%i.%i.%i:%i, protocol %i\n",
				+addr.octets[0], +addr.octets[1], +addr.octets[2], +addr.octets[3], +addr.port, nProtocol);
			mem::vcall<51, void>(thisptr, adr, "Bad Protocol");
			return false;			
		}
		return true;
	} else {
		if (nProtocol != 10041){
			ConColorMsg({255,0,0,255}, "CheckProtocol: bad protocol for ps3/pc host %i.%i.%i.%i:%i, protocol %i\n",
				+addr.octets[0], +addr.octets[1], +addr.octets[2], +addr.octets[3], +addr.port, nProtocol);
			mem::vcall<51, void>(thisptr, adr, "Bad Protocol");
			return false;			
		}
		return true;
	}
}

safetyhook::InlineHook g_CNetChan_SendNetMsg;
bool __fastcall CNetChan_SendNetMsg(void* thisptr, void* edx, void* msg, bool bForceReliable, bool bVoice){
	auto type = mem::vcall<7, int>(msg);
	if (type == 23) {
		auto* msg_ = reinterpret_cast<SVC_UserMessage*>(msg);

		std::string_view msgname(msg_->m_sDebugName.Get());
		if (msgname == "ProcessSpottedEntityUpdate"){
			return true;
		}
	}
	if (type == 15){
		return true;
	}
	return g_CNetChan_SendNetMsg.thiscall<bool>(thisptr,msg,bForceReliable,bVoice);
}

void __fastcall FillServerInfo(void* ecx, void* edx, void* base_client, SVC_ServerInfo *const serverinfo) {
	//
	using pfnFillServerInfo = void(__thiscall*)(void* ecx, SVC_ServerInfo *const serverinfo);
	reinterpret_cast<pfnFillServerInfo>(mem::rva<"engine">(0x146640))(ecx,serverinfo);

	//+0x0DC - netchannel
	uint8_t* abysmal = reinterpret_cast<uint8_t*>(base_client);
	void* instance = *reinterpret_cast<void**>(abysmal+0x0DC);
	auto addr = ip_from_netadr_t(mem::vcall<49, netadr_s *const>(instance));
	bool is_xbox = false;
	bool has_address = address_cache.get(addr, is_xbox);
	if (!has_address){
		ConColorMsg({255,0,0,255}, "FillServerInfo: no address in cache: %i.%i.%i.%i:%i\n",
			+addr.octets[0], +addr.octets[1], +addr.octets[2], +addr.octets[3], +addr.port);
	} else {
		serverinfo->m_nProtocol = is_xbox ? 10040 : 10041;
		ConColorMsg({218,177,218, 255}, "[net] FillServerInfo: spoofing protocol for %i.%i.%i.%i:%i, xbox: %s\n", +addr.octets[0], +addr.octets[1], +addr.octets[2], +addr.octets[3], +addr.port, is_xbox ? "true" : "false");
	}
	
}

void init_net() {
	using namespace asmjit;
	if (client){
		g_GetHostVersion = safetyhook::create_inline(mem::rva<"engine">(0x2036B0), GetHostVersion_client);
		return;
	}

	// assembling trampoline
	static JitRuntime rt;
	void* trampoline_code = nullptr;
	{
		// push ebx
		// call GetHostVersion
		// add esp, 4
		// jump back

		// real_Ret 147550
		// hook addr 14754B

		CodeHolder code;
		code.init(rt.environment());

		x86::Assembler a(&code);
		a.push(x86::ebx);
		a.call(Imm(&GetHostVersion));
		a.add(x86::esp, 4);
		a.jmp(Imm(mem::rva<"engine">(0x147550)));
		Error err = rt.add(&trampoline_code, &code);
	}

	// jmp trampoline
	{
		CodeHolder code;
		code.init(Environment::host(), mem::rva<"engine">(0x14754B));
		x86::Assembler a(&code);
		a.jmp(Imm(trampoline_code));
		CodeBuffer& buffer = code.sectionById(0)->buffer();
		patch_bytes(mem::rva<"engine">(0x14754B), buffer);
	}

	// logon gate
	void* trampoline_logon = nullptr;
	{
		// pop eax
		// push esi
		// push eax	
		// jmp FillServerInfo

		CodeHolder code;
		code.init(rt.environment());
		x86::Assembler a(&code);
		a.pop(x86::eax);
		a.push(x86::esi);
		a.push(x86::eax);
		a.jmp(Imm(&FillServerInfo));
		Error err = rt.add(&trampoline_logon, &code);
	}

#pragma pack(push,1)
	struct {
		uint8_t op = 0xB8;
		void* func;
		uint8_t pad = 0x90;
	} patch_ins3;
#pragma pack(pop)
	patch_ins3.func = trampoline_logon;
	patch_bytes(mem::rva<"engine">(0x129F72), {reinterpret_cast<uint8_t*>(&patch_ins3), sizeof(patch_ins3)});	

	#pragma pack(push, 1)
	struct {
		uint8_t op[2] = {0x8B, 0x0D};
		ISteamSocketMgr** ptr;
	} patch_ins2;
	#pragma pack(pop)
	patch_ins2.ptr = g_ppMySocketmgr;
	patch_bytes(mem::rva<"engine">(0x1CDAA7), {reinterpret_cast<uint8_t*>(&patch_ins2), sizeof(patch_ins2)});

	#pragma pack(push, 1)
	struct {
		uint8_t op[2] = {0x38, 0x1d};
		bool* ptr;
	} patch_ins;
	#pragma pack(pop)
	patch_ins.ptr = &g_is_next_xbox;
	patch_bytes(mem::rva<"engine">(0x1CDAF2), std::span<uint8_t>{reinterpret_cast<uint8_t*>(&patch_ins), 6});

	uint8_t dgaf_about_challenge_bitch[] = { 0x31, 0xC0, 0x40, 0xC2, 0x18, 0x00 };

	patch_bytes(mem::rva<"engine">(0x0146A50), std::span<uint8_t>(dgaf_about_challenge_bitch));
	g_CheckProtocol = safetyhook::create_inline(rva<"engine">(0x1469E0), &CheckProtocol);
	g_NET_ReceiveDatagram_mid = safetyhook::create_mid(rva<"engine">(0x1CDAF8), &NET_RecvDgram_mid);
	g_NET_SendTo = safetyhook::create_inline(rva<"engine">(0x1CA170 ), &NET_SendToImpl);
	g_CNetChan_SendNetMsg = safetyhook::create_inline(mem::rva<"engine">(0x1BDF90), &CNetChan_SendNetMsg);
}