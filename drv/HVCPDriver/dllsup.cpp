#include "internal.h"
#include "dllsup.tmh"

const GUID CLSID_MyDriverCoClass = MYDRIVER_CLASS_ID;

BOOL WINAPI DllMain(HINSTANCE ModuleHandle,DWORD Reason,PVOID)
{

    UNREFERENCED_PARAMETER( ModuleHandle );

    if (DLL_PROCESS_ATTACH == Reason)
    {
        //
        // Initialize tracing.
        //

        WPP_INIT_TRACING(MYDRIVER_TRACING_ID);

    }
    else if (DLL_PROCESS_DETACH == Reason)
    {
        //
        // Cleanup tracing.
        //

        WPP_CLEANUP();
    }

    return TRUE;
}

__control_entrypoint(DllExport)
HRESULT STDAPICALLTYPE DllGetClassObject(__in REFCLSID ClassId,__in REFIID InterfaceId,__deref_out LPVOID *Interface)
{
    PCClassFactory factory;

    HRESULT hr = S_OK;

    *Interface = NULL;

    //
    // If the CLSID doesn't match that of our "coclass" (defined in the IDL
    // file) then we can't create the object the caller wants.  This may
    // indicate that the COM registration is incorrect, and another CLSID
    // is referencing this drvier.
    //

    if (IsEqualCLSID(ClassId, CLSID_MyDriverCoClass) == false)
    {
        return CLASS_E_CLASSNOTAVAILABLE;
    }

    //
    // Create an instance of the class factory for the caller.
    //

    factory = new CClassFactory();

    if (NULL == factory)
    {
        hr = E_OUTOFMEMORY;
    }

    //
    // Query the object we created for the interface the caller wants.  After
    // that we release the object.  This will drive the reference count to
    // 1 (if the QI succeeded an referenced the object) or 0 (if the QI failed).
    // In the later case the object is automatically deleted.
    //

    if (SUCCEEDED(hr))
    {
        hr = factory->QueryInterface(InterfaceId, Interface);
        factory->Release();
    }

    return hr;
}

