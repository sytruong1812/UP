#include "common.h"

namespace SQLiteHelper {

	std::wstring STR2WSTR(const std::string& str)
	{
		const std::locale& loc = std::locale{};
		std::vector<wchar_t> buf(str.size());
		std::use_facet<std::ctype<wchar_t>>(loc).widen(str.data(), str.data() + str.size(), buf.data());
		return std::wstring(buf.data(), buf.size());
	}

	std::string WSTR2STR(const std::wstring& wstr)
	{
		const std::locale& loc = std::locale{};
		std::vector<char> buf(wstr.size());
		std::use_facet<std::ctype<wchar_t>>(loc).narrow(wstr.data(), wstr.data() + wstr.size(), '?', buf.data());
		return std::string(buf.data(), buf.size());
	}

	bool is_number(const std::string& s)
	{
		std::string::const_iterator it = s.begin();
		while (it != s.end() && std::isdigit(*it)) ++it;
		return !s.empty() && it == s.end();
	}

	void remove_sstr(std::string& str, const std::string& str_to_rm)
	{
		for (auto i = str.find(str_to_rm); i != std::string::npos; i = str.find(str_to_rm))
		{
			str.erase(i, str_to_rm.size());
		}
	}
}