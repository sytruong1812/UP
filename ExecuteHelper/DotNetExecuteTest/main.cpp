#include <string>
#include <tchar.h>
#include <stdio.h>
#include <iostream>
#include <windows.h>

#define OPTION 1

#if (OPTION == 1)

#include <comdef.h>
#include "executor.h"

#define DLL_PATH L"E:\\dev\\SE32-Execute_Library\\ExecuteHelper\\MyLibrary\\bin\\Release\\MyLibrary.dll"

void TEST_1()
{
	BOOL result = FALSE;

	//result = ExecuteMethodFromFile(DLL_PATH, L"MyLibrary.Math", L"Add", L"1 2");
	result = ExecuteMethodFromFile(DLL_PATH, L"MyLibrary.Show", L"ShowCmd", L"It Works!");
	//result = ExecuteMethodFromFile(DLL_PATH, L"MyLibrary.Show", L"ShowMsg", L"Hello World!");
	if (result)
	{
		std::wcout << "Function successfully." << std::endl;
	}
}

void TEST_2()
{
	FILE* fd;
	BOOL result = FALSE;
	struct _stat64i32 fs;
	_wstat(DLL_PATH, &fs);
	if (fs.st_size == 0)
	{
		wprintf(L"File is empty!\n");
		return;
	}
	_wfopen_s(&fd, DLL_PATH, L"rb");
	if (fd == NULL)
	{
		wprintf(L"Unable to open \"%s\".\n", DLL_PATH);
		return;
	}
	void* mem = malloc(fs.st_size);
	if (mem != NULL)
	{
		fread(mem, 1, fs.st_size, fd);

		//result = ExecuteMethodFromMemory((BYTE*)mem, fs.st_size, DLL_PATH, L"MyLibrary.Math", L"Add", L"1 2");
		result = ExecuteMethodFromMemory((BYTE*)mem, fs.st_size, L"MyLibrary.Show", L"ShowCmd", L"It Works!");
		//result = ExecuteMethodFromMemory((BYTE*)mem, fs.st_size, L"MyLibrary.Show", L"ShowMsg", L"Hello World!");
		if (result)
		{
			std::wcout << "Function successfully." << std::endl;
		}
		free(mem);
	}
	fclose(fd);
}

int wmain(int argc, wchar_t* argv[])
{
	TEST_1();
	TEST_2();
	return 0;
}

#endif
#if (OPTION == 2)

typedef double (*MathFunction)(double, double);
typedef void (*ShowFunction1)();
typedef void (*ShowFunction2)(int);
typedef int (*ShowFunction3)(char*);

int wmain(int argc, wchar_t* argv[])
{
	std::wstring PE_DLL = L"E:\\dev\\SE32-Execute_Library\\ExecuteHelper\\MyLibrary\\bin\\Release\\new\\MyLibrary.dll";
	HINSTANCE hinst = LoadLibraryW(PE_DLL.c_str());
	if (hinst)
	{
#pragma region Math Class
		FARPROC Add = GetProcAddress(hinst, "Add");
		if (NULL != Add)
		{
			MathFunction method = (MathFunction)Add;
			double c = (*method)(5, 5);
			std::wcout << "[Math Class] Add: " << c << std::endl;
		}
		FARPROC Subtract = GetProcAddress(hinst, "Subtract");
		if (NULL != Subtract)
		{
			MathFunction method = (MathFunction)Subtract;
			double c = (*method)(5, 5);
			std::wcout << "[Math Class] Subtract: " << c << std::endl;
		}
		FARPROC Multiply = GetProcAddress(hinst, "Multiply");
		if (NULL != Multiply)
		{
			MathFunction method = (MathFunction)Multiply;
			double c = (*method)(5, 5);
			std::wcout << "[Math Class] Multiply: " << c << std::endl;
		}
		FARPROC Divide = GetProcAddress(hinst, "Divide");
		if (NULL != Divide)
		{
			MathFunction method = (MathFunction)Divide;
			double c = (*method)(5, 5);
			std::wcout << "[Math Class] Divide: " << c << std::endl;
		}
#pragma endregion	//End Math Class
#pragma region Show Class
		FARPROC ShowCmd1 = GetProcAddress(hinst, "ShowCmd1");
		if (NULL != ShowCmd1)
		{
			ShowFunction1 method = (ShowFunction1)ShowCmd1;
			(*method)();
		}
		FARPROC ShowCmd2 = GetProcAddress(hinst, "ShowCmd2");
		if (NULL != ShowCmd2)
		{
			ShowFunction2 method = (ShowFunction2)ShowCmd2;
			(*method)(99);
		}
		FARPROC ShowCmd3 = GetProcAddress(hinst, "ShowCmd3");
		if (NULL != ShowCmd3)
		{
			ShowFunction3 method = (ShowFunction3)ShowCmd3;
			(*method)((char*)"It Work!");
		}

		FARPROC ShowMsg1 = GetProcAddress(hinst, "ShowMsg1");
		if (NULL != ShowMsg1)
		{
			ShowFunction1 method = (ShowFunction1)ShowMsg1;
			(*method)();
		}
		FARPROC ShowMsg2 = GetProcAddress(hinst, "ShowMsg2");
		if (NULL != ShowMsg2)
		{
			ShowFunction2 method = (ShowFunction2)ShowMsg2;
			(*method)(88);
		}
		FARPROC ShowMsg3 = GetProcAddress(hinst, "ShowMsg3");
		if (NULL != ShowMsg3)
		{
			ShowFunction3 method = (ShowFunction3)ShowMsg3;
			(*method)((char*)"It Work!");
		}
#pragma endregion	//End Show Class

		FreeLibrary(hinst);
	}

	return 0;
}

#endif
