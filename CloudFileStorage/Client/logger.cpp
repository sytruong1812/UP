#include "logger.h"

#ifdef TRACE_LOGGER

static CRITICAL_SECTION s_mutex;

//Singleton 
TraceLogger* TraceLogger::s_instance = 0;

TraceLogger::TraceLogger(LOG_OPT option_, LOG_LEVEL level_)
{
	option = option_;
	active_level = level_;
	WCHAR buffer[MAX_PATH] = { 0 };
	GetModuleFileNameW(NULL, buffer, MAX_PATH);
	size_t pos = std::wstring(buffer).find_last_of(L"\\/");
	file_log = std::wstring(buffer).substr(0, pos) + L"\\log.txt";
	InitializeCriticalSection(&s_mutex);
}

void TraceLogger::EnableLog(BOOL enable) {
	enable_log = enable;
}

void TraceLogger::EnableTrace(BOOL enable) {
	enable_trace = enable;
}

void TraceLogger::SetLogOut(LOG_OPT option_) {
	if (option_ == LOG_OPT::WRITE_FILE)
	{
		hFile = CreateFileW(file_log.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED);
			std::wcout << L"SetConsoleTextAttribute returned error: " << GetLastError() << std::endl;
		}
		else
		{
			std::wcout << L"Path: " << file_log << std::endl;
		}
	}
	option = option_;
}

void TraceLogger::SetLogLevel(LOG_LEVEL level_)
{
	active_level = level_;
}

void TraceLogger::SetFilePath(std::string path)
{
	file_log = STR2WSTR(path);
}

bool TraceLogger::W_LOG(const CHAR* buffer, size_t size) {
	DWORD dwNumberOfBytesWrite = 0;
	if (buffer != NULL)
	{
		if (!WriteFile(hFile, buffer, static_cast<DWORD>(size), &dwNumberOfBytesWrite, NULL) || dwNumberOfBytesWrite != size)
		{
			CloseHandle(hFile);
			return false;
		}
	}
	else
	{
		return false;
	}
	return true;
}

void TraceLogger::L_OUT_A(const std::string& ss, size_t color = WHITE) {
	DWORD dwNumberOfBytesWrite = 0;
	switch (option)
	{
		case LOG_OPT::WRITE_FILE:
			if (W_LOG(ss.c_str(), ss.length()) == false)
			{
				if (!SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED))
				{
					std::cout << "SetConsoleTextAttribute returned error: " << GetLastError() << std::endl;
				}
				std::cout << "Write data to file is failed!" << std::endl;
			}
			break;
		case LOG_OPT::SHOW_MESSAGE:
		case LOG_OPT::SHOW_CONSOLE:
			if (!SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), static_cast<WORD>(color)))
			{
				std::cout << "SetConsoleTextAttribute returned error: " << GetLastError() << std::endl;
			}
			else
			{
				std::cout << ss;
			}
			break;
		case LOG_OPT::OUTPUT_DEBUG:
			OutputDebugStringA(ss.c_str());
			break;
		default:
			break;
	}
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), WHITE);
}

void TraceLogger::L_OUT_W(const std::wstring& ss, size_t color = WHITE) {
	DWORD dwNumberOfBytesWrite = 0;
	switch (option)
	{
		case LOG_OPT::WRITE_FILE:
			if (W_LOG(WSTR2STR(ss).c_str(), ss.length()) == false)
			{
				if (!SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED))
				{
					std::wcout << L"SetConsoleTextAttribute returned error: " << GetLastError() << std::endl;
				}
			}
			break;
		case LOG_OPT::SHOW_MESSAGE:
		case LOG_OPT::SHOW_CONSOLE:
			if (!SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), static_cast<WORD>(color)))
			{
				std::wcout << L"SetConsoleTextAttribute returned error: " << GetLastError() << std::endl;
			}
			else
			{
				std::wcout << ss;
			}
			break;
		case LOG_OPT::OUTPUT_DEBUG:
			OutputDebugStringW(ss.c_str());
			break;
		default:
			break;
	}
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), WHITE);
}

#pragma region LOGGER
void TraceLogger::LogA(ULONG line, const CHAR* format, ...) {
	EnterCriticalSection(&s_mutex);
	CHAR* buf = NULL;
	ULONG size = 0;
	if (active_level >= NONCE_LEVEL && enable_log)
	{
		va_list args;
		va_start(args, format);
		size = _vscprintf(format, args) + 1;
		buf = new CHAR[size];
		if (buf != NULL)
		{
			vsprintf_s(buf, size, format, args);
			if (option == OUTPUT_DEBUG || option == WRITE_FILE)
			{
				L_OUT_A(GetTimeA()
					+ "[LOG][Line: " + std::to_string(line) + ']'
					+ buf + "\r\n");
			}
			else if (option == SHOW_MESSAGE)
			{
				L_OUT_A(std::string(buf) + "\r\n");
			}
			else
			{
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
	LeaveCriticalSection(&s_mutex);
}

void TraceLogger::DebugA(const CHAR* func, ULONG line, const CHAR* format, ...) {
	EnterCriticalSection(&s_mutex);
	CHAR* buf = NULL;
	ULONG size = 0;
	if (active_level >= DEBUG_LEVEL && enable_log)
	{
		va_list args;
		va_start(args, format);
		size = _vscprintf(format, args) + 1;
		buf = new CHAR[size];
		if (buf != NULL)
		{
			vsprintf_s(buf, size, format, args);
			if (option == OUTPUT_DEBUG || option == WRITE_FILE)
			{
				L_OUT_A(GetTimeA()
					+ "[DEBUG]"
					+ "[Function: " + std::string(func) + ']'
					+ "[Thread_id: " + std::to_string(GetCurrentThreadId()) + ']'
					+ "[Line: " + std::to_string(line) + ']'
					+ buf + "\r\n");
			}
			else if (option == SHOW_MESSAGE)
			{
				L_OUT_A(std::string(buf) + "\r\n");
			}
			else
			{
				L_OUT_A(GetTimeA(), WHITE);
				L_OUT_A("[", WHITE);
				L_OUT_A("DEBUG", DARK_BLUE);
				L_OUT_A("]", WHITE);
				L_OUT_A("[Function: " + std::string(func) + ']', WHITE);
				L_OUT_A("[Thread_id: " + std::to_string(GetCurrentThreadId()) + ']', WHITE);
				L_OUT_A("[Line: " + std::to_string(line) + ']' + buf + "\r\n", WHITE);
			}
			delete[] buf;
		}
		va_end(args);
	}
	LeaveCriticalSection(&s_mutex);
}

void TraceLogger::InfoA(const CHAR* func, ULONG line, const CHAR* format, ...) {
	EnterCriticalSection(&s_mutex);
	CHAR* buf = NULL;
	ULONG size = 0;
	if (active_level >= INFO_LEVEL && enable_log)
	{
		va_list args;
		va_start(args, format);
		size = _vscprintf(format, args) + 1;
		buf = new CHAR[size];
		if (buf != NULL)
		{
			vsprintf_s(buf, size, format, args);
			if (option == OUTPUT_DEBUG || option == WRITE_FILE)
			{
				L_OUT_A(GetTimeA()
					+ "[INFO]"
					+ "[Function: " + std::string(func) + ']'
					+ "[Thread_id: " + std::to_string(GetCurrentThreadId()) + ']'
					+ "[Line: " + std::to_string(line) + ']'
					+ buf + "\r\n");
			}
			else if (option == SHOW_MESSAGE)
			{
				L_OUT_A(std::string(buf) + "\r\n");
			}
			else
			{
				L_OUT_A(GetTimeA(), WHITE);
				L_OUT_A("[", WHITE);
				L_OUT_A("INFO", CYAN);
				L_OUT_A("]", WHITE);
				L_OUT_A("[Function: " + std::string(func) + ']', WHITE);
				L_OUT_A("[Thread_id: " + std::to_string(GetCurrentThreadId()) + ']', WHITE);
				L_OUT_A("[Line: " + std::to_string(line) + ']' + buf + "\r\n", WHITE);
			}
			delete[] buf;
		}
		va_end(args);
	}
	LeaveCriticalSection(&s_mutex);
}

void TraceLogger::WarningA(const CHAR* func, ULONG line, const CHAR* format, ...) {
	EnterCriticalSection(&s_mutex);
	CHAR* buf = NULL;
	ULONG size = 0;
	if (active_level >= WARN_LEVEL && enable_log)
	{
		va_list args;
		va_start(args, format);
		size = _vscprintf(format, args) + 1;
		buf = new CHAR[size];
		if (buf != NULL)
		{
			vsprintf_s(buf, size, format, args);
			if (option == OUTPUT_DEBUG || option == WRITE_FILE)
			{
				L_OUT_A(GetTimeA()
					+ "[WARNING]"
					+ "[Function: " + std::string(func) + ']'
					+ "[Thread_id: " + std::to_string(GetCurrentThreadId()) + ']'
					+ "[Line: " + std::to_string(line) + ']'
					+ buf + "\r\n");
			}
			else if (option == SHOW_MESSAGE)
			{
				L_OUT_A(std::string(buf) + "\r\n");
			}
			else
			{
				L_OUT_A(GetTimeA(), WHITE);
				L_OUT_A("[", WHITE);
				L_OUT_A("WARNING", YELLOW);
				L_OUT_A("]", WHITE);
				L_OUT_A("[Function: " + std::string(func) + ']', WHITE);
				L_OUT_A("[Thread_id: " + std::to_string(GetCurrentThreadId()) + ']', WHITE);
				L_OUT_A("[Line: " + std::to_string(line) + ']' + buf + "\r\n", WHITE);
			}
			delete[] buf;
		}
		va_end(args);
	}
	LeaveCriticalSection(&s_mutex);
}

void TraceLogger::ErrorA(const CHAR* func, ULONG line, const CHAR* file, const CHAR* format, ...) {
	EnterCriticalSection(&s_mutex);
	CHAR* buf = NULL;
	ULONG size = 0;
	if (active_level >= ERROR_LEVEL && enable_log)
	{
		va_list args;
		va_start(args, format);
		size = _vscprintf(format, args) + 1;
		buf = new CHAR[size];
		if (buf != NULL)
		{
			vsprintf_s(buf, size, format, args);
			if (option == OUTPUT_DEBUG || option == WRITE_FILE)
			{
				L_OUT_A(GetTimeA()
					+ "[ERROR]"
					+ "[Function: " + std::string(func) + ']'
					+ "[Thread_id: " + std::to_string(GetCurrentThreadId()) + ']'
					+ "[File:" + std::string(file) + ']'
					+ "[Line:" + std::to_string(line) + ']'
					+ buf + "\r\n");
			}
			else if (option == SHOW_MESSAGE)
			{
				L_OUT_A(std::string(buf) + "\r\n");
			}
			else
			{
				L_OUT_A(GetTimeA(), WHITE);
				L_OUT_A("[", WHITE);
				L_OUT_A("ERROR", RED);
				L_OUT_A("]", WHITE);
				L_OUT_A("[Function: " + std::string(func) + ']', WHITE);
				L_OUT_A("[Thread_id: " + std::to_string(GetCurrentThreadId()) + ']', WHITE);
				L_OUT_A("[File: " + std::string(file) + ']', WHITE);
				L_OUT_A("[Line: " + std::to_string(line) + ']' + buf + "\r\n", WHITE);
			}
			delete[] buf;
		}
		va_end(args);
	}
	LeaveCriticalSection(&s_mutex);
}

void TraceLogger::SuccessA(const CHAR* func, ULONG line, const CHAR* file, const CHAR* format, ...) {
	EnterCriticalSection(&s_mutex);
	CHAR* buf = NULL;
	ULONG size = 0;
	if (active_level >= SUCCS_LEVEL && enable_log)
	{
		va_list args;
		va_start(args, format);
		size = _vscprintf(format, args) + 1;
		buf = new CHAR[size];
		if (buf != NULL)
		{
			vsprintf_s(buf, size, format, args);
			if (option == OUTPUT_DEBUG || option == WRITE_FILE)
			{
				L_OUT_A(GetTimeA()
					+ "[SUCCESS]"
					+ "[Function: " + std::string(func) + ']'
					+ "[Thread_id: " + std::to_string(GetCurrentThreadId()) + ']'
					+ "[File:" + std::string(file) + ']'
					+ "[Line: " + std::to_string(line) + ']'
					+ buf + "\r\n");
			}
			else if (option == SHOW_MESSAGE)
			{
				L_OUT_A(std::string(buf) + "\r\n");
			}
			else
			{
				L_OUT_A(GetTimeA(), WHITE);
				L_OUT_A("[", WHITE);
				L_OUT_A("SUCCESS", FOREGROUND_GREEN);
				L_OUT_A("]", WHITE);
				L_OUT_A("[Function: " + std::string(func) + ']', WHITE);
				L_OUT_A("[Thread_id: " + std::to_string(GetCurrentThreadId()) + ']', WHITE);
				L_OUT_A("[File: " + std::string(file) + ']', WHITE);
				L_OUT_A("[Line: " + std::to_string(line) + ']' + buf + "\r\n", WHITE);
			}
			delete[] buf;
		}
		va_end(args);
	}
	LeaveCriticalSection(&s_mutex);
}

void TraceLogger::CriticalA(const CHAR* func, ULONG line, const CHAR* file, const CHAR* format, ...) {
	EnterCriticalSection(&s_mutex);
	CHAR* buf = NULL;
	ULONG size = 0;
	if (active_level >= CRIT_LEVEL && enable_log)
	{
		va_list args;
		va_start(args, format);
		size = _vscprintf(format, args) + 1;
		buf = new CHAR[size];
		if (buf != NULL)
		{
			vsprintf_s(buf, size, format, args);
			if (option == OUTPUT_DEBUG || option == WRITE_FILE)
			{
				L_OUT_A(GetTimeA()
					+ "[CRITICAL]"
					+ "[Function: " + std::string(func) + ']'
					+ "[Thread_id: " + std::to_string(GetCurrentThreadId()) + ']'
					+ "[File:" + std::string(file) + ']'
					+ "[Line: " + std::to_string(line) + ']'
					+ buf + "\r\n");
			}
			else if (option == SHOW_MESSAGE)
			{
				L_OUT_A(std::string(buf) + "\r\n");
			}
			else
			{
				L_OUT_A(GetTimeA(), WHITE);
				L_OUT_A("[", WHITE);
				L_OUT_A("CRITICAL", BACKGROUND_RED_2 | WHITE);
				L_OUT_A("]", WHITE);
				L_OUT_A("[Function: " + std::string(func) + ']', WHITE);
				L_OUT_A("[Thread_id: " + std::to_string(GetCurrentThreadId()) + ']', WHITE);
				L_OUT_A("[File: " + std::string(file) + ']', WHITE);
				L_OUT_A("[Line: " + std::to_string(line) + ']' + buf + "\r\n", WHITE);
			}
			delete[] buf;
		}
		va_end(args);
	}
	LeaveCriticalSection(&s_mutex);
}

void TraceLogger::LogW(ULONG line, const WCHAR* format, ...) {
	EnterCriticalSection(&s_mutex);
	WCHAR* buf = NULL;
	ULONG size = 0;
	if (active_level >= NONCE_LEVEL && enable_log)
	{
		va_list args;
		va_start(args, format);
		size = _vscwprintf(format, args) + 1;
		buf = new WCHAR[size];
		if (buf != NULL)
		{
			vswprintf_s(buf, size, format, args);
			if (option == OUTPUT_DEBUG || option == WRITE_FILE)
			{
				L_OUT_W(GetTimeW()
					+ L"[LOG][Line: " + std::to_wstring(line) + L']'
					+ buf + L"\r\n");
			}
			else if (option == SHOW_MESSAGE)
			{
				L_OUT_W(std::wstring(buf) + L"\r\n");
			}
			else
			{
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
	LeaveCriticalSection(&s_mutex);
}

void TraceLogger::DebugW(const CHAR* func, ULONG line, const WCHAR* format, ...) {
	EnterCriticalSection(&s_mutex);
	WCHAR* buf = NULL;
	ULONG size = 0;
	if (active_level >= DEBUG_LEVEL && enable_log)
	{
		va_list args;
		va_start(args, format);
		size = _vscwprintf(format, args) + 1;
		buf = new WCHAR[size];
		if (buf != NULL)
		{
			vswprintf_s(buf, size, format, args);
			if (option == OUTPUT_DEBUG || option == WRITE_FILE)
			{
				L_OUT_W(GetTimeW()
					+ L"[DEBUG]"
					+ L"[Function: " + STR2WSTR(func) + L']'
					+ L"[Thread_id: " + std::to_wstring(GetCurrentThreadId()) + L']'
					+ L"[Line:" + std::to_wstring(line) + L']'
					+ buf + L"\r\n");
			}
			else if (option == SHOW_MESSAGE)
			{
				L_OUT_W(std::wstring(buf) + L"\r\n");
			}
			else
			{
				L_OUT_W(GetTimeW(), WHITE);
				L_OUT_W(L"[", WHITE);
				L_OUT_W(L"DEBUG", DARK_BLUE);
				L_OUT_W(L"]", WHITE);
				L_OUT_W(L"[Function: " + STR2WSTR(func) + L']', WHITE);
				L_OUT_W(L"[Thread_id: " + std::to_wstring(GetCurrentThreadId()) + L']', WHITE);
				L_OUT_W(L"[Line: " + std::to_wstring(line) + L']' + buf + L"\r\n", WHITE);
			}
			delete[] buf;
		}
		va_end(args);
	}
	LeaveCriticalSection(&s_mutex);
}

void TraceLogger::InfoW(const CHAR* func, ULONG line, const WCHAR* format, ...) {
	EnterCriticalSection(&s_mutex);
	WCHAR* buf = NULL;
	ULONG size = 0;
	if (active_level >= INFO_LEVEL && enable_log)
	{
		va_list args;
		va_start(args, format);
		size = _vscwprintf(format, args) + 1;
		buf = new WCHAR[size];
		if (buf != NULL)
		{
			vswprintf_s(buf, size, format, args);
			if (option == OUTPUT_DEBUG || option == WRITE_FILE)
			{
				L_OUT_W(GetTimeW()
					+ L"[INFO]"
					+ L"[Function: " + STR2WSTR(func) + L']'
					+ L"[Thread_id: " + std::to_wstring(GetCurrentThreadId()) + L']'
					+ L"[Line: " + std::to_wstring(line) + L']'
					+ buf + L"\r\n");
			}
			else if (option == SHOW_MESSAGE)
			{
				L_OUT_W(std::wstring(buf) + L"\r\n");
			}
			else
			{
				L_OUT_W(GetTimeW(), WHITE);
				L_OUT_W(L"[", WHITE);
				L_OUT_W(L"INFO", CYAN);
				L_OUT_W(L"]", WHITE);
				L_OUT_W(L"[Function: " + STR2WSTR(func) + L']', WHITE);
				L_OUT_W(L"[Thread_id: " + std::to_wstring(GetCurrentThreadId()) + L']', WHITE);
				L_OUT_W(L"[Line: " + std::to_wstring(line) + L']' + buf + L"\r\n", WHITE);
			}
			delete[] buf;
		}
		va_end(args);
	}
	LeaveCriticalSection(&s_mutex);
}

void TraceLogger::WarningW(const CHAR* func, ULONG line, const WCHAR* format, ...) {
	EnterCriticalSection(&s_mutex);
	WCHAR* buf = NULL;
	ULONG size = 0;
	if (active_level >= WARN_LEVEL && enable_log)
	{
		va_list args;
		va_start(args, format);
		size = _vscwprintf(format, args) + 1;
		buf = new WCHAR[size];
		if (buf != NULL)
		{
			vswprintf_s(buf, size, format, args);
			if (option == OUTPUT_DEBUG || option == WRITE_FILE)
			{
				L_OUT_W(GetTimeW()
					+ L"[WARNING]"
					+ L"[Function: " + STR2WSTR(func) + L']'
					+ L"[Thread_id: " + std::to_wstring(GetCurrentThreadId()) + L']'
					+ L"[Line: " + std::to_wstring(line) + L']'
					+ buf + L"\r\n");
			}
			else if (option == SHOW_MESSAGE)
			{
				L_OUT_W(std::wstring(buf) + L"\r\n");
			}
			else
			{
				L_OUT_W(GetTimeW(), WHITE);
				L_OUT_W(L"[", WHITE);
				L_OUT_W(L"WARNING", YELLOW);
				L_OUT_W(L"]", WHITE);
				L_OUT_W(L"[Function: " + STR2WSTR(func) + L']', WHITE);
				L_OUT_W(L"[Thread_id: " + std::to_wstring(GetCurrentThreadId()) + L']', WHITE);
				L_OUT_W(L"[Line: " + std::to_wstring(line) + L']' + buf + L"\r\n", WHITE);
			}
			delete[] buf;
		}
		va_end(args);
	}
	LeaveCriticalSection(&s_mutex);
}

void TraceLogger::ErrorW(const CHAR* func, ULONG line, const CHAR* file, const WCHAR* format, ...) {
	EnterCriticalSection(&s_mutex);
	WCHAR* buf = NULL;
	ULONG size = 0;
	if (active_level >= ERROR_LEVEL && enable_log)
	{
		va_list args;
		va_start(args, format);
		size = _vscwprintf(format, args) + 1;
		buf = new WCHAR[size];
		if (buf != NULL)
		{
			vswprintf_s(buf, size, format, args);
			if (option == OUTPUT_DEBUG || option == WRITE_FILE)
			{
				L_OUT_W(GetTimeW()
					+ L"[ERROR]"
					+ L"[Function: " + STR2WSTR(func) + L']'
					+ L"[Thread_id: " + std::to_wstring(GetCurrentThreadId()) + L']'
					+ L"[File: " + STR2WSTR(file) + L']'
					+ L"[Line: " + std::to_wstring(line) + L']'
					+ buf + L"\r\n");
			}
			else if (option == SHOW_MESSAGE)
			{
				L_OUT_W(std::wstring(buf) + L"\r\n");
			}
			else
			{
				L_OUT_W(GetTimeW(), WHITE);
				L_OUT_W(L"[", WHITE);
				L_OUT_W(L"ERROR", RED);
				L_OUT_W(L"]", WHITE);
				L_OUT_W(L"[Function: " + STR2WSTR(func) + L']', WHITE);
				L_OUT_W(L"[Thread_id: " + std::to_wstring(GetCurrentThreadId()) + L']', WHITE);
				L_OUT_W(L"[File: " + STR2WSTR(file) + L']', WHITE);
				L_OUT_W(L"[Line: " + std::to_wstring(line) + L']' + buf + L"\r\n", WHITE);
			}
			delete[] buf;
		}
		va_end(args);
	}
	LeaveCriticalSection(&s_mutex);
}

void TraceLogger::SuccessW(const CHAR* func, ULONG line, const CHAR* file, const WCHAR* format, ...) {
	EnterCriticalSection(&s_mutex);
	WCHAR* buf = NULL;
	ULONG size = 0;
	if (active_level >= SUCCS_LEVEL && enable_log)
	{
		va_list args;
		va_start(args, format);
		size = _vscwprintf(format, args) + 1;
		buf = new WCHAR[size];
		if (buf != NULL)
		{
			vswprintf_s(buf, size, format, args);
			if (option == OUTPUT_DEBUG || option == WRITE_FILE)
			{
				L_OUT_W(GetTimeW()
					+ L"[SUCCESS]"
					+ L"[Function: " + STR2WSTR(func) + L']'
					+ L"[Thread_id: " + std::to_wstring(GetCurrentThreadId()) + L']'
					+ L"[File: " + STR2WSTR(file) + L']'
					+ L"[Line: " + std::to_wstring(line) + L']'
					+ buf + L"\r\n");
			}
			else if (option == SHOW_MESSAGE)
			{
				L_OUT_W(std::wstring(buf) + L"\r\n");
			}
			else
			{
				L_OUT_W(GetTimeW(), WHITE);
				L_OUT_W(L"[", WHITE);
				L_OUT_W(L"SUCCESS", FOREGROUND_GREEN);
				L_OUT_W(L"]", WHITE);
				L_OUT_W(L"[Function: " + STR2WSTR(func) + L']', WHITE);
				L_OUT_W(L"[Thread_id: " + std::to_wstring(GetCurrentThreadId()) + L']', WHITE);
				L_OUT_W(L"[File: " + STR2WSTR(file) + L']', WHITE);
				L_OUT_W(L"[Line: " + std::to_wstring(line) + L']' + buf + L"\r\n", WHITE);
			}
			delete[] buf;
		}
		va_end(args);
	}
	LeaveCriticalSection(&s_mutex);
}

void TraceLogger::CriticalW(const CHAR* func, ULONG line, const CHAR* file, const WCHAR* format, ...) {
	EnterCriticalSection(&s_mutex);
	WCHAR* buf = NULL;
	ULONG size = 0;
	if (active_level >= CRIT_LEVEL && enable_log)
	{
		va_list args;
		va_start(args, format);
		size = _vscwprintf(format, args) + 1;
		buf = new WCHAR[size];
		if (buf != NULL)
		{
			vswprintf_s(buf, size, format, args);
			if (option == OUTPUT_DEBUG || option == WRITE_FILE)
			{
				L_OUT_W(GetTimeW()
					+ L"[CRITICAL]"
					+ L"[Function: " + STR2WSTR(func) + L']'
					+ L"[Thread_id: " + std::to_wstring(GetCurrentThreadId()) + L']'
					+ L"[File: " + STR2WSTR(file) + L']'
					+ L"[Line: " + std::to_wstring(line) + L']'
					+ buf + L"\r\n");
			}
			else if (option == SHOW_MESSAGE)
			{
				L_OUT_W(std::wstring(buf) + L"\r\n");
			}
			else
			{
				L_OUT_W(GetTimeW(), WHITE);
				L_OUT_W(L"[", WHITE);
				L_OUT_W(L"CRITICAL", BACKGROUND_RED_2 | WHITE);
				L_OUT_W(L"]", WHITE);
				L_OUT_W(L"[Function: " + STR2WSTR(func) + L']', WHITE);
				L_OUT_W(L"[Thread_id: " + std::to_wstring(GetCurrentThreadId()) + L']', WHITE);
				L_OUT_W(L"[File: " + STR2WSTR(file) + L']', WHITE);
				L_OUT_W(L"[Line: " + std::to_wstring(line) + L']' + buf + L"\r\n", WHITE);
			}
			delete[] buf;
		}
		va_end(args);
	}
	LeaveCriticalSection(&s_mutex);
}
#pragma endregion

#pragma region TRACER
void TraceLogger::TraceA(const CHAR* format, ...) {
	EnterCriticalSection(&s_mutex);
	CHAR* buf = NULL;
	ULONG size = 0;
	if (enable_trace)
	{
		va_list args;
		va_start(args, format);
		size = _vscprintf(format, args) + 1;
		buf = new CHAR[size];
		if (buf != NULL)
		{
			vsprintf_s(buf, size, format, args);
			if (option == OUTPUT_DEBUG || option == WRITE_FILE)
			{
				L_OUT_A(GetTimeA() + "[TRACING]" + buf + "\r\n");
			}
			else if (option == SHOW_MESSAGE)
			{
				L_OUT_A(std::string(buf) + "\r\n");
			}
			else
			{
				L_OUT_A(GetTimeA(), WHITE);
				L_OUT_A("[", WHITE);
				L_OUT_A("TRACING", DARK_BLUE);
				L_OUT_A("]" + std::string(buf) + "\r\n", WHITE);
			}
			delete[] buf;
		}
		va_end(args);
	}
	LeaveCriticalSection(&s_mutex);
}

void TraceLogger::TraceInA(const CHAR* func, ULONG line, const CHAR* format, ...) {
	EnterCriticalSection(&s_mutex);
	CHAR* buf = NULL;
	ULONG size = 0;
	if (enable_trace)
	{
		va_list args;
		va_start(args, format);
		size = _vscprintf(format, args) + 1;
		buf = new CHAR[size];
		if (buf != NULL)
		{
			vsprintf_s(buf, size, format, args);
			if (option == OUTPUT_DEBUG || option == WRITE_FILE)
			{
				L_OUT_A(GetTimeA()
					+ "[TRACE_IN]"
					+ "[Function: " + std::string(func) + ']'
					+ "[Thread_id: " + std::to_string(GetCurrentThreadId()) + ']'
					+ "[Line: " + std::to_string(line) + ']'
					+ buf + "\r\n");
			}
			else if (option == SHOW_MESSAGE)
			{
				L_OUT_A(std::string(buf) + "\r\n");
			}
			else
			{
				L_OUT_A(GetTimeA(), WHITE);
				L_OUT_A("[", WHITE);
				L_OUT_A("TRACE_IN", BLUE);
				L_OUT_A("]", WHITE);
				L_OUT_A("[Function: " + std::string(func) + ']', WHITE);
				L_OUT_A("[Thread_id: " + std::to_string(GetCurrentThreadId()) + ']', WHITE);
				L_OUT_A("[Line: " + std::to_string(line) + ']' + std::string(buf) + "\r\n", WHITE);
			}
			delete[] buf;
		}
		va_end(args);
	}
	LeaveCriticalSection(&s_mutex);
}

void TraceLogger::TraceOutA(const CHAR* func, ULONG line, const CHAR* format, ...) {
	EnterCriticalSection(&s_mutex);
	CHAR* buf = NULL;
	ULONG size = 0;
	if (enable_trace)
	{
		va_list args;
		va_start(args, format);
		size = _vscprintf(format, args) + 1;
		buf = new CHAR[size];
		if (buf != NULL)
		{
			vsprintf_s(buf, size, format, args);
			if (option == OUTPUT_DEBUG || option == WRITE_FILE)
			{
				L_OUT_A(GetTimeA()
					+ "[TRACE_OUT]"
					+ "[Function: " + std::string(func) + ']'
					+ "[Thread_id: " + std::to_string(GetCurrentThreadId()) + ']'
					+ "[Line: " + std::to_string(line) + ']'
					+ buf + "\r\n");
			}
			else if (option == SHOW_MESSAGE)
			{
				L_OUT_A(std::string(buf) + "\r\n");
			}
			else
			{
				L_OUT_A(GetTimeA(), WHITE);
				L_OUT_A("[", WHITE);
				L_OUT_A("TRACE_OUT", BLUE);
				L_OUT_A("]", WHITE);
				L_OUT_A("[Function: " + std::string(func) + ']', WHITE);
				L_OUT_A("[Thread_id: " + std::to_string(GetCurrentThreadId()) + ']', WHITE);
				L_OUT_A("[Line: " + std::to_string(line) + ']' + std::string(buf) + "\r\n", WHITE);
			}
			delete[] buf;
		}
		va_end(args);
	}
	LeaveCriticalSection(&s_mutex);
}

void TraceLogger::TraceW(const WCHAR* format, ...) {
	EnterCriticalSection(&s_mutex);
	WCHAR* buf = NULL;
	ULONG size = 0;
	if (enable_trace)
	{
		va_list args;
		va_start(args, format);
		size = _vscwprintf(format, args) + 1;
		buf = new WCHAR[size];
		if (buf != NULL)
		{
			vswprintf_s(buf, size, format, args);
			if (option == OUTPUT_DEBUG || option == WRITE_FILE)
			{
				L_OUT_W(GetTimeW() + L"[TRACING]" + buf + L"\r\n");
			}
			else if (option == SHOW_MESSAGE)
			{
				L_OUT_W(std::wstring(buf) + L"\r\n");
			}
			else
			{
				L_OUT_W(GetTimeW(), WHITE);
				L_OUT_W(L"[", WHITE);
				L_OUT_W(L"TRACING", DARK_BLUE);
				L_OUT_W(L"]" + std::wstring(buf) + L"\r\n", WHITE);
			}
			delete[] buf;
		}
		va_end(args);
	}
	LeaveCriticalSection(&s_mutex);
}

void TraceLogger::TraceInW(const CHAR* func, ULONG line, const WCHAR* format, ...) {
	EnterCriticalSection(&s_mutex);
	WCHAR* buf = NULL;
	ULONG size = 0;
	if (enable_trace)
	{
		va_list args;
		va_start(args, format);
		size = _vscwprintf(format, args) + 1;
		buf = new WCHAR[size];
		if (buf != NULL)
		{
			vswprintf_s(buf, size, format, args);
			if (option == OUTPUT_DEBUG || option == WRITE_FILE)
			{
				L_OUT_W(GetTimeW()
					+ L"[TRACE_IN]"
					+ L"[Function: " + STR2WSTR(func) + L']'
					+ L"[Thread_id: " + std::to_wstring(GetCurrentThreadId()) + L']'
					+ L"[Line:" + std::to_wstring(line) + L']'
					+ buf + L"\r\n");
			}
			else if (option == SHOW_MESSAGE)
			{
				L_OUT_W(std::wstring(buf) + L"\r\n");
			}
			else
			{
				L_OUT_W(GetTimeW(), WHITE);
				L_OUT_W(L"[", WHITE);
				L_OUT_W(L"TRACE_IN", BLUE);
				L_OUT_W(L"]", WHITE);
				L_OUT_W(L"[Function: " + STR2WSTR(func) + L']', WHITE);
				L_OUT_W(L"[Thread_id: " + std::to_wstring(GetCurrentThreadId()) + L']', WHITE);
				L_OUT_W(L"[Line: " + std::to_wstring(line) + L']' + std::wstring(buf) + L"\r\n", WHITE);
			}
			delete[] buf;
		}
		va_end(args);
	}
	LeaveCriticalSection(&s_mutex);
}

void TraceLogger::TraceOutW(const CHAR* func, ULONG line, const WCHAR* format, ...) {
	EnterCriticalSection(&s_mutex);
	WCHAR* buf = NULL;
	ULONG size = 0;
	if (enable_trace)
	{
		va_list args;
		va_start(args, format);
		size = _vscwprintf(format, args) + 1;
		buf = new WCHAR[size];
		if (buf != NULL)
		{
			vswprintf_s(buf, size, format, args);
			if (option == OUTPUT_DEBUG || option == WRITE_FILE)
			{
				L_OUT_W(GetTimeW()
					+ L"[TRACE_OUT]"
					+ L"[Function: " + STR2WSTR(func) + L']'
					+ L"[Thread_id: " + std::to_wstring(GetCurrentThreadId()) + L']'
					+ L"[Line:" + std::to_wstring(line) + L']'
					+ buf + L"\r\n");
			}
			else if (option == SHOW_MESSAGE)
			{
				L_OUT_W(std::wstring(buf) + L"\r\n");
			}
			else
			{
				L_OUT_W(GetTimeW(), WHITE);
				L_OUT_W(L"[", WHITE);
				L_OUT_W(L"TRACE_OUT", BLUE);
				L_OUT_W(L"]", WHITE);
				L_OUT_W(L"[Function: " + STR2WSTR(func) + L']', WHITE);
				L_OUT_W(L"[Thread_id: " + std::to_wstring(GetCurrentThreadId()) + L']', WHITE);
				L_OUT_W(L"[Line: " + std::to_wstring(line) + L']' + std::wstring(buf) + L"\r\n", WHITE);
			}
			delete[] buf;
		}
		va_end(args);
	}
	LeaveCriticalSection(&s_mutex);
}
#pragma endregion

std::string TraceLogger::GetTimeA() {
	SYSTEMTIME sys;
	GetLocalTime(&sys);
	char tmp[64] = { '\0' };
	sprintf_s(tmp, "[%04d-%02d-%02d|%02d:%02d:%02d.%03d]", sys.wYear, sys.wMonth, sys.wDay, sys.wHour, sys.wMinute, sys.wSecond, sys.wMilliseconds);
	return tmp;
}
std::wstring TraceLogger::GetTimeW() {
	SYSTEMTIME sys;
	GetLocalTime(&sys);
	wchar_t tmp[64] = { L'\0' };
	swprintf_s(tmp, L"[%04d-%02d-%02d|%02d:%02d:%02d.%03d]", sys.wYear, sys.wMonth, sys.wDay, sys.wHour, sys.wMinute, sys.wSecond, sys.wMilliseconds);
	return tmp;
}
std::wstring TraceLogger::STR2WSTR(const std::string& str)
{
	const std::locale& loc = std::locale{};
	std::vector<wchar_t> buf(str.size());
	std::use_facet<std::ctype<wchar_t>>(loc).widen(str.data(), str.data() + str.size(), buf.data());
	return std::wstring(buf.data(), buf.size());
}
std::string TraceLogger::WSTR2STR(const std::wstring& wstr)
{
	const std::locale& loc = std::locale{};
	std::vector<char> buf(wstr.size());
	std::use_facet<std::ctype<wchar_t>>(loc).narrow(wstr.data(), wstr.data() + wstr.size(), '?', buf.data());
	return std::string(buf.data(), buf.size());
}

#endif