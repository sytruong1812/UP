#pragma once
#include <string>
#include <sstream>
#include <fstream>
#include <stdint.h>
#if __cplusplus >= 201703L
#include <string_view>
#endif  // __cplusplus >= 201703L

namespace Crypto {

	std::string base64_encode_pem(std::string const& s);
	std::string base64_encode_mime(std::string const& s);
	std::string base64_encode(std::string const& s, bool url = false);
	std::string base64_encode(unsigned char const*, size_t len, bool url = false);
	std::string base64_decode(std::string const& s, bool remove_linebreaks = false);

#if __cplusplus >= 201703L
	std::string base64_encode_pem(std::string_view s);
	std::string base64_encode_mime(std::string_view s);
	std::string base64_encode(std::string_view s, bool url = false);
	std::string base64_decode(std::string_view s, bool remove_linebreaks = false);
#endif  // __cplusplus >= 201703L
}