#pragma once
#include <string>
#include <vector>
#include <locale>

namespace SQLiteHelper {

	std::wstring STR2WSTR(const std::string& str);

	std::string WSTR2STR(const std::wstring& wstr);

	bool is_number(const std::string& s);

	void remove_sstr(std::string& str, const std::string& str_to_rm);

	template<typename Type>
	bool isStringNumber(Type v)
	{
		bool first = false;
		if (v.empty())
			return false;
		for (size_t i = 0; i < v.length(); i++)
		{
			if ((v[i] == '-' || v[i] == '+') && first == false && (i + 1) < v.length())
			{
				first = true;
				continue;
			}
			if (!isdigit(v[i]))
				return false;
		}
		return true;
	}
	template<typename T>
	T toLowerCase(T input)
	{
		for (size_t i = 0; i < input.length(); i++)
		{
			input[i] = tolower(input[i]);
		}
		return input;
	}
	template<typename T>
	T toUpperCase(T input)
	{
		for (size_t i = 0; i < input.length(); i++)
		{
			input[i] = toupper(input[i]);
		}
		return input;
	}
}