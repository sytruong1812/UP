#pragma once
#include <string>
#include <windows.h>
#include <corerror.h>
#include <metahost.h>
#pragma comment(lib, "mscoree.lib")
#include "TraceLogger.h"

// Define CLR Versions
#define CLR_VERSION_V1_0     L"v1.0.3705"
#define CLR_VERSION_V1_1     L"v1.1.4322"
#define CLR_VERSION_V2_0     L"v2.0.50727"
#define CLR_VERSION_V4_0     L"v4.0.30319"
#define CLR_VERSION CLR_VERSION_V4_0   // Choose CLR version for your application

/// <summary>
/// Loads a .NET assembly from a file on disk and executes a static method in the specified class.
/// </summary>
/// <param name="assemblyPath: ">Full path to the .NET assembly file (.dll or .exe).</param>
/// <param name="className: ">Fully qualified class name including namespace (e.g., "MyNamespace.MyClass").</param>
/// <param name="methodName: ">Name of the static method to invoke.</param>
/// <param name="parameters: ">A string parameter to pass to the target method.</param>
/// <returns>Returns TRUE if the method is executed successfully, or FALSE on failure.</returns>
BOOL ExecuteMethodFromFile(const std::wstring& assemblyPath, const std::wstring& className, const std::wstring& methodName, const std::wstring& parameters);

/// <summary>
/// Loads a .NET assembly from a memory buffer and executes a static method in the specified class.
/// </summary>
/// <param name="assemblyPtr: ">Pointer to the memory buffer containing the .NET assembly bytes.</param>
/// <param name="assemblyLen: ">Size of the memory buffer in bytes.</param>
/// <param name="className: ">Fully qualified class name including namespace (e.g., "MyNamespace.MyClass").</param>
/// <param name="methodName: ">Name of the static method to invoke.</param>
/// <param name="parameters: ">A string parameter to pass to the target method.</param>
/// <returns>Returns TRUE if the method is executed successfully, or FALSE on failure.</returns>
BOOL ExecuteMethodFromMemory(const BYTE* assemblyPtr, size_t assemblyLen, const std::wstring& className, const std::wstring& methodName, const std::wstring& parameters);
