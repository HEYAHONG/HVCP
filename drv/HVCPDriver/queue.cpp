#include "internal.h"
CMyQueue::~CMyQueue(VOID)
{
    WUDF_TEST_DRIVER_ASSERT(m_Device);

    m_Device->Release();
}


HRESULT CMyQueue::CreateInstance(__in  CMyDevice *pDevice,__in IWDFDevice *FxDevice,__out PCMyQueue *Queue)
{
    CMyQueue *pMyQueue = new CMyQueue(pDevice);
    HRESULT hr;

    if (pMyQueue == NULL)
    {
        return E_OUTOFMEMORY;
    }

    hr = pMyQueue->Initialize(FxDevice);

    if (SUCCEEDED(hr))
    {
        *Queue = pMyQueue;
    }
    else
    {
        pMyQueue->Release();
    }
    return hr;
}

HRESULT CMyQueue::Initialize(__in IWDFDevice *FxDevice)
{
    IWDFIoQueue *fxQueue;
    IUnknown *unknown = NULL;
    HRESULT hr;

    for(int i=0; i<sizeof(Handle)/sizeof(Handle[0]); i++)
    {
        hr = Handle[i].Initialize();

        if (FAILED(hr))
        {
            goto Exit;
        }
    }

    unknown = QueryIUnknown();

    //
    // Create the default queue
    //

    {
        hr = FxDevice->CreateIoQueue(unknown,
                                     TRUE,
                                     WdfIoQueueDispatchParallel,
                                     TRUE,
                                     FALSE,
                                     &fxQueue);
    }

    if (FAILED(hr))
    {
        goto Exit;
    }

    m_FxQueue = fxQueue;

    fxQueue->Release();

    //
    // Create a manual queue to hold pending read requests. By keeping
    // them in the queue, framework takes care of cancelling them if the app
    // exits
    //

    {
        hr = FxDevice->CreateIoQueue(NULL,
                                     FALSE,
                                     WdfIoQueueDispatchManual,
                                     TRUE,
                                     FALSE,
                                     &fxQueue);
    }

    if (FAILED(hr))
    {
        goto Exit;
    }

    m_FxReadQueue = fxQueue;

    fxQueue->Release();

Exit:
    SAFE_RELEASE(unknown);
    return hr;
}

HRESULT STDMETHODCALLTYPE CMyQueue::QueryInterface(__in REFIID InterfaceId,__out PVOID *Object)
{
    HRESULT hr;

    if (IsEqualIID(InterfaceId, __uuidof(IFileCallbackCleanup)))
    {
        *Object = QueryIFileCallbackCleanup();
        hr = S_OK;
    }
    else if (IsEqualIID(InterfaceId, __uuidof(IQueueCallbackCreate)))
    {
        *Object = QueryIQueueCallbackCreate();
        hr = S_OK;
    }
    else if (IsEqualIID(InterfaceId, __uuidof(IQueueCallbackWrite)))
    {
        *Object = QueryIQueueCallbackWrite();
        hr = S_OK;
    }
    else if (IsEqualIID(InterfaceId, __uuidof(IQueueCallbackRead)))
    {
        *Object = QueryIQueueCallbackRead();
        hr = S_OK;
    }
    else if (IsEqualIID(InterfaceId, __uuidof(IQueueCallbackDeviceIoControl)))
    {
        *Object = QueryIQueueCallbackDeviceIoControl();
        hr = S_OK;
    }
    else
    {
        hr = CUnknown::QueryInterface(InterfaceId, Object);
    }

    return hr;
}

VOID STDMETHODCALLTYPE CMyQueue::OnCreateFile( __in IWDFIoQueue   *pWdfQueue,__in IWDFIoRequest *pWDFRequest,__in IWDFFile      *pWdfFileObject)
{
    WUDF_TEST_DRIVER_ASSERT(pWdfQueue);
    WUDF_TEST_DRIVER_ASSERT(pWDFRequest);
    WUDF_TEST_DRIVER_ASSERT(pWdfFileObject);
    HRESULT hr = S_OK;
    pWDFRequest->Complete(hr);
}

VOID STDMETHODCALLTYPE CMyQueue::OnDeviceIoControl(__in IWDFIoQueue *pWdfQueue,__in IWDFIoRequest *pWdfRequest,__in ULONG ControlCode,__in SIZE_T InputBufferSizeInBytes,__in SIZE_T OutputBufferSizeInBytes)
{
    UNREFERENCED_PARAMETER(OutputBufferSizeInBytes);
    UNREFERENCED_PARAMETER(InputBufferSizeInBytes);
    UNREFERENCED_PARAMETER(pWdfQueue);

    HRESULT hr = S_OK;
    SIZE_T reqCompletionInfo = 0;
    IWDFMemory *inputMemory = NULL;
    IWDFMemory *outputMemory = NULL;
    UINT i;

    WUDF_TEST_DRIVER_ASSERT(pWdfRequest);
    WUDF_TEST_DRIVER_ASSERT(m_Device);

    switch (ControlCode)
    {
    case IOCTL_SERIAL_SET_BAUD_RATE:
    {
        //
        // This is a driver for a virtual serial port. Since there is no
        // actual hardware, we just store the baud rate and don't do
        // anything with it.
        //
        SERIAL_BAUD_RATE baudRateBuffer;
        ZeroMemory(&baudRateBuffer, sizeof(SERIAL_BAUD_RATE));

        pWdfRequest->GetInputMemory(&inputMemory);
        if (NULL == inputMemory)
        {
            hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        }

        if (SUCCEEDED(hr))
        {
            hr = inputMemory->CopyToBuffer(0,
                                           (void*) &baudRateBuffer,
                                           sizeof(SERIAL_BAUD_RATE));
        }

        if (SUCCEEDED(hr))
        {
            m_Device->SetBaudRate(baudRateBuffer.BaudRate);
        }

        break;
    }
    case IOCTL_SERIAL_GET_BAUD_RATE:
    {
        SERIAL_BAUD_RATE baudRateBuffer;
        ZeroMemory(&baudRateBuffer, sizeof(SERIAL_BAUD_RATE));

        baudRateBuffer.BaudRate = m_Device->GetBaudRate();

        pWdfRequest->GetOutputMemory(&outputMemory);
        if (NULL == outputMemory)
        {
            hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        }

        if (SUCCEEDED(hr))
        {
            hr = outputMemory->CopyFromBuffer(0,
                                              (void*) &baudRateBuffer,
                                              sizeof(SERIAL_BAUD_RATE));
        }

        if (SUCCEEDED(hr))
        {
            reqCompletionInfo = sizeof(SERIAL_BAUD_RATE);
        }

        break;
    }
    case IOCTL_SERIAL_SET_MODEM_CONTROL:
    {
        //
        // This is a driver for a virtual serial port. Since there is no
        // actual hardware, we just store the modem control register
        // configuration and don't do anything with it.
        //
        ULONG *pModemControlRegister = NULL;

        pWdfRequest->GetInputMemory(&inputMemory);
        if (NULL == inputMemory)
        {
            hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        }

        if (SUCCEEDED(hr))
        {
            pModemControlRegister = m_Device->GetModemControlRegisterPtr();
            WUDF_TEST_DRIVER_ASSERT(pModemControlRegister);

            hr = inputMemory->CopyToBuffer(0,
                                           (void*) pModemControlRegister,
                                           sizeof(ULONG));
        }

        break;
    }
    case IOCTL_SERIAL_GET_MODEM_CONTROL:
    {
        ULONG *pModemControlRegister = NULL;

        pWdfRequest->GetOutputMemory(&outputMemory);
        if (NULL == outputMemory)
        {
            hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        }

        if (SUCCEEDED(hr))
        {
            pModemControlRegister = m_Device->GetModemControlRegisterPtr();
            WUDF_TEST_DRIVER_ASSERT(pModemControlRegister);

            hr = outputMemory->CopyFromBuffer(0,
                                              (void*) pModemControlRegister,
                                              sizeof(ULONG));
        }

        if (SUCCEEDED(hr))
        {
            reqCompletionInfo = sizeof(ULONG);
        }

        break;
    }
    case IOCTL_SERIAL_SET_FIFO_CONTROL:
    {
        //
        // This is a driver for a virtual serial port. Since there is no
        // actual hardware, we just store the FIFO control register
        // configuration and don't do anything with it.
        //
        ULONG *pFifoControlRegister = NULL;

        pWdfRequest->GetInputMemory(&inputMemory);
        if (NULL == inputMemory)
        {
            hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        }

        if (SUCCEEDED(hr))
        {
            pFifoControlRegister = m_Device->GetFifoControlRegisterPtr();

            hr = inputMemory->CopyToBuffer(0,
                                           (void*) pFifoControlRegister,
                                           sizeof(ULONG));
        }

        break;
    }
    case IOCTL_SERIAL_GET_LINE_CONTROL:
    {
        ULONG *pLineControlRegister = NULL;
        SERIAL_LINE_CONTROL lineControl;
        ULONG lineControlSnapshot;

        ZeroMemory(&lineControl, sizeof(SERIAL_LINE_CONTROL));

        pLineControlRegister = m_Device->GetLineControlRegisterPtr();
        WUDF_TEST_DRIVER_ASSERT(pLineControlRegister);

        //
        // Take a snapshot of the line control register variable
        //
        lineControlSnapshot = *pLineControlRegister;

        //
        // Decode the word length
        //
        if ((lineControlSnapshot & SERIAL_DATA_MASK) == SERIAL_5_DATA)
        {
            lineControl.WordLength = 5;
        }
        else if ((lineControlSnapshot & SERIAL_DATA_MASK) == SERIAL_6_DATA)
        {
            lineControl.WordLength = 6;
        }
        else if ((lineControlSnapshot & SERIAL_DATA_MASK) == SERIAL_7_DATA)
        {
            lineControl.WordLength = 7;
        }
        else if ((lineControlSnapshot & SERIAL_DATA_MASK) == SERIAL_8_DATA)
        {
            lineControl.WordLength = 8;
        }

        //
        // Decode the parity
        //
        if ((lineControlSnapshot & SERIAL_PARITY_MASK) == SERIAL_NONE_PARITY)
        {
            lineControl.Parity = NO_PARITY;
        }
        else if ((lineControlSnapshot & SERIAL_PARITY_MASK) == SERIAL_ODD_PARITY)
        {
            lineControl.Parity = ODD_PARITY;
        }
        else if ((lineControlSnapshot & SERIAL_PARITY_MASK) == SERIAL_EVEN_PARITY)
        {
            lineControl.Parity = EVEN_PARITY;
        }
        else if ((lineControlSnapshot & SERIAL_PARITY_MASK) == SERIAL_MARK_PARITY)
        {
            lineControl.Parity = MARK_PARITY;
        }
        else if ((lineControlSnapshot & SERIAL_PARITY_MASK) == SERIAL_SPACE_PARITY)
        {
            lineControl.Parity = SPACE_PARITY;
        }

        //
        // Decode the length of the stop bit
        //
        if (lineControlSnapshot & SERIAL_2_STOP)
        {
            if (lineControl.WordLength == 5)
            {
                lineControl.StopBits = STOP_BITS_1_5;
            }
            else
            {
                lineControl.StopBits = STOP_BITS_2;
            }
        }
        else
        {
            lineControl.StopBits = STOP_BIT_1;
        }

        //
        // Copy the information that was decoded to the caller's buffer
        //
        pWdfRequest->GetOutputMemory(&outputMemory);
        if (NULL == outputMemory)
        {
            hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        }

        if (SUCCEEDED(hr))
        {
            hr = outputMemory->CopyFromBuffer(0,
                                              (void*) &lineControl,
                                              sizeof(SERIAL_LINE_CONTROL));
        }

        if (SUCCEEDED(hr))
        {
            reqCompletionInfo = sizeof(SERIAL_LINE_CONTROL);
        }

        break;
    }
    case IOCTL_SERIAL_SET_LINE_CONTROL:
    {
        ULONG *pLineControlRegister = NULL;
        SERIAL_LINE_CONTROL lineControl;
        UCHAR lineControlData = 0;
        UCHAR lineControlStop = 0;
        UCHAR lineControlParity = 0;
        ULONG lineControlSnapshot;
        ULONG lineControlNew;
        ULONG lineControlPrevious;

        ZeroMemory(&lineControl, sizeof(SERIAL_LINE_CONTROL));

        pLineControlRegister = m_Device->GetLineControlRegisterPtr();
        WUDF_TEST_DRIVER_ASSERT(pLineControlRegister);

        //
        // This is a driver for a virtual serial port. Since there is no
        // actual hardware, we just store the line control register
        // configuration and don't do anything with it.
        //
        pWdfRequest->GetInputMemory(&inputMemory);
        if (NULL == inputMemory)
        {
            hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        }

        if (SUCCEEDED(hr))
        {
            hr = inputMemory->CopyToBuffer(0,
                                           (void*) &lineControl,
                                           sizeof(SERIAL_LINE_CONTROL));
        }

        //
        // Bits 0 and 1 of the line control register
        //
        if (SUCCEEDED(hr))
        {
            switch (lineControl.WordLength)
            {
            case 5:
                lineControlData = SERIAL_5_DATA;
                m_Device->SetValidDataMask(0x1f);
                break;

            case 6:
                lineControlData = SERIAL_6_DATA;
                m_Device->SetValidDataMask(0x3f);
                break;

            case 7:
                lineControlData = SERIAL_7_DATA;
                m_Device->SetValidDataMask(0x7f);
                break;

            case 8:
                lineControlData = SERIAL_8_DATA;
                m_Device->SetValidDataMask(0xff);
                break;

            default:
                hr = E_INVALIDARG;
            }
        }

        //
        // Bit 2 of the line control register
        //
        if (SUCCEEDED(hr))
        {
            switch (lineControl.StopBits)
            {
            case STOP_BIT_1:
                lineControlStop = SERIAL_1_STOP;
                break;

            case STOP_BITS_1_5:
                if (lineControlData != SERIAL_5_DATA)
                {
                    hr = E_INVALIDARG;
                    break;
                }
                lineControlStop = SERIAL_1_5_STOP;
                break;

            case STOP_BITS_2:
                if (lineControlData == SERIAL_5_DATA)
                {
                    hr = E_INVALIDARG;
                    break;
                }
                lineControlStop = SERIAL_2_STOP;
                break;

            default:
                hr = E_INVALIDARG;
            }
        }

        //
        // Bits 3, 4 and 5 of the line control register
        //
        if (SUCCEEDED(hr))
        {
            switch (lineControl.Parity)
            {
            case NO_PARITY:
                lineControlParity = SERIAL_NONE_PARITY;
                break;

            case EVEN_PARITY:
                lineControlParity = SERIAL_EVEN_PARITY;
                break;

            case ODD_PARITY:
                lineControlParity = SERIAL_ODD_PARITY;
                break;

            case SPACE_PARITY:
                lineControlParity = SERIAL_SPACE_PARITY;
                break;

            case MARK_PARITY:
                lineControlParity = SERIAL_MARK_PARITY;
                break;

            default:
                hr = E_INVALIDARG;
            }
        }

        //
        // Update our line control register variable atomically
        //
        i=0;
        do
        {
            i++;
            if ((i & 0xf) == 0)
            {
                //
                // We've been spinning in a loop for a while trying to
                // update the line control register variable atomically.
                // Yield the CPU for other threads for a while.
                //
                SwitchToThread();
            }

            lineControlSnapshot = *pLineControlRegister;

            lineControlNew = (lineControlSnapshot & SERIAL_LCR_BREAK) |
                             (lineControlData |
                              lineControlParity |
                              lineControlStop);

            lineControlPrevious = InterlockedCompareExchange((LONG *) pLineControlRegister,
                                  lineControlNew,
                                  lineControlSnapshot);

        }
        while (lineControlPrevious != lineControlSnapshot);

        break;
    }
    case IOCTL_SERIAL_GET_TIMEOUTS:
    {
        SERIAL_TIMEOUTS timeoutValues;

        pWdfRequest->GetOutputMemory(&outputMemory);
        if (NULL == outputMemory)
        {
            hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        }

        if (SUCCEEDED(hr))
        {
            m_Device->GetTimeouts(&timeoutValues);

            hr = outputMemory->CopyFromBuffer(0,
                                              (void*) &timeoutValues,
                                              sizeof(timeoutValues));
        }

        if (SUCCEEDED(hr))
        {
            reqCompletionInfo = sizeof(SERIAL_TIMEOUTS);
        }

        break;
    }
    case IOCTL_SERIAL_SET_TIMEOUTS:
    {
        SERIAL_TIMEOUTS timeoutValues;

        ZeroMemory(&timeoutValues, sizeof(SERIAL_LINE_CONTROL));

        pWdfRequest->GetInputMemory(&inputMemory);
        if (NULL == inputMemory)
        {
            hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        }

        if (SUCCEEDED(hr))
        {
            hr = inputMemory->CopyToBuffer(0,
                                           (void*) &timeoutValues,
                                           sizeof(timeoutValues));
        }

        if (SUCCEEDED(hr))
        {
            if ((timeoutValues.ReadIntervalTimeout == MAXULONG) &&
                    (timeoutValues.ReadTotalTimeoutMultiplier == MAXULONG) &&
                    (timeoutValues.ReadTotalTimeoutConstant == MAXULONG))
            {
                hr = E_INVALIDARG;
            }
        }

        if (SUCCEEDED(hr))
        {
            m_Device->SetTimeouts(timeoutValues);
        }

        break;
    }

    }

    //
    // clean up
    //
    if (inputMemory)
    {
        inputMemory->Release();
    }

    if (outputMemory)
    {
        outputMemory->Release();
    }

    //
    // complete the request
    //
    pWdfRequest->CompleteWithInformation(hr, reqCompletionInfo);

    return;
}

VOID STDMETHODCALLTYPE CMyQueue::OnWrite(__in IWDFIoQueue *pWdfQueue,__in IWDFIoRequest *pWdfRequest,__in SIZE_T BytesToWrite)
{
    IWDFMemory* pRequestMemory = NULL;
    IWDFIoRequest* pSavedRequest = NULL;
    SIZE_T savedRequestBufferSize = 0;
    HRESULT hr = S_OK;

    UNREFERENCED_PARAMETER(pWdfQueue);

    CMyQueueHandle *handle=GetHandleByPid(pWdfRequest->GetRequestorProcessId());

    if(handle==NULL)
    {
        pWdfRequest->Complete(E_FAIL);
        goto Exit;
    }


    //
    // Get memory object
    //

    pWdfRequest->GetInputMemory(&pRequestMemory);

    //
    // Process input
    //

    ProcessWriteBytes((PUCHAR)pRequestMemory->GetDataBuffer(NULL), BytesToWrite,pWdfRequest->GetRequestorProcessId());

    //
    // Release memory object and complete request
    //
    SAFE_RELEASE(pRequestMemory);
    pWdfRequest->CompleteWithInformation(hr, BytesToWrite);



    //process read queue
    while(BytesToWrite > 0)
    {
        //
        // Continue with the next request, if there is one pending
        //
        hr = m_FxReadQueue->RetrieveNextRequest(&pSavedRequest);
        if ((pSavedRequest == NULL) || (FAILED(hr)))
        {
            goto Exit;
        }

        pSavedRequest->GetReadParameters(&savedRequestBufferSize, NULL, NULL);

        OnRead(m_FxQueue,pSavedRequest,savedRequestBufferSize);

        //
        // RetrieveNextRequest from a manual queue increments the reference
        // counter by 1. We need to decrement it, otherwise the request will
        // not be released and there will be an object leak.
        //
        SAFE_RELEASE(pSavedRequest);
    }

Exit:
    return;
}

VOID STDMETHODCALLTYPE CMyQueue::OnRead(__in IWDFIoQueue *pWdfQueue,__in IWDFIoRequest *pWdfRequest,__in SIZE_T SizeInBytes)
{
    IWDFMemory* pRequestMemory = NULL;
    SIZE_T BytesCopied = 0;
    HRESULT     hr;

    UNREFERENCED_PARAMETER(pWdfQueue);

    CMyQueueHandle *handle=GetHandleByPid(pWdfRequest->GetRequestorProcessId());

    if(handle==NULL)
    {
        pWdfRequest->Complete(E_FAIL);
        goto Exit;
    }

    //
    // Get memory object
    //
    pWdfRequest->GetOutputMemory(&pRequestMemory);

    hr = handle->m_RingBuffer.Read((PBYTE)pRequestMemory->GetDataBuffer(NULL),SizeInBytes,&BytesCopied);
    handle->last_read=time(NULL);

    //
    // Release memory object.
    //
    SAFE_RELEASE(pRequestMemory);

    if (FAILED(hr))
    {
        //
        // Error reading buffer
        //
        pWdfRequest->Complete(hr);
        goto Exit;
    }


    pWdfRequest->CompleteWithInformation(hr, BytesCopied);


Exit:
    return;
}

VOID STDMETHODCALLTYPE CMyQueue::OnCleanupFile(__in IWDFFile *pWdfFileObject)
{
    WUDF_TEST_DRIVER_ASSERT(pWdfFileObject);
}

VOID CMyQueue::ProcessWriteBytes(__in_bcount(Length) PUCHAR Characters,__in SIZE_T Length,__in ULONG pid)
{

    UCHAR currentCharacter;
    while (Length != 0)
    {

        currentCharacter = *(Characters++);
        Length--;

        for(int i=0; i<sizeof(Handle)/sizeof(Handle[0]); i++)
        {
            if(Handle[i].pid !=pid)
            {
                Handle[i].m_RingBuffer.Write(&currentCharacter, sizeof(currentCharacter));
            }
        }


    }
    return;
}
