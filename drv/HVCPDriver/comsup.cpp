#include "internal.h"

#include "comsup.tmh"


CUnknown::CUnknown(VOID) : m_ReferenceCount(1)
{

}

HRESULT STDMETHODCALLTYPE CUnknown::QueryInterface(__in REFIID InterfaceId,__out PVOID *Object)
{
    if (IsEqualIID(InterfaceId, __uuidof(IUnknown)))
    {
        *Object = QueryIUnknown();
        return S_OK;
    }
    else
    {
        *Object = NULL;
        return E_NOINTERFACE;
    }
}

IUnknown * CUnknown::QueryIUnknown(VOID)
{
    AddRef();
    return static_cast<IUnknown *>(this);
}

ULONG STDMETHODCALLTYPE CUnknown::AddRef(VOID)
{
    return InterlockedIncrement(&m_ReferenceCount);
}

ULONG STDMETHODCALLTYPE CUnknown::Release(VOID)
{
    ULONG count = InterlockedDecrement(&m_ReferenceCount);

    if (count == 0)
    {
        delete this;
    }
    return count;
}

LONG CClassFactory::s_LockCount = 0;

IClassFactory * CClassFactory::QueryIClassFactory(VOID)
{
    AddRef();
    return static_cast<IClassFactory *>(this);
}

HRESULT CClassFactory::QueryInterface(__in REFIID InterfaceId,__out PVOID *Object)
{
    //
    // This class only supports IClassFactory so check for that.
    //

    if (IsEqualIID(InterfaceId, __uuidof(IClassFactory)))
    {
        *Object = QueryIClassFactory();
        return S_OK;
    }
    else
    {
        //
        // See if the base class supports the interface.
        //

        return CUnknown::QueryInterface(InterfaceId, Object);
    }
}

HRESULT STDMETHODCALLTYPE CClassFactory::CreateInstance(__in_opt IUnknown *,__in REFIID InterfaceId,__out PVOID *Object)
{
    HRESULT hr;

    PCMyDriver driver;

    *Object = NULL;

    hr = CMyDriver::CreateInstance(&driver);

    if (SUCCEEDED(hr))
    {
        hr = driver->QueryInterface(InterfaceId, Object);
        driver->Release();
    }

    return hr;
}

HRESULT STDMETHODCALLTYPE CClassFactory::LockServer(__in BOOL Lock)
{
    if (Lock)
    {
        InterlockedIncrement(&s_LockCount);
    }
    else
    {
        InterlockedDecrement(&s_LockCount);
    }
    return S_OK;
}


