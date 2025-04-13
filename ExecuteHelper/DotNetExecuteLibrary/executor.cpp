#include <comdef.h> 
#include <atlbase.h>
#include "executor.h"

// Import mscorlib.tlb (Microsoft Common Language Runtime Class Library).
#import <mscorlib.tlb>	raw_interfaces_only						\
    	high_property_prefixes("_get","_put","_putref")			\
    	rename("ReportEvent", "InteropServices_ReportEvent")	\
		rename("or", "InteropServices_or")

using namespace mscorlib;

BOOL ExecuteMethodFromFile(const std::wstring& assemblyPath, const std::wstring& className, const std::wstring& methodName, const std::wstring& parameters)
{
	HRESULT hr;
	CComPtr<ICLRMetaHost> pMetaHost;
	CComPtr<ICLRRuntimeInfo> pRuntimeInfo;
	CComPtr<ICLRRuntimeHost> pRuntimeHost;

	// Create CLRMetaHost instance
	hr = CLRCreateInstance(CLSID_CLRMetaHost, IID_PPV_ARGS(&pMetaHost));
	if (FAILED(hr))
	{
		LOG_ERROR_W(L"Failed to create CLRMetaHost instance.");
		return FALSE;
	}

	// Get the CLR runtime info for a specific CLR version
	hr = pMetaHost->GetRuntime(CLR_VERSION, IID_PPV_ARGS(&pRuntimeInfo));
	if (FAILED(hr))
	{
		LOG_ERROR_W(L"Failed to get CLR runtime.");
		return FALSE;
	}

	// Check if the CLR runtime is loadable
	BOOL fLoadable;
	hr = pRuntimeInfo->IsLoadable(&fLoadable);
	if (FAILED(hr) || !fLoadable)
	{
		LOG_ERROR_W(L"CLR runtime is not loadable.");
		return FALSE;
	}

	// Get the runtime host interface
	hr = pRuntimeInfo->GetInterface(CLSID_CLRRuntimeHost, IID_ICLRRuntimeHost, (LPVOID*)&pRuntimeHost);
	if (FAILED(hr))
	{
		LOG_ERROR_W(L"Failed to get CLRRuntimeHost interface.");
		return FALSE;
	}

	// Start the CLR runtime
	hr = pRuntimeHost->Start();
	if (FAILED(hr))
	{
		LOG_ERROR_W(L"Failed to start CLR runtime.");
		return FALSE;
	}

	DWORD result;
	// Execute the method in the default AppDomain
	hr = pRuntimeHost->ExecuteInDefaultAppDomain(assemblyPath.c_str(),
												 className.c_str(),
												 methodName.c_str(),
												 parameters.c_str(),
												 &result);
	if (FAILED(hr))
	{
		LOG_ERROR_W(L"Failed to execute method in default AppDomain.");
		switch (hr)
		{
		case E_FAIL:
			LOG_ERROR_W(L"E_FAIL: An unknown catastrophic failure occurred.");
			break;
		case HOST_E_CLRNOTAVAILABLE:
			LOG_ERROR_W(L"HOST_E_CLRNOTAVAILABLE: The CLR has not been loaded.");
			break;
		case HOST_E_TIMEOUT:
			LOG_ERROR_W(L"HOST_E_TIMEOUT: The call timed out.");
			break;
		case HOST_E_NOT_OWNER:
			LOG_ERROR_W(L"HOST_E_NOT_OWNER: The caller does not own the lock.");
			break;
		case HOST_E_ABANDONED:
			LOG_ERROR_W(L"HOST_E_ABANDONED: An event was canceled.");
			break;
		case COR_E_MISSINGMETHOD:
			LOG_ERROR_W(L"COR_E_MISSINGMETHOD: Method does not exist.");
			break;
		default:
			LOG_ERROR_W(L"Unknown error code: %d", (int)hr);
			break;
		}
		return FALSE;
	}

	return TRUE;
}


BOOL ExecuteMethodFromMemory(const BYTE* assemblyPtr, size_t assemblyLen, const std::wstring& className, const std::wstring& methodName, const std::wstring& parameters)
{
	HRESULT hr;
	CComPtr<ICLRMetaHost> pMetaHost;
	CComPtr<ICLRRuntimeInfo> pRuntimeInfo;
	CComPtr<ICorRuntimeHost> pCorRuntimeHost;

	hr = CLRCreateInstance(CLSID_CLRMetaHost, IID_PPV_ARGS(&pMetaHost));
	if (FAILED(hr))
	{
		LOG_ERROR_W(L"Failed to create CLRMetaHost instance.");
		return FALSE;
	}
	// Get the ICLRRuntimeInfo corresponding to a particular CLR version.
	hr = pMetaHost->GetRuntime(CLR_VERSION, IID_PPV_ARGS(&pRuntimeInfo));
	if (FAILED(hr))
	{
		LOG_ERROR_W(L"Failed to get CLR runtime.");
		return FALSE;
	}
	// Check if the specified runtime can be loaded into the process. 
	BOOL fLoadable;
	hr = pRuntimeInfo->IsLoadable(&fLoadable);
	if (FAILED(hr))
	{
		LOG_ERROR_W(L"Failed to check if runtime is loadable.");
		return FALSE;
	}
	if (!fLoadable)
	{
		LOG_ERROR_W(L"Runtime is not loadable.");
		return FALSE;
	}
	// Load the CLR into the current process and return a runtime interface pointer.
	hr = pRuntimeInfo->GetInterface(CLSID_CorRuntimeHost, IID_PPV_ARGS(&pCorRuntimeHost));
	if (FAILED(hr))
	{
		LOG_ERROR_W(L"Failed to get CorRuntimeHost interface.");
		return FALSE;
	}
	// Start the CLR
	hr = pCorRuntimeHost->Start();
	if (FAILED(hr))
	{
		LOG_ERROR_W(L"Failed to start CLR runtime.");
		return FALSE;
	}

	// Get a pointer to the default AppDomain in the CLR.
	IUnknownPtr spAppDomainThunk = NULL;
	hr = pCorRuntimeHost->GetDefaultDomain(&spAppDomainThunk);
	if (FAILED(hr))
	{
		LOG_ERROR_W(L"Failed to get default AppDomain.");
		return FALSE;
	}
	_AppDomainPtr spDefaultAppDomain = NULL;
	hr = spAppDomainThunk->QueryInterface(IID_PPV_ARGS(&spDefaultAppDomain));
	if (FAILED(hr))
	{
		LOG_ERROR_W(L"Failed to query default AppDomain interface.");
		return FALSE;
	}

	// Load the assembly from memory, declared as an unsigned char array
	SAFEARRAYBOUND bounds[1];
	bounds[0].cElements = assemblyLen;
	bounds[0].lLbound = 0;

	SAFEARRAY* arr = SafeArrayCreate(VT_UI1, 1, bounds);
	SafeArrayLock(arr);
	memcpy(arr->pvData, assemblyPtr, assemblyLen);
	SafeArrayUnlock(arr);

	_AssemblyPtr spAssembly = NULL;
	hr = spDefaultAppDomain->Load_3(arr, &spAssembly);
	if (FAILED(hr))
	{
		LOG_ERROR_W(L"Failed to load assembly from memory.");
		return FALSE;
	}

	// Get the Type (ie: Namespace and Class type) to be instantiated from the assembly
	bstr_t bstrClassName(className.c_str());
	_TypePtr spType = NULL;

	hr = spAssembly->GetType_2(bstrClassName, &spType);
	if (FAILED(hr))
	{
		LOG_ERROR_W(L"Failed to get type from assembly.");
		return FALSE;
	}

	// Finally, invoke the method passing it some arguments as a single string
	bstr_t bstrStaticMethodName(methodName.c_str());
	SAFEARRAY* psaStaticMethodArgs = NULL;
	variant_t vtStringArg(parameters.c_str());
	variant_t vtPSEntryPointReturnVal;
	variant_t vtEmpty;

	LONG index = 0;
	psaStaticMethodArgs = SafeArrayCreateVector(VT_VARIANT, 0, 1);
	hr = SafeArrayPutElement(psaStaticMethodArgs, &index, &vtStringArg);
	if (FAILED(hr))
	{
		LOG_ERROR_W(L"Failed to put argument into SafeArray.");
		return FALSE;
	}

	// Invoke the method from the Type interface.
	hr = spType->InvokeMember_3(bstrStaticMethodName,
		static_cast<BindingFlags>(BindingFlags_InvokeMethod | BindingFlags_Static | BindingFlags_Public),
		NULL,
		vtEmpty,
		psaStaticMethodArgs,
		&vtPSEntryPointReturnVal);
	if (FAILED(hr))
	{
		LOG_ERROR_W(L"Failed to invoke method from type.");
		return FALSE;
	}
	SafeArrayDestroy(psaStaticMethodArgs);
	psaStaticMethodArgs = NULL;
	return TRUE;
}

