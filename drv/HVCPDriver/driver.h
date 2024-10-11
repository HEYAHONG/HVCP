#pragma once


class CMyDriver : public CUnknown, public IDriverEntry
{

private:

    IDriverEntry *QueryIDriverEntry(VOID)
    {
        AddRef();
        return static_cast<IDriverEntry*>(this);
    }

    HRESULT Initialize(VOID);


public:

    static HRESULT CreateInstance(__out PCMyDriver *Driver);

public:
    virtual HRESULT STDMETHODCALLTYPE OnInitialize(__in IWDFDriver *FxWdfDriver)
    {
        UNREFERENCED_PARAMETER( FxWdfDriver );
        return S_OK;
    }

    virtual HRESULT STDMETHODCALLTYPE OnDeviceAdd(__in IWDFDriver *FxWdfDriver,__in IWDFDeviceInitialize *FxDeviceInit);

    virtual VOID STDMETHODCALLTYPE OnDeinitialize(__in IWDFDriver *FxWdfDriver)
    {
        UNREFERENCED_PARAMETER( FxWdfDriver );
        return;
    }


    virtual ULONG STDMETHODCALLTYPE AddRef(VOID)
    {
        return __super::AddRef();
    }

    __drv_arg(this, __drv_freesMem(object))
    virtual ULONG STDMETHODCALLTYPE Release(VOID)
    {
        return __super::Release();
    }

    virtual HRESULT STDMETHODCALLTYPE QueryInterface(__in REFIID InterfaceId,__out PVOID *Object);
};

