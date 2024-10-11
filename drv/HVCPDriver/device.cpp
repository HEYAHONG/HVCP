#include "internal.h"
#include "device.tmh"

HRESULT CMyDevice::CreateInstance(__in IWDFDriver *FxDriver,__in IWDFDeviceInitialize * FxDeviceInit,__out PCMyDevice *Device)
{
    PCMyDevice device;
    HRESULT hr;

    //
    // Allocate a new instance of the device class.
    //

    device = new CMyDevice();

    if (NULL == device)
    {
        return E_OUTOFMEMORY;
    }

    //
    // Initialize the instance.
    //

    hr = device->Initialize(FxDriver, FxDeviceInit);

    if (SUCCEEDED(hr))
    {
        *Device = device;
    }
    else
    {
        device->Release();
    }

    return hr;
}

HRESULT CMyDevice::Initialize(__in IWDFDriver           * FxDriver,__in IWDFDeviceInitialize * FxDeviceInit)
{
    IWDFDevice *fxDevice;
    HRESULT hr;

    //
    // Configure things like the locking model before we go to create our
    // partner device.
    //
    FxDeviceInit->SetLockingConstraint(WdfDeviceLevel);

    //
    // Create a new FX device object and assign the new callback object to
    // handle any device level events that occur.
    //

    //
    // QueryIUnknown references the IUnknown interface that it returns
    // (which is the same as referencing the device).  We pass that to
    // CreateDevice, which takes its own reference if everything works.
    //

    {
        IUnknown *unknown = this->QueryIUnknown();

        hr = FxDriver->CreateDevice(FxDeviceInit, unknown, &fxDevice);

        unknown->Release();
    }

    //
    // If that succeeded then set our FxDevice member variable.
    //

    if (SUCCEEDED(hr))
    {
        m_FxDevice = fxDevice;

        //
        // Drop the reference we got from CreateDevice.  Since this object
        // is partnered with the framework object they have the same
        // lifespan - there is no need for an additional reference.
        //

        fxDevice->Release();
    }

    return hr;
}

HRESULT CMyDevice::Configure(VOID)
{
    IWDFPropertyStoreFactory *pPropertyStoreFactory = NULL;
    IWDFNamedPropertyStore2 * pHardwarePropertyStore = NULL;
    IWDFNamedPropertyStore2 * pLegacyHardwarePropertyStore = NULL;
    WDF_PROPERTY_STORE_ROOT RootSpecifier;
    PROPVARIANT comPortPV;
    WCHAR portName[] = L"PortName";
    size_t comPortSuffixCch = 0;
    WCHAR *comPortFullName = NULL;
    size_t comPortFullNameCch = 0;
    WCHAR *pdoName = NULL;
    PCMyQueue defaultQueue;

    HRESULT hr;

    PropVariantInit(&comPortPV);

    //
    // Create device interface
    //
    hr = m_FxDevice->CreateDeviceInterface((LPGUID) &GUID_DEVINTERFACE_MODEM,NULL);

    if (FAILED(hr))
    {
        goto Exit;
    }

    hr = m_FxDevice->AssignDeviceInterfaceState((LPGUID) &GUID_DEVINTERFACE_MODEM,
         NULL,
         TRUE);

    if (FAILED(hr))
    {
        goto Exit;
    }

    //
    // Create Symbolic Link
    //

    //
    // First we need to read the COM number from the registry
    //

    hr = m_FxDevice->QueryInterface(IID_PPV_ARGS(&pPropertyStoreFactory));

    if (FAILED(hr))
    {
        goto Exit;
    }

    RootSpecifier.LengthCb = sizeof(WDF_PROPERTY_STORE_ROOT);
    RootSpecifier.RootClass = WdfPropertyStoreRootClassHardwareKey;
    RootSpecifier.Qualifier.HardwareKey.ServiceName =
        WDF_PROPERTY_STORE_HARDWARE_KEY_ROOT;

    hr = pPropertyStoreFactory->RetrieveDevicePropertyStore(
             &RootSpecifier,
             WdfPropertyStoreNormal,
             KEY_QUERY_VALUE,
             NULL,
             &pHardwarePropertyStore,
             NULL
         );

    if (FAILED(hr))
    {
        goto Exit;
    }

    hr = pHardwarePropertyStore->GetNamedValue(portName, &comPortPV);

    if (FAILED(hr))
    {
        goto Exit;
    }

    //
    // Then we need to construct the COM port name
    //
    hr = StringCchLength(comPortPV.pwszVal,STRSAFE_MAX_CCH,&comPortSuffixCch);

    if (FAILED(hr))
    {
        goto Exit;
    }

    hr = SizeTAdd(ARRAYSIZE(SYMBOLIC_LINK_NAME_PREFIX),
                  comPortSuffixCch,
                  &comPortFullNameCch);

    if (FAILED(hr))
    {
        goto Exit;
    }

    comPortFullName = (WCHAR*) new WCHAR[comPortFullNameCch];

    if (NULL == comPortFullName)
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    hr = StringCchPrintf(comPortFullName,
                         comPortFullNameCch,
                         L"%ws%ws",
                         SYMBOLIC_LINK_NAME_PREFIX,
                         comPortPV.pwszVal);

    if (FAILED(hr))
    {
        goto Exit;
    }

    //
    // Finally we create the symbolic link
    //
    hr = m_FxDevice->CreateSymbolicLink(comPortFullName);

    if (FAILED(hr))
    {
        goto Exit;
    }

    //
    // Write the com name to the legacy hardware key
    //
    hr = GetPdoName(&pdoName);

    if (FAILED(hr) || (pdoName == NULL))
    {
        goto Exit;
    }

    //
    // Write the name to the legacy hardware key
    //
    RootSpecifier.LengthCb = sizeof(WDF_PROPERTY_STORE_ROOT);
    RootSpecifier.RootClass = WdfPropertyStoreRootClassLegacyHardwareKey;
    RootSpecifier.Qualifier.LegacyHardwareKey.LegacyMapName = L"SERIALCOMM";

    hr = pPropertyStoreFactory->RetrieveDevicePropertyStore(
             &RootSpecifier,
             WdfPropertyStoreCreateVolatile,
             KEY_SET_VALUE,
             NULL,
             &pLegacyHardwarePropertyStore,
             NULL
         );

    if (FAILED(hr))
    {
        goto Exit;
    }

    hr = pLegacyHardwarePropertyStore->SetNamedValue(pdoName,
         &comPortPV);

    if (FAILED(hr))
    {
        goto Exit;
    }

    m_CreatedLegacyHardwareKey = TRUE;
    m_LegacyHardwarePropertyStore = pLegacyHardwarePropertyStore;
    m_PdoName = pdoName;

    //
    // Create and configure the queues
    //
    hr = CMyQueue::CreateInstance(this, m_FxDevice, &defaultQueue);

    if (FAILED(hr))
    {
        goto Exit;
    }

    defaultQueue->Release();


Exit:

    if (m_CreatedLegacyHardwareKey == FALSE)
    {
        //
        // If the legacy hardware key has been written, then the cleanup
        // will happen, when the device is unloaded
        //
        SAFE_RELEASE(pLegacyHardwarePropertyStore);
        if (pdoName != NULL)
        {
            delete[] pdoName;
        }
    }

    SAFE_RELEASE(pHardwarePropertyStore);
    SAFE_RELEASE(pPropertyStoreFactory);

    if (comPortFullName != NULL)
    {
        delete[] comPortFullName;
    }

    return hr;
}

HRESULT CMyDevice::QueryInterface(__in REFIID InterfaceId,__out PVOID *Object)
{
    HRESULT hr;

    if(IsEqualIID(InterfaceId, __uuidof(IObjectCleanup)))
    {
        *Object = QueryIObjectCleanup();
        hr = S_OK;
    }
    else
    {
        hr = CUnknown::QueryInterface(InterfaceId, Object);
    }

    return hr;
}

VOID CMyDevice::OnCleanup(IWDFObject*  pWdfObject)
{
    UNREFERENCED_PARAMETER(pWdfObject);

    if ((m_CreatedLegacyHardwareKey == TRUE) &&
            (m_LegacyHardwarePropertyStore != NULL))
    {
        m_LegacyHardwarePropertyStore->DeleteNamedValue(m_PdoName);
        SAFE_RELEASE(m_LegacyHardwarePropertyStore);
        delete[] m_PdoName;
    }
}

HRESULT CMyDevice::GetPdoName(__out WCHAR **PdoName)
{
    HRESULT hr = S_OK;
    WCHAR *devInstId = NULL;
    ULONG devInstIdLen;
    HDEVINFO hDevInfoSet = INVALID_HANDLE_VALUE;
    SP_DEVINFO_DATA devInfoData;
    WCHAR *pdoName = NULL;
    DWORD pdoNameCb = 0;


    //
    // First let's get the device instance ID
    //
    devInstIdLen = 0;
    hr = m_FxDevice->RetrieveDeviceInstanceId(NULL, &devInstIdLen);
    if (SUCCEEDED(hr))
    {
        devInstId = (WCHAR*) new WCHAR[devInstIdLen];
        if (NULL == devInstId)
        {
            hr = E_OUTOFMEMORY;
        }
    }
    if (SUCCEEDED(hr))
    {
        hr = m_FxDevice->RetrieveDeviceInstanceId(devInstId, &devInstIdLen);
    }

    //
    // Now use the SetupDiXxx functions to get a handle to the device's
    // hardware key
    //
    if (SUCCEEDED(hr))
    {
        //
        // Create a new device information set
        //
        hDevInfoSet = SetupDiCreateDeviceInfoList(NULL, NULL);
        if (INVALID_HANDLE_VALUE == hDevInfoSet)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }

    if (SUCCEEDED(hr))
    {
        //
        // Add our device to this device information set
        //
        ZeroMemory(&devInfoData, sizeof(devInfoData));
        devInfoData.cbSize = sizeof(devInfoData);
        if (!SetupDiOpenDeviceInfo(hDevInfoSet,
                                   devInstId,
                                   NULL,
                                   0,
                                   &devInfoData))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }

    if (SUCCEEDED(hr))
    {
        //
        // Get the length of the PDO name
        //
        if (!SetupDiGetDeviceRegistryProperty(hDevInfoSet,
                                              &devInfoData,
                                              SPDRP_PHYSICAL_DEVICE_OBJECT_NAME,
                                              NULL,
                                              (PBYTE)pdoName,
                                              0,
                                              &pdoNameCb))
        {
            //
            // The only reason for this call is to get the length of the
            // buffer. The only non acceptable reason for failure is, if we
            // asked for invalid data.
            //
            if (GetLastError() == ERROR_INVALID_DATA)
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
            }
        }
    }

    //
    // Get the PDO name
    //
    if (SUCCEEDED(hr))
    {
        pdoName = (WCHAR *)new BYTE[pdoNameCb];

        if (pdoName == NULL)
        {
            hr = E_OUTOFMEMORY;
        }
    }

    if (SUCCEEDED(hr))
    {
        if (!SetupDiGetDeviceRegistryProperty(hDevInfoSet,
                                              &devInfoData,
                                              SPDRP_PHYSICAL_DEVICE_OBJECT_NAME,
                                              NULL,
                                              (PBYTE)pdoName,
                                              pdoNameCb,
                                              NULL))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }

    }

    if (SUCCEEDED(hr))
    {
        *PdoName = pdoName;
    }

    //
    // clean up to be done regardless of success or failure
    //
    if (NULL != devInstId)
    {
        delete[] devInstId;
    }

    if (INVALID_HANDLE_VALUE != hDevInfoSet)
    {
        SetupDiDestroyDeviceInfoList(hDevInfoSet);
    }

    //
    // clean up to be done in case of failure only
    //
    if (FAILED(hr))
    {
        if (NULL != pdoName)
        {
            delete[] pdoName;
            pdoName = NULL;
        }
    }

    return hr;
}
