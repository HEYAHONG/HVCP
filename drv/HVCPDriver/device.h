#pragma once
#include "initguid.h"
#include "serial.h"

DEFINE_GUID(GUID_DEVINTERFACE_MODEM,0x2c7089aa, 0x2e0e,0x11d1,0xb1, 0x14, 0x00, 0xc0, 0x4f, 0xc2, 0xaa, 0xe4);

#define SYMBOLIC_LINK_NAME_PREFIX   L"\\DosDevices\\Global\\"


class CMyDevice :public CUnknown,public IObjectCleanup
{
private:

    IWDFDevice *m_FxDevice;

    ULONG m_BaudRate;

    ULONG m_MCR;

    ULONG m_FCR;

    ULONG m_LCR;

    UCHAR m_ValidDataMask;

    SERIAL_TIMEOUTS m_Timeouts;

    BOOL m_CreatedLegacyHardwareKey;

    IWDFNamedPropertyStore2 * m_LegacyHardwarePropertyStore;

    PWCHAR m_PdoName;

private:

    CMyDevice(VOID) :
        m_FxDevice(NULL),
        m_BaudRate(0),
        m_MCR(0),
        m_FCR(0),
        m_LCR(0),
        m_ValidDataMask(0),
        m_CreatedLegacyHardwareKey(FALSE),
        m_LegacyHardwarePropertyStore(NULL),
        m_PdoName(NULL)
    {
        ZeroMemory(&m_Timeouts, sizeof(SERIAL_TIMEOUTS));
    }

    HRESULT Initialize(__in IWDFDriver *FxDriver,__in IWDFDeviceInitialize *FxDeviceInit);

    HRESULT CMyDevice::GetPdoName(__out WCHAR **PdoName);
public:

    static HRESULT CreateInstance(__in IWDFDriver *FxDriver,__in IWDFDeviceInitialize *FxDeviceInit,__out PCMyDevice *Device);

    HRESULT Configure(VOID);

public:
    virtual ULONG STDMETHODCALLTYPE AddRef(VOID)
    {
        return __super::AddRef();
    }

    __drv_arg(this, __drv_freesMem(object))
    virtual ULONG STDMETHODCALLTYPE  Release(VOID)
    {
        return __super::Release();
    }

    virtual HRESULT STDMETHODCALLTYPE QueryInterface(__in REFIID InterfaceId,__out PVOID *Object);

    IObjectCleanup * QueryIObjectCleanup(VOID)
    {
        AddRef();
        return static_cast<IObjectCleanup *>(this);
    }

    virtual void STDMETHODCALLTYPE OnCleanup(IWDFObject*  pWdfObject);


    ULONG GetBaudRate(VOID)
    {
        return m_BaudRate;
    }

    void SetBaudRate(__in ULONG BaudRate)
    {
        m_BaudRate = BaudRate;
        return;
    }

    ULONG * GetModemControlRegisterPtr(VOID)
    {
        return &m_MCR;
    }

    ULONG * GetFifoControlRegisterPtr(VOID)
    {
        return &m_FCR;
    }

    ULONG * GetLineControlRegisterPtr(VOID)
    {
        return &m_LCR;
    }

    VOID SetValidDataMask(__in UCHAR Mask)
    {
        m_ValidDataMask = Mask;
    }

    VOID SetTimeouts(__in SERIAL_TIMEOUTS Timeouts)
    {
        m_Timeouts = Timeouts;
    }

    VOID GetTimeouts(__out SERIAL_TIMEOUTS *Timeouts)
    {
        *Timeouts = m_Timeouts;;
    }

};
