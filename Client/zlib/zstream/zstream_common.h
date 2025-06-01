#ifndef ZIP_STREAM_COMMON_HPP
#define ZIP_STREAM_COMMON_HPP

#include <stdint.h>

namespace zstream {

/// Compression strategy, see zlib doc.
enum EStrategy {
	StrategyFiltered = 1, StrategyHuffmanOnly = 2, DefaultStrategy = 0
};

namespace detail {
/// default gzip buffer size,
/// change this to suite your needs
const size_t default_buffer_size = 4096;

const int gz_magic[2] = { 0x1f, 0x8b }; /* gzip magic header */

/* gzip flag byte */
const int gz_ascii_flag = 0x01; /* bit 0 set: file probably ascii text */
const int gz_head_crc = 0x02; /* bit 1 set: header CRC present */
const int gz_extra_field = 0x04; /* bit 2 set: extra field present */
const int gz_orig_name = 0x08; /* bit 3 set: original file name present */
const int gz_comment = 0x10; /* bit 4 set: file comment present */
const int gz_reserved = 0xE0; /* bits 5..7: reserved */

} // detail
} // zstream

#endif

