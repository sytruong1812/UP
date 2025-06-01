#pragma once
#ifdef _WIN32
# include <io.h>
# include <vector>
# include <direct.h>
#else
# include <unistd.h>
# include <utime.h>
#endif

#include "zip.h"
#include "unzip.h"

#define FOPEN_FUNC(filename, mode) fopen64(filename, mode)
#define FTELLO_FUNC(stream) ftello64(stream)
#define FSEEKO_FUNC(stream, offset, origin) fseeko64(stream, offset, origin)

#define CASESENSITIVITY (0)
#define WRITEBUFFERSIZE (8192)
#define MAXFILENAME (256)

#ifdef _WIN32
#define USEWIN32IOAPI
#include "iowin32.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

enum ZLIB_VN_ERROR 
{
	UNZIP_VN_OK,
	OPEN_FILE_ZIP_ERROR,
	ALLOCATE_MEMORY_ERROR,
	GET_CURRENT_FILE_ERROR,
	OPEN_CURRENT_FILE_ERROR,
	OPEN_FILE_OUT_ERROR,
	READ_CURRENT_FILE_ERROR,
	WRITE_EXTRACTED_FILE_ERROR,
	CLOSE_CURRENT_FILE_ERROR,
	CHANGES_WORKING_DIR_ERROR,
	UNZIP_BUFFER_ERROR,
	READ_ALL_FILE_ERROR,
	GET_GLOBAL_INFO_ERROR,
	GO_TO_NEXT_FILE_ERROR
};

#if _DEBUG
static const char* ErrorMessage[] =
{
	"",
	"Error: Failed to open file zip!",
	"Error: Failed to allocate memory!",
	"Error: Failed to get current file info!",
	"Error: Failed to open current file!",
	"Error: Failed to open file out!",
	"Error: Failed to read current file!",
	"Error: Failed to write extracted file!",
	"Error: Failed to close current file!",
	"Error: Failed to unzip buffer!",
	"Error: Failed to read data from zip file!",
	"Error: Failed to write info about the ZipFile in the structure!",
	"Error: Failed to set the current file of the ZipFile to the next file!"
};
#endif

extern void ZEXPORT compressFile(FILE* source, FILE* dest);
extern void ZEXPORT decompressFile(FILE* source, FILE* dest);

extern void ZEXPORT compressBuffer(FILE* source, std::vector<unsigned char>& dest);
extern void ZEXPORT decompressBuffer(std::vector<unsigned char>& source, FILE* dest);

extern int ZEXPORT unZipBuffer(unsigned char* buf, unsigned long buf_len, const wchar_t* dest_path, const char* password);
extern int ZEXPORT unZipFile(const wchar_t* src_path, const wchar_t* dest_path, const char* password);

#ifdef __cplusplus
}
#endif