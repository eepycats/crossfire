#pragma once
#include <srcsdk/sdk.hpp>
#include <string_view>
namespace dt {
	void destroy_member(ssdk::dt::SendTable* table, std::string_view prop_name);
	ssdk::dt::ServerClass* GetAllServerClasses();
	void dump_sendtables(std::string_view filename);
}