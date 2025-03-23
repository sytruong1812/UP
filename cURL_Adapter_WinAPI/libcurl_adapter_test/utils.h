#pragma once
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <stdint.h>

namespace Utils
{

    bool is_number(const std::string& s);

    std::wstring str_to_wstr(const std::string& str);
    std::string wstr_to_str(const std::wstring& wstr);

    std::string char_to_hex(uint8_t cchar);
    uint64_t get_file_size(const std::string& filename);

    std::string open_file_binay(const std::string& filename);
    std::string open_file_text(const std::string& filename);

    bool save_file_binary(const std::string& filename, const std::string& data_out);
    bool save_file_text(const std::string& filename, const std::string& data_out);

}