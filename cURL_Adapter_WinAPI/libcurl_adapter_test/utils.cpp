#include "utils.h"

namespace Utils
{

	bool is_number(const std::string& s)
	{
		std::string::const_iterator it = s.begin();
		while (it != s.end() && std::isdigit(*it)) ++it;
		return !s.empty() && it == s.end();
	}

	std::wstring str_to_wstr(const std::string& str)
	{
		const std::locale& loc = std::locale{};
		std::vector<wchar_t> buf(str.size());
		std::use_facet<std::ctype<wchar_t>>(loc).widen(str.data(), str.data() + str.size(), buf.data());
		return std::wstring(buf.data(), buf.size());
	}

	std::string wstr_to_str(const std::wstring& wstr)
	{
		const std::locale& loc = std::locale{};
		std::vector<char> buf(wstr.size());
		std::use_facet<std::ctype<wchar_t>>(loc).narrow(wstr.data(), wstr.data() + wstr.size(), '?', buf.data());
		return std::string(buf.data(), buf.size());
	}

	// helper function that converts an unsigned char to hex
	// this is faster than using the std::stringstream object
	// _in_  : unsigned char character
	// _out_ : hex value stored in a string object
	inline std::string char_to_hex(const uint8_t cchar)
	{
		std::string hexchar;
		hexchar.reserve(2);

		uint8_t uchar = cchar >> 4;
		uint8_t lchar = cchar & 0b1111;

		hexchar += (uchar < 10) ? char('0' + uchar) : char('A' + uchar - 10);
		hexchar += (lchar & 0b1111) < 10 ? char('0' + lchar) : char('A' + lchar - 10);

		return hexchar;
	}

	// helper function that retrieves the file size
	// _in_  : file name
	// _out_ : size of file as uint64_t
	uint64_t get_file_size(const std::string& filename)
	{
		uint64_t fsize = 0;
		std::ifstream file_bin;
		file_bin.open(filename, std::ios::in | std::ifstream::binary | std::ios::ate);

		if (file_bin.is_open())
		{
			fsize = file_bin.tellg();
			file_bin.close();
		}
		return fsize;
	}

	// helper function that writes to a file on disk (in binary mode)
	// _in_ (filename)  : file name to open/(write to)
	// _in_ (data_out)  : data
	// _out_ : true if writing to (filename) was successful, otherwise, false
	bool save_file_binary(const std::string& filename, const std::string& data_out)
	{
		std::ofstream file_out;
		file_out.open(filename, std::ios::out | std::ios::binary);

		if (file_out)
		{
			for (size_t i = 0; i < data_out.size(); i += 2)
			{
				file_out << static_cast<uint8_t>(strtol(data_out.substr(i, 2).data(), nullptr, 16) & 0xff);
			}

			file_out.close();
			return (!file_out);
		}
		return false;
	}

	// helper function that writes to a file on disk (in text mode)
	// _in_ (filename)  : file name to open/(write to)
	// _in_ (data_out)  : data
	// _out_ : true if writing to (filename) was successful, otherwise, false
	bool save_file_text(const std::string& filename, const std::string& data_out)
	{
		std::ofstream file_out;
		file_out.open(filename, std::ios::out);

		if (file_out)
		{
			file_out.write(data_out.c_str(), data_out.size());
			file_out.close();

			return (!file_out);
		}
		return false;
	}

	// helper function that opens a file on disk (in binary mode)
	// _in_  : file name to open
	// _out_ : a string object holding the content of the file
	std::string open_file_binay(const std::string& filename)
	{
		std::ifstream file_in;
		std::string file_buff_hex_stream;
		file_in.open(filename, std::ios::in | std::ios::binary);

		if (file_in.is_open())
		{
			size_t fsize = static_cast<size_t>(get_file_size(filename));

			unsigned char* file_buff = new unsigned char[fsize];

			file_in.seekg(0, std::ios::beg);
			file_in.read((char*)file_buff, fsize);
			file_in.close();

			file_buff_hex_stream.reserve(fsize);

			for (size_t i = 0; i < fsize; ++i)
			{
				file_buff_hex_stream += char_to_hex(static_cast<uint8_t>(file_buff[i]));
			}
			if (file_buff != nullptr) {
				delete[] file_buff;
			}
		}
		return file_buff_hex_stream;
	}

	// helper function that opens a file on disk (in text mode)
	// _in_  : file name to open
	// _out_ : a string object holding the content of the file
	std::string open_file_text(const std::string& filename)
	{
		std::ifstream file_in;
		std::stringstream file_in_strstream;

		file_in.open(filename, std::ios::in | std::ios::binary);

		if (file_in.is_open())
		{
			file_in_strstream << file_in.rdbuf();
			file_in.close();
		}
		return file_in_strstream.str();
	}
}