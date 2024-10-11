#pragma once

#include "internal.h"
#include "time.h"

#define DATA_BUFFER_SIZE 1024
#define MAX_OPEN_HANDLE_QUANTITY 32


#define CTL_CODE( DeviceType, Function, Method, Access ) (                 \
    ((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method) \
)

class CMyQueueHandle:
    public IObjectCleanup,
    public CUnknown
{
public:
    LONG64  pid;
    time_t last_read;
    CRingBuffer    m_RingBuffer;
    CMyQueueHandle():
        pid(-1),
        last_read(0)
    {

    }
    ~CMyQueueHandle()
    {

    }
    HRESULT Initialize()
    {
        return m_RingBuffer.Initialize(DATA_BUFFER_SIZE);
    }
    virtual VOID STDMETHODCALLTYPE OnCleanup(__in IWDFObject *pWdfObject)
    {
        if(pWdfObject!=NULL)
        {
            pid=-1;
        }
    }
    virtual ULONG STDMETHODCALLTYPE AddRef(VOID)
    {
        return CUnknown::AddRef();
    }
    __drv_arg(this, __drv_freesMem(object))
    virtual ULONG STDMETHODCALLTYPE Release(VOID)
    {
        return CUnknown::Release();
    }
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(__in REFIID InterfaceId,__out PVOID *Object)
    {
        HRESULT hr=S_OK;
        if(IsEqualIID(InterfaceId, __uuidof(IObjectCleanup)))
        {
            hr=S_OK;
            (*Object)=static_cast<IObjectCleanup*>(this);
        }
        else
        {
            hr=CUnknown::QueryInterface(InterfaceId, Object);
        }
        return hr;
    }
};


class CMyQueue :
    public IQueueCallbackCreate,
    public IQueueCallbackDeviceIoControl,
    public IQueueCallbackRead,
    public IQueueCallbackWrite,
    public IFileCallbackCleanup,
    public CUnknown
{

    IWDFIoQueue    *m_FxQueue;
    IWDFIoQueue    *m_FxReadQueue;
    CMyQueueHandle Handle[MAX_OPEN_HANDLE_QUANTITY];


    PCMyDevice     m_Device;

    CMyQueue(CMyDevice *pDevice) :
        m_FxQueue(NULL),
        m_FxReadQueue(NULL)
    {
        WUDF_TEST_DRIVER_ASSERT(pDevice);
        pDevice->AddRef();
        m_Device = pDevice;
    }

    virtual ~CMyQueue();

    HRESULT Initialize(__in IWDFDevice *FxDevice);

    VOID CMyQueue::ProcessWriteBytes(__in_bcount(Length) PUCHAR Characters,__in SIZE_T Length,__in ULONG pid);

    CMyQueueHandle * GetHandleByPid(ULONG pid)
    {
        CMyQueueHandle * default_handle=&Handle[0];
        for(int i=0; i<sizeof(Handle)/sizeof(Handle[0]); i++)
        {
            if(Handle[i].pid==pid)
            {
                return &Handle[i];
            }
            if(Handle[i].pid==-1)
            {
                Handle[i].pid=pid;
                return &Handle[i];
            }
            else
            {
                if(default_handle->last_read < Handle[i].last_read)
                {
                    default_handle= &Handle[i];
                }
            }
        }
        return default_handle;
    }

public:

    static HRESULT CreateInstance(__in  CMyDevice *pDevice,__in IWDFDevice *FxDevice, __out PCMyQueue *Queue);

    HRESULT Configure(VOID)
    {
        return S_OK;
    }


    IQueueCallbackCreate * QueryIQueueCallbackCreate(VOID)
    {
        AddRef();
        return static_cast<IQueueCallbackCreate *>(this);
    }

    IQueueCallbackDeviceIoControl * QueryIQueueCallbackDeviceIoControl(VOID)
    {
        AddRef();
        return static_cast<IQueueCallbackDeviceIoControl *>(this);
    }

    IQueueCallbackRead * QueryIQueueCallbackRead(VOID)
    {
        AddRef();
        return static_cast<IQueueCallbackRead *>(this);
    }

    IQueueCallbackWrite * QueryIQueueCallbackWrite(VOID)
    {
        AddRef();
        return static_cast<IQueueCallbackWrite *>(this);
    }
    IFileCallbackCleanup * QueryIFileCallbackCleanup(VOID)
    {
        AddRef();
        return static_cast<IFileCallbackCleanup *>(this);
    }


    virtual ULONG STDMETHODCALLTYPE AddRef(VOID)
    {
        return CUnknown::AddRef();
    }

    __drv_arg(this, __drv_freesMem(object))
    virtual ULONG STDMETHODCALLTYPE Release(VOID)
    {
        return CUnknown::Release();
    }



    virtual HRESULT STDMETHODCALLTYPE QueryInterface(__in REFIID InterfaceId,__out PVOID *Object);
    virtual VOID STDMETHODCALLTYPE OnCreateFile( __in IWDFIoQueue   *pWdfQueue,__in IWDFIoRequest *pWDFRequest,__in IWDFFile      *pWdfFileObject);
    virtual VOID STDMETHODCALLTYPE OnDeviceIoControl(__in IWDFIoQueue *pWdfQueue,__in IWDFIoRequest *pWdfRequest,__in ULONG ControlCode,__in SIZE_T InputBufferSizeInBytes,__in SIZE_T OutputBufferSizeInBytes);
    virtual VOID STDMETHODCALLTYPE OnWrite(__in IWDFIoQueue *pWdfQueue,__in IWDFIoRequest *pWdfRequest,__in SIZE_T NumOfBytesToWrite);
    virtual VOID STDMETHODCALLTYPE OnRead(__in IWDFIoQueue *pWdfQueue,__in IWDFIoRequest *pWdfRequest,__in SIZE_T NumOfBytesToRead);
    virtual VOID STDMETHODCALLTYPE OnCleanupFile(__in IWDFFile *pWdfFileObject);
};

