#include "TraceLogger.h"
#include <vector>
#include <iostream>

#ifdef TRACE_LOGGER
//Singleton 
TraceLogger* TraceLogger::s_instance = 0;

void TraceLogger::EnableLog(BOOL enable) {
	enable_log = enable;
}
void TraceLogger::EnableTrace(BOOL enable) {
	enable_trace = enable;
}
void TraceLogger::SetLogOut(LOG_OPT option_) {
	WCHAR buffer[MAX_PATH] = { 0 };
	GetModuleFileNameW(NULL, buffer, MAX_PATH);
	size_t pos = std::wstring(buffer).find_last_of(L"\\/");
	std::wstring path = std::wstring(buffer).substr(0, pos) + L"\\log.txt";
	if (option_ == LOG_OPT::WRITE_FILE) {
		hFile = CreateFileW(path.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE) {
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED);
			DWORD error = GetLastError();
			std::wcout << L"SetConsoleTextAttribute returned error: " << error << std::endl;
		}
		else {
			std::wcout << L"Path: " << path << std::endl;
		}
	}
	option = option_;
}
void TraceLogger::SetLogLevel(LOG_LEVEL level_)
{
	active_level = level_;
}

bool TraceLogger::W_LOG(const CHAR* buffer, size_t size) {
	DWORD dwNumberOfBytesWrite = 0;
	if (buffer != NULL) {
		if (!WriteFile(hFile, buffer, static_cast<DWORD>(size), &dwNumberOfBytesWrite, NULL) || dwNumberOfBytesWrite != size) {
			CloseHandle(hFile);
			return false;
		}
	}
	else {
		return false;
	}
	return true;
}
void TraceLogger::L_OUT_A(const std::string& ss, size_t color = WHITE | BLACK) {
	DWORD dwNumberOfBytesWrite = 0;
	switch (option) {
	case LOG_OPT::WRITE_FILE:
		if (W_LOG(ss.c_str(), ss.length()) == false) {
			if (!SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED)) {
				DWORD error = GetLastError();
				std::cout << "SetConsoleTextAttribute returned error: " << error << std::endl;
			}
			std::cout << "Write data to file is faild!" << std::endl;
		}
		break;
	case LOG_OPT::SHOW_CONSOLE:
		if (!SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), static_cast<WORD>(color))) {
			DWORD error = GetLastError();
			std::cout << "SetConsoleTextAttribute returned error: " << error << std::endl;
		}
		else {
			std::cout << ss;
		}
		break;
	case LOG_OPT::OUTPUT_DEBUG:
		OutputDebugStringA(ss.c_str());
		break;
	default:
		break;
	}
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), WHITE | BLACK);
}
void TraceLogger::L_OUT_W(const std::wstring& ss, size_t color = WHITE | BLACK) {
	DWORD dwNumberOfBytesWrite = 0;
	switch (option) {
	case LOG_OPT::WRITE_FILE:
		if (W_LOG(wstr_to_str(ss).c_str(), ss.length()) == false) {
			if (!SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED)) {
				DWORD error = GetLastError();
				std::wcout << L"SetConsoleTextAttribute returned error: " << error << std::endl;
			}
		}
		break;
	case LOG_OPT::SHOW_CONSOLE:
		if (!SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), static_cast<WORD>(color))) {
			DWORD error = GetLastError();
			std::wcout << L"SetConsoleTextAttribute returned error: " << error << std::endl;
		}
		else {
			std::wcout << ss;
		}
		break;
	case LOG_OPT::OUTPUT_DEBUG:
		OutputDebugStringW(ss.c_str());
		break;
	default:
		break;
	}
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), WHITE | BLACK);
}

#pragma region LOGGER
void TraceLogger::LogA(ULONG line, const CHAR* format, ...) {
	CHAR* buf = NULL;
	ULONG size = 0;
	if (active_level >= NOTSET && enable_log) {
		va_list args;
		va_start(args, format);
		size = _vscprintf(format, args) + 1;
		buf = new CHAR[size];
		if (buf != NULL) {
			vsprintf_s(buf, size, format, args);
			if (option == OUTPUT_DEBUG || option == WRITE_FILE) {
				L_OUT_A(GetTimeA() + "[LOG][Line: " + std::to_string(line) + ']' + buf + "\r\n");
			}
			else {
				L_OUT_A(GetTimeA(), WHITE);
				L_OUT_A("[", WHITE);
				L_OUT_A("LOG", WHITE);
				L_OUT_A("]", WHITE);
				L_OUT_A("[Line: " + std::to_string(line) + ']' + buf + "\r\n", WHITE);
			}
			delete[] buf;
		}
		va_end(args);
	}
}
void TraceLogger::DebugA(const CHAR* func, ULONG line, const CHAR* format, ...) {
	CHAR* buf = NULL;
	ULONG size = 0;
	if (active_level >= DEBUG && enable_log) {
		va_list args;
		va_start(args, format);
		size = _vscprintf(format, args) + 1;
		buf = new CHAR[size];
		if (buf != NULL) {
			vsprintf_s(buf, size, format, args);
			if (option == OUTPUT_DEBUG || option == WRITE_FILE) {
				L_OUT_A(GetTimeA() + "[DEBUG][Line: " + std::to_string(line) + ']' + "[Function: " + std::string(func) + ']' + buf + "\r\n");
			}
			else {
				L_OUT_A(GetTimeA(), WHITE);
				L_OUT_A("[", WHITE);
				L_OUT_A("DEBUG", BLUE);
				L_OUT_A("]", WHITE);
				L_OUT_A("[Line: " + std::to_string(line) + ']', WHITE);
				L_OUT_A("[Function: " + std::string(func) + ']' + buf + "\r\n", WHITE);
			}
			delete[] buf;
		}
		va_end(args);
	}
}
void TraceLogger::InfoA(const CHAR* func, ULONG line, const CHAR* format, ...) {
	CHAR* buf = NULL;
	ULONG size = 0;
	if (active_level >= INFO && enable_log) {
		va_list args;
		va_start(args, format);
		size = _vscprintf(format, args) + 1;
		buf = new CHAR[size];
		if (buf != NULL) {
			vsprintf_s(buf, size, format, args);
			if (option == OUTPUT_DEBUG || option == WRITE_FILE) {
				L_OUT_A(GetTimeA() + "[INFO][Line: " + std::to_string(line) + ']' + "[Function: " + std::string(func) + ']' + buf + "\r\n");
			}
			else {
				L_OUT_A(GetTimeA(), WHITE);
				L_OUT_A("[", WHITE);
				L_OUT_A("INFO", FOREGROUND_GREEN);
				L_OUT_A("]", WHITE);
				L_OUT_A("[Line: " + std::to_string(line) + ']', WHITE);
				L_OUT_A("[Function: " + std::string(func) + ']' + buf + "\r\n", WHITE);
			}
			delete[] buf;
		}
		va_end(args);
	}
}
void TraceLogger::WarningA(const CHAR* func, ULONG line, const CHAR* format, ...) {
	CHAR* buf = NULL;
	ULONG size = 0;
	if (active_level >= WARN && enable_log) {
		va_list args;
		va_start(args, format);
		size = _vscprintf(format, args) + 1;
		buf = new CHAR[size];
		if (buf != NULL) {
			vsprintf_s(buf, size, format, args);
			if (option == OUTPUT_DEBUG || option == WRITE_FILE) {
				L_OUT_A(GetTimeA() + "[WARNING][Line: " + std::to_string(line) + ']' + "[Function: " + std::string(func) + ']' + buf + "\r\n");
			}
			else {
				L_OUT_A(GetTimeA(), WHITE);
				L_OUT_A("[", WHITE);
				L_OUT_A("WARNING", YELLOW);
				L_OUT_A("]", WHITE);
				L_OUT_A("[Line: " + std::to_string(line) + ']', WHITE);
				L_OUT_A("[Function: " + std::string(func) + ']' + buf + "\r\n", WHITE);
			}
			delete[] buf;
		}
		va_end(args);
	}
}
void TraceLogger::ErrorA(const CHAR* func, ULONG line, const CHAR* file, const CHAR* format, ...) {
	CHAR* buf = NULL;
	ULONG size = 0;
	if (active_level >= ERR && enable_log) {
		va_list args;
		va_start(args, format);
		size = _vscprintf(format, args) + 1;
		buf = new CHAR[size];
		if (buf != NULL) {
			vsprintf_s(buf, size, format, args);
			if (option == OUTPUT_DEBUG || option == WRITE_FILE) {
				L_OUT_A(GetTimeA() + "[ERROR][Line: " + std::to_string(line) + ']' + "[Function: " + std::string(func) + ']' + "[File:" + std::string(file) + ']' + buf + "\r\n");
			}
			else {
				L_OUT_A(GetTimeA(), WHITE);
				L_OUT_A("[", WHITE);
				L_OUT_A("ERROR", RED);
				L_OUT_A("]", WHITE);
				L_OUT_A("[Line: " + std::to_string(line) + ']', WHITE);
				L_OUT_A("[Function: " + std::string(func) + ']', WHITE);
				L_OUT_A("[File: " + std::string(file) + ']' + buf + "\r\n", WHITE);
			}
			delete[] buf;
		}
		va_end(args);
	}
}
void TraceLogger::CriticalA(const CHAR* func, ULONG line, const CHAR* file, const CHAR* format, ...) {
	CHAR* buf = NULL;
	ULONG size = 0;
	if (active_level >= CRIT && enable_log) {
		va_list args;
		va_start(args, format);
		size = _vscprintf(format, args) + 1;
		buf = new CHAR[size];
		if (buf != NULL) {
			vsprintf_s(buf, size, format, args);
			if (option == OUTPUT_DEBUG || option == WRITE_FILE) {
				L_OUT_A(GetTimeA() + "[CRITICAL][Line: " + std::to_string(line) + ']' + "[Function: " + std::string(func) + ']' + "[File:" + std::string(file) + ']' + buf + "\r\n");
			}
			else {
				L_OUT_A(GetTimeA(), WHITE);
				L_OUT_A("[", WHITE);
				L_OUT_A("CRITICAL", BACKGROUND_RED_2 | WHITE);
				L_OUT_A("]", WHITE);
				L_OUT_A("[Line: " + std::to_string(line) + ']', WHITE);
				L_OUT_A("[Function: " + std::string(func) + ']', WHITE);
				L_OUT_A("[File: " + std::string(file) + ']' + buf + "\r\n", WHITE);
			}
			delete[] buf;
		}
		va_end(args);
	}
}

void TraceLogger::LogW(ULONG line, const WCHAR* format, ...) {
	WCHAR* buf = NULL;
	ULONG size = 0;
	if (active_level >= NOTSET && enable_log) {
		va_list args;
		va_start(args, format);
		size = _vscwprintf(format, args) + 1;
		buf = new WCHAR[size];
		if (buf != NULL) {
			vswprintf_s(buf, size, format, args);
			if (option == OUTPUT_DEBUG || option == WRITE_FILE) {
				L_OUT_W(GetTimeW() + L"[LOG][Line: " + std::to_wstring(line) + L']' + buf + L"\r\n");
			}
			else {
				L_OUT_W(GetTimeW(), WHITE);
				L_OUT_W(L"[", WHITE);
				L_OUT_W(L"LOG", WHITE);
				L_OUT_W(L"]", WHITE);
				L_OUT_W(L"[Line: " + std::to_wstring(line) + L']' + buf + L"\r\n", WHITE);
			}
			delete[] buf;
		}
		va_end(args);
	}
}
void TraceLogger::DebugW(const CHAR* func, ULONG line, const WCHAR* format, ...) {
	WCHAR* buf = NULL;
	ULONG size = 0;
	if (active_level >= DEBUG && enable_log) {
		va_list args;
		va_start(args, format);
		size = _vscwprintf(format, args) + 1;
		buf = new WCHAR[size];
		if (buf != NULL) {
			vswprintf_s(buf, size, format, args);
			if (option == OUTPUT_DEBUG || option == WRITE_FILE) {
				L_OUT_W(GetTimeW() + L"[DEBUG][Line:" + std::to_wstring(line) + L']' + L"[Function: " + str_to_wstr(func) + L']' + buf + L"\r\n");
			}
			else {
				L_OUT_W(GetTimeW(), WHITE);
				L_OUT_W(L"[", WHITE);
				L_OUT_W(L"DEBUG", BLUE);
				L_OUT_W(L"]", WHITE);
				L_OUT_W(L"[Function: " + str_to_wstr(func) + L']', WHITE);
				L_OUT_W(L"[Line: " + std::to_wstring(line) + L']' + buf + L"\r\n", WHITE);
			}
			delete[] buf;
		}
		va_end(args);
	}
}
void TraceLogger::InfoW(const CHAR* func, ULONG line, const WCHAR* format, ...) {
	WCHAR* buf = NULL;
	ULONG size = 0;
	if (active_level >= INFO && enable_log) {
		va_list args;
		va_start(args, format);
		size = _vscwprintf(format, args) + 1;
		buf = new WCHAR[size];
		if (buf != NULL) {
			vswprintf_s(buf, size, format, args);
			if (option == OUTPUT_DEBUG || option == WRITE_FILE) {
				L_OUT_W(GetTimeW() + L"[INFO][Line: " + std::to_wstring(line) + L']' + L"[Function: " + str_to_wstr(func) + L']' + buf + L"\r\n");
			}
			else {
				L_OUT_W(GetTimeW(), WHITE);
				L_OUT_W(L"[", WHITE);
				L_OUT_W(L"INFO", FOREGROUND_GREEN);
				L_OUT_W(L"]", WHITE);
				L_OUT_W(L"[Function: " + str_to_wstr(func) + L']', WHITE);
				L_OUT_W(L"[Line: " + std::to_wstring(line) + L']' + buf + L"\r\n", WHITE);
			}
			delete[] buf;
		}
		va_end(args);
	}
}
void TraceLogger::WarningW(const CHAR* func, ULONG line, const WCHAR* format, ...) {
	WCHAR* buf = NULL;
	ULONG size = 0;
	if (active_level >= WARN && enable_log) {
		va_list args;
		va_start(args, format);
		size = _vscwprintf(format, args) + 1;
		buf = new WCHAR[size];
		if (buf != NULL) {
			vswprintf_s(buf, size, format, args);
			if (option == OUTPUT_DEBUG || option == WRITE_FILE) {
				L_OUT_W(GetTimeW() + L"[WARNING][Line: " + std::to_wstring(line) + L']' + L"[Function: " + str_to_wstr(func) + L']' + buf + L"\r\n");
			}
			else {
				L_OUT_W(GetTimeW(), WHITE);
				L_OUT_W(L"[", WHITE);
				L_OUT_W(L"WARNING", YELLOW);
				L_OUT_W(L"]", WHITE);
				L_OUT_W(L"[Function: " + str_to_wstr(func) + L']', WHITE);
				L_OUT_W(L"[Line: " + std::to_wstring(line) + L']' + buf + L"\r\n", WHITE);
			}
			delete[] buf;
		}
		va_end(args);
	}
}
void TraceLogger::ErrorW(const CHAR* func, ULONG line, const CHAR* file, const WCHAR* format, ...) {
	WCHAR* buf = NULL;
	ULONG size = 0;
	if (active_level >= ERR && enable_log) {
		va_list args;
		va_start(args, format);
		size = _vscwprintf(format, args) + 1;
		buf = new WCHAR[size];
		if (buf != NULL) {
			vswprintf_s(buf, size, format, args);
			if (option == OUTPUT_DEBUG || option == WRITE_FILE) {
				L_OUT_W(GetTimeW() + L"[ERROR][Line: " + std::to_wstring(line) + L']' + L"[Function: " + str_to_wstr(func) + L']' + L"[File: " + str_to_wstr(file) + L']' + buf + L"\r\n");
			}
			else {
				L_OUT_W(GetTimeW(), WHITE);
				L_OUT_W(L"[", WHITE);
				L_OUT_W(L"ERROR", RED);
				L_OUT_W(L"]", WHITE);
				L_OUT_W(L"[Function: " + str_to_wstr(func) + L']', WHITE);
				L_OUT_W(L"[Line: " + std::to_wstring(line) + L']', WHITE);
				L_OUT_W(L"[File: " + str_to_wstr(file) + L']' + buf + L"\r\n", WHITE);
			}
			delete[] buf;
		}
		va_end(args);
	}
}
void TraceLogger::CriticalW(const CHAR* func, ULONG line, const CHAR* file, const WCHAR* format, ...) {
	WCHAR* buf = NULL;
	ULONG size = 0;
	if (active_level >= CRIT && enable_log) {
		va_list args;
		va_start(args, format);
		size = _vscwprintf(format, args) + 1;
		buf = new WCHAR[size];
		if (buf != NULL) {
			vswprintf_s(buf, size, format, args);
			if (option == OUTPUT_DEBUG || option == WRITE_FILE) {
				L_OUT_W(GetTimeW() + L"[CRITICAL][Line: " + std::to_wstring(line) + L']' + L"[Function: " + str_to_wstr(func) + L']' + L"[File: " + str_to_wstr(file) + L']' + buf + L"\r\n");
			}
			else {
				L_OUT_W(GetTimeW(), WHITE);
				L_OUT_W(L"[", WHITE);
				L_OUT_W(L"CRITICAL", BACKGROUND_RED_2 | WHITE);
				L_OUT_W(L"]", WHITE);
				L_OUT_W(L"[Function: " + str_to_wstr(func) + L']', WHITE);
				L_OUT_W(L"[Line: " + std::to_wstring(line) + L']', WHITE);
				L_OUT_W(L"[File: " + str_to_wstr(file) + L']' + buf + L"\r\n", WHITE);
			}
			delete[] buf;
		}
		va_end(args);
	}
}
#pragma endregion

#pragma region TRACER
void TraceLogger::TraceA(const CHAR* format, ...) {
	CHAR* buf = NULL;
	ULONG size = 0;
	if (enable_trace) {
		va_list args;
		va_start(args, format);
		size = _vscprintf(format, args) + 1;
		buf = new CHAR[size];
		if (buf != NULL) {
			vsprintf_s(buf, size, format, args);
			if (option == OUTPUT_DEBUG || option == WRITE_FILE) {
				L_OUT_A(GetTimeA() + "[TRACING]" + buf + "\r\n");
			}
			else {
				L_OUT_A(GetTimeA(), WHITE);
				L_OUT_A("[", WHITE);
				L_OUT_A("TRACING", DARK_BLUE);
				L_OUT_A("]" + std::string(buf) + "\r\n", WHITE);
			}
			delete[] buf;
		}
		va_end(args);
	}
}
void TraceLogger::TraceInA(const CHAR* func, ULONG line, const CHAR* format, ...) {
	CHAR* buf = NULL;
	ULONG size = 0;
	if (enable_trace) {
		va_list args;
		va_start(args, format);
		size = _vscprintf(format, args) + 1;
		buf = new CHAR[size];
		if (buf != NULL) {
			vsprintf_s(buf, size, format, args);
			if (option == OUTPUT_DEBUG || option == WRITE_FILE) {
				L_OUT_A(GetTimeA() + "[TRACE_IN][Line: " + std::to_string(line) + ']' + "[Function: " + std::string(func) + ']' + buf + "\r\n");
			}
			else {
				L_OUT_A(GetTimeA(), WHITE);
				L_OUT_A("[", WHITE);
				L_OUT_A("TRACE_IN", DARK_RED);
				L_OUT_A("]", WHITE);
				L_OUT_A("[Function: " + std::string(func) + ']', WHITE);
				L_OUT_A("[Line: " + std::to_string(line) + ']' + std::string(buf) + "\r\n", WHITE);
			}
			delete[] buf;
		}
		va_end(args);
	}
}
void TraceLogger::TraceOutA(const CHAR* func, ULONG line, const CHAR* format, ...) {
	CHAR* buf = NULL;
	ULONG size = 0;
	if (enable_trace) {
		va_list args;
		va_start(args, format);
		size = _vscprintf(format, args) + 1;
		buf = new CHAR[size];
		if (buf != NULL) {
			vsprintf_s(buf, size, format, args);
			if (option == OUTPUT_DEBUG || option == WRITE_FILE) {
				L_OUT_A(GetTimeA() + "[TRACE_OUT][Line: " + std::to_string(line) + ']' + "[Function: " + std::string(func) + ']' + buf + "\r\n");
			}
			else {
				L_OUT_A(GetTimeA(), WHITE);
				L_OUT_A("[", WHITE);
				L_OUT_A("TRACE_OUT", DARK_GREEN);
				L_OUT_A("]", WHITE);
				L_OUT_A("[Function: " + std::string(func) + ']', WHITE);
				L_OUT_A("[Line: " + std::to_string(line) + ']' + std::string(buf) + "\r\n", WHITE);
			}
			delete[] buf;
		}
		va_end(args);
	}
}

void TraceLogger::TraceW(const WCHAR* format, ...) {
	WCHAR* buf = NULL;
	ULONG size = 0;
	if (enable_trace) {
		va_list args;
		va_start(args, format);
		size = _vscwprintf(format, args) + 1;
		buf = new WCHAR[size];
		if (buf != NULL) {
			vswprintf_s(buf, size, format, args);
			if (option == OUTPUT_DEBUG || option == WRITE_FILE) {
				L_OUT_W(GetTimeW() + L"[TRACING]" + buf + L"\r\n");
			}
			else {
				L_OUT_W(GetTimeW(), WHITE);
				L_OUT_W(L"[", WHITE);
				L_OUT_W(L"TRACING", DARK_BLUE);
				L_OUT_W(L"]" + std::wstring(buf) + L"\r\n", WHITE);
			}
			delete[] buf;
		}
		va_end(args);
	}
}
void TraceLogger::TraceInW(const CHAR* func, ULONG line, const WCHAR* format, ...) {
	WCHAR* buf = NULL;
	ULONG size = 0;
	if (enable_trace) {
		va_list args;
		va_start(args, format);
		size = _vscwprintf(format, args) + 1;
		buf = new WCHAR[size];
		if (buf != NULL) {
			vswprintf_s(buf, size, format, args);
			if (option == OUTPUT_DEBUG || option == WRITE_FILE) {
				L_OUT_W(GetTimeW() + L"[TRACE_IN][Line:" + std::to_wstring(line) + L']' + L"[Function: " + str_to_wstr(func) + L']' + buf + L"\r\n");
			}
			else {
				L_OUT_W(GetTimeW(), WHITE);
				L_OUT_W(L"[", WHITE);
				L_OUT_W(L"TRACE_IN", DARK_RED);
				L_OUT_W(L"]", WHITE);
				L_OUT_W(L"[Function: " + str_to_wstr(func) + L']', WHITE);
				L_OUT_W(L"[Line: " + std::to_wstring(line) + L']' + std::wstring(buf) + L"\r\n", WHITE);
			}
			delete[] buf;
		}
		va_end(args);
	}
}
void TraceLogger::TraceOutW(const CHAR* func, ULONG line, const WCHAR* format, ...) {
	WCHAR* buf = NULL;
	ULONG size = 0;
	if (enable_trace) {
		va_list args;
		va_start(args, format);
		size = _vscwprintf(format, args) + 1;
		buf = new WCHAR[size];
		if (buf != NULL) {
			vswprintf_s(buf, size, format, args);
			if (option == OUTPUT_DEBUG || option == WRITE_FILE) {
				L_OUT_W(GetTimeW() + L"[TRACE_OUT][Line:" + std::to_wstring(line) + L']' + L"[Function: " + str_to_wstr(func) + L']' + buf + L"\r\n");
			}
			else {
				L_OUT_W(GetTimeW(), WHITE);
				L_OUT_W(L"[", WHITE);
				L_OUT_W(L"TRACE_OUT", DARK_GREEN);
				L_OUT_W(L"]", WHITE);
				L_OUT_W(L"[Function: " + str_to_wstr(func) + L']', WHITE);
				L_OUT_W(L"[Line: " + std::to_wstring(line) + L']' + std::wstring(buf) + L"\r\n", WHITE);
			}
			delete[] buf;
		}
		va_end(args);
	}
}
#pragma endregion

std::string GetTimeA() {
	SYSTEMTIME sys;
	GetLocalTime(&sys);
	char tmp[64] = { '\0' };
	sprintf_s(tmp, "[%04d-%02d-%02d|%02d:%02d:%02d.%03d]", sys.wYear, sys.wMonth, sys.wDay, sys.wHour, sys.wMinute, sys.wSecond, sys.wMilliseconds);
	return tmp;
}
std::wstring GetTimeW() {
	SYSTEMTIME sys;
	GetLocalTime(&sys);
	wchar_t tmp[64] = { L'\0' };
	swprintf_s(tmp, L"[%04d-%02d-%02d|%02d:%02d:%02d.%03d]", sys.wYear, sys.wMonth, sys.wDay, sys.wHour, sys.wMinute, sys.wSecond, sys.wMilliseconds);
	return tmp;
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

#endif