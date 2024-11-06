#include <map>
#include <string>
#include <chrono>
#include <thread>
#include <iostream>
#include <windows.h>

#include "MD5.h"
#include "utils.h"
#include "SHA-256.h"
#include "TraceLogger.h"
#include "backup_import.h"

using namespace SQLiteHelper;

#define MAX_THREAD 2
HANDLE hMutex = NULL;
HANDLE hSemaphore = NULL;

enum OPTION { 
	PATH_IN, 
	PATH_OUT,
	KEY_SEE, 
	KEY_AES,
	COLUMN,
	WILDCARD,
	FROM, 
	TO,
	LIMIT,
	THREAD,
	SHOW_LOG
};

std::map<std::wstring, OPTION> LIST_OPTION{
	{L"/input"	,	PATH_IN},
	{L"/output",	PATH_OUT},
	{L"/see"	,	KEY_SEE},
	{L"/key_see",	KEY_SEE},
	{L"/aes"	,	KEY_AES},
	{L"/key_aes",	KEY_AES},
	{L"/column",	COLUMN},
	{L"/wildcard",	WILDCARD},
	{L"/from",		FROM},
	{L"/to",		TO},
	{L"/limit",		LIMIT},
	{L"/thread",	THREAD},
	{L"/log",		SHOW_LOG}
};

void Usage() {
	std::wstring tool_name = Utils::get_exe_name();
	std::wcout << L"\nUSAGE: " << tool_name << L" <FILE PATH> <FILTER> <OPTION>." << std::endl;
	std::wcout << L"\tFILE PATH:" << std::endl;
	std::wcout << L"\t\t/input   : Path to the input file or folder." << std::endl;
	std::wcout << L"\t\t/output  : Path to the output file or folder." << std::endl;
	std::wcout << L"\tFILTER:" << std::endl;
	std::wcout << L"\t\t/column  : Column name with type date time utc." << std::endl;
	std::wcout << L"\t\t/wildcard: A wildcard character used to substitute one or more characters in a string." << std::endl;
	std::wcout << L"\t\t/from    : Start date time (UTC)." << std::endl;
	std::wcout << L"\t\t/to      : End date time (UTC)." << std::endl;
	std::wcout << L"\tOPTION:" << std::endl;
	std::wcout << L"\t\t/see     : Specify the encryption key on a newly opened database connection (MD5 hash)(File output)." << std::endl;
	std::wcout << L"\t\t/aes     : Specify the decryption key for data decrypted with Aes-256-gcm in a blob column (SHA-256 hash)(File input)." << std::endl;
	std::wcout << L"\t\t/limit   : Number of rows backup/import per section (default = 100 rows)." << std::endl;
	std::wcout << L"\t\t/thread  : Set number of threads run (default = 2 thread)." << std::endl;
	std::wcout << L"\t\t/log     : Enable logging." << std::endl;
	std::wcout << L"EXAMPLES:" << std::endl;
	std::wcout << L"\tEx1: " << tool_name << L" /input C:\\Document\\From\\file.db /output C:\\Document\\To\\backup.db " << std::endl;
	std::wcout << L"\tEx2: " << tool_name << L" /input C:\\Document\\From\\file.db /output C:\\Document\\To\\backup.db /see \"Key123\" /aes \"Key456\" /limit 10 " << std::endl;
	std::wcout << L"\tEx3: " << tool_name << L" /input C:\\Document\\From\\file.db /output C:\\Document\\To\\backup.db /column \"creation_utc\" /from \"13-10-2024\" /to \"15-10-2024\" " << std::endl;
}

struct thread_info {
	DWORD id = 0;
	HANDLE hThread = NULL;
	BackupImport* object = NULL;
	std::wstring path_in;
	std::wstring path_out;
};

void backupFile(BackupImport* helper, const std::wstring& path_input, const std::wstring& path_output)
{
	int result = FUNCTION_DONE;
	if ((result = helper->backupDatabase(path_input, path_output)) != FUNCTION_DONE) {
		std::wcout << L"--> Error: " << helper->showError(result) << std::endl;
	}
	else {
		std::wcout << "\n" << helper->showResult() << std::endl;
	}
}

void WINAPI ThreadProc(thread_info* info)
{
	DWORD dwWaitSemaphore = WaitForSingleObject(hSemaphore, INFINITE);		// Wait for permission from Semaphore
	DWORD dwWaitMutex = WaitForSingleObject(hMutex, INFINITE);				// Wait for permission from Mutex
	if (Utils::path_exists(info->path_out)) {
		if (Utils::delete_file(info->path_out) == false) {
			std::wcout << L"--> Error: Failed to delete the file, the file is currently open!" << std::endl;
		}
	}
	else {
		std::wstring dir = Utils::getDirFromPath(info->path_out);
		if (Utils::createNestedDir(dir) == false) {
			std::wcout << L"--> Error: Failed to create nested directories! " << std::endl;
		}
	}
	if (dwWaitMutex == WAIT_OBJECT_0) {
		ReleaseMutex(hMutex);		// Release the semaphore when task is finished
	}
	
	backupFile(info->object, info->path_in, info->path_out);
	if (info->hThread != NULL) {
		CloseHandle(info->hThread);
	}
	if (info->object != NULL) {
		delete info->object;
	}
	if (dwWaitSemaphore == WAIT_OBJECT_0) {
		ReleaseSemaphore(hSemaphore, 1, NULL);		// Release the semaphore when task is finished
	}
}

void backupFolder(BackupImport* base, int max_thread, const std::wstring& path_input, const std::wstring& path_output)
{
	std::vector<HANDLE> hThreads;
	std::vector<thread_info*> lThreads;
	std::vector<std::pair<std::wstring, std::wstring>> lFilePath;
	if (!Utils::getFilesInNestedFolder_v2(path_input, path_output, &lFilePath)) {
		return;
	}
	std::wcout << L"--> Number of file backup: " << lFilePath.size() << std::endl;

	hSemaphore = CreateSemaphore(NULL, max_thread, max_thread, NULL);
	if (!hSemaphore) {
		std::wcout << L"Failed to create semaphore, error = " << GetLastError() << std::endl;
		return;
	}
	hMutex = CreateMutex(NULL, FALSE, NULL);
	if (!hMutex) {
		std::wcout << L"Failed to create mutex, error = " << GetLastError() << std::endl;
		return;
	}
	for (auto it : lFilePath) 
	{
		thread_info* info = new thread_info();
		info->object = new BackupImport(base);
		info->path_in = it.first;
		info->path_out = it.second;
		info->hThread = CreateThread(NULL, STACK_SIZE_PARAM_IS_A_RESERVATION, (LPTHREAD_START_ROUTINE)ThreadProc, info, 0, &info->id);
		if (info->hThread == NULL) {
			std::wcout << L"Failed to create thread." << std::endl;
			delete info;
		}
		else {
			hThreads.push_back(info->hThread);
			lThreads.push_back(info);
		}
	}
	WaitForMultipleObjects((DWORD)hThreads.size(), hThreads.data(), TRUE, INFINITE);
	for (thread_info* it : lThreads) {
		delete it;
	}
	if (hSemaphore != NULL) {
		CloseHandle(hSemaphore);
	}
}

static void CommandLine(int argc, wchar_t* argv[])
{
	if (argc < 4)
	{
		std::wcout << L"--> Insufficient number of arguments to execute the program, please check again." << std::endl;
		return;
	}
	int max_thread = MAX_THREAD;
	std::wstring path_input;
	std::wstring path_output;
	std::unique_ptr<BackupImport> helper = std::make_unique<BackupImport>();
	for (int i = 1; i < argc; i++)
	{
		switch (LIST_OPTION[argv[i]])
		{
		case OPTION::PATH_IN:
			if ((i + 1 != argc))
			{
				path_input = Utils::getPathFromEnv(argv[i + 1]);
				i++;
			}
			break;
		case OPTION::PATH_OUT:
			if ((i + 1 != argc))
			{
				path_output = Utils::getPathFromEnv(argv[i + 1]);
				i++;
			}
			break;
		case OPTION::KEY_SEE:
			if ((i + 1 != argc))
			{
				std::wstring k = argv[i + 1];
				BYTE keySEE_MD5[MD5_DIGEST_LENGTH];
				if (MD5Hash((BYTE*)k.c_str(), k.length(), keySEE_MD5)) {
					std::wcout << L"--> Key SEE: " << k << " -> Hash : ";
					for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
						wprintf(L"%02x", keySEE_MD5[i]);
					}
					std::wcout << L" [Algorithm MD5] " << std::endl;
				}
				helper->setKeySee(keySEE_MD5, MD5_DIGEST_LENGTH);
				i++;
			}
			break;
		case OPTION::KEY_AES:
			if ((i + 1 != argc))
			{
				std::wstring k = argv[i + 1];
				BYTE keyAES_SHA256[SHA256_DIGEST_LENGTH];
				if (SHA256_Hash((BYTE*)k.c_str(), k.length(), keyAES_SHA256)) {
					std::wcout << L"--> Key AES: " << k << " -> Hash : ";
					for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
						wprintf(L"%02x", keyAES_SHA256[i]);
					}
					std::wcout << L" [Algorithm SHA256] " << std::endl;
				}
				AesManager::instance()->setEncryptFlag(false);
				AesManager::instance()->setDecryptFlag(true);
				AesManager::instance()->setKey(keyAES_SHA256, SHA256_DIGEST_LENGTH);
				i++;
			}
			break;
		case OPTION::COLUMN:
			if ((i + 1 != argc))
			{
				DateTime::instance()->setColumn_utc(Utils::wstr2str(argv[i + 1]));
				i++;
			}
			break;
		case OPTION::WILDCARD:
			if ((i + 1 != argc))
			{
				DateTime::instance()->setWildCard(Utils::wstr2str(argv[i + 1]));
				i++;
			}
			break;
		case OPTION::FROM:
			if ((i + 1 != argc))
			{
				DateTime::instance()->setDateTimeFrom(Utils::wstr2str(argv[i + 1]));
				i++;
			}
			break;
		case OPTION::TO:
			if ((i + 1 != argc))
			{
				DateTime::instance()->setDateTimeTo(Utils::wstr2str(argv[i + 1]));
				i++;
			}
			break;
		case OPTION::LIMIT:
			if ((i + 1 != argc))
			{
				helper->setLimitRow(_wtoi(argv[i + 1]));
				i++;
			}
			break;
		case OPTION::THREAD:
			if ((i + 1 != argc))
			{
				max_thread = _wtoi(argv[i + 1]);
				i++;
			}
			break;
		case OPTION::SHOW_LOG:
			if ((i != argc))
			{
				ENABLE_LOG(TRUE);
			}
			break;
		default:
			break;
		}
	}

	if (path_input.empty() || path_output.empty())
	{
		std::wcout << L"--> Error: The input file path not valid!" << std::endl;
	}
	else
	{
		auto start = std::chrono::high_resolution_clock::now();
		if (!Utils::path_exists(path_input)) {
			std::wcout << L"\n--> Error: Path input does not exist!" << std::endl;
		}
		else {
			if (PathIsDirectoryW(path_input.c_str())) {
				std::wcout << "--> Maximum running thread: " << max_thread << std::endl;
				backupFolder(helper.get(), max_thread, path_input, path_output);
			}
			else {
				std::wstring dir = Utils::getDirFromPath(path_output);
				if (Utils::createNestedDir(dir) == false) {
					std::wcout << L"--> Error: Failed to create nested directories! " << std::endl;
				}
				else {
					backupFile(helper.get(), path_input, path_output);
				}
			}
		}
		auto stop = std::chrono::high_resolution_clock::now();
		std::wcout << "\n--> Execution time: "
			<< std::chrono::duration_cast<std::chrono::minutes>(stop - start).count() << "m "
			<< std::chrono::duration_cast<std::chrono::seconds>(stop - start).count() << "s "
			<< std::endl;
	}
}

int wmain(int argc, wchar_t* argv[])
{
	ENABLE_LOG(FALSE);
	SET_LOG_LEVEL(ERR);
	SET_LOG_OUT(SHOW_CONSOLE);

	if (argc == 1)
	{
		std::wcout << L"/? <help>: Display this help screen." << std::endl;
	}
	else if (wcscmp(argv[1], L"/?") == 0)
	{
		Usage();
	}
	else
	{
		CommandLine(argc, argv);
	}
	return 0;
}
