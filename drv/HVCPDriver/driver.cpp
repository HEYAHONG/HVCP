#include "internal.h"
#include "driver.tmh"

HRESULT CMyDriver::CreateInstance(__out PCMyDriver *Driver)
{
    PCMyDriver driver;
    HRESULT hr;

    //
    // Allocate the callback object.
    //

    driver = new CMyDriver();

    if (NULL == driver)
    {
        return E_OUTOFMEMORY;
    }

    //
    // Initialize the callback object.
    //

    hr = driver->Initialize();

    if (SUCCEEDED(hr))
    {
        //
        // Store a pointer to the new, initialized object in the output
        // parameter.
        //

        *Driver = driver;
    }
    else
    {

        //
        // Release the reference on the driver object to get it to delete
        // itself.
        //

        driver->Release();
    }

    return hr;
}

HRESULT CMyDriver::Initialize(VOID)
{
    return S_OK;
}

HRESULT CMyDriver::QueryInterface(__in REFIID InterfaceId,__out PVOID *Interface)
{
    if (IsEqualIID(InterfaceId, __uuidof(IDriverEntry)))
    {
        *Interface = QueryIDriverEntry();
        return S_OK;
    }
    else
    {
        return CUnknown::QueryInterface(InterfaceId, Interface);
    }
}

HRESULT CMyDriver::OnDeviceAdd(__in IWDFDriver *FxWdfDriver,__in IWDFDeviceInitialize *FxDeviceInit)
{
    HRESULT hr;

    PCMyDevice device = NULL;

    //
    // TODO: Do any per-device initialization (reading settings from the
    //       registry for example) that's necessary before creating your
    //       device callback object here.  Otherwise you can leave such
    //       initialization to the initialization of the device event
    //       handler.
    //

    //
    // Create a new instance of our device callback object
    //

    hr = CMyDevice::CreateInstance(FxWdfDriver, FxDeviceInit, &device);

    //
    // TODO: Change any per-device settings that the object exposes before
    //       calling Configure to let it complete its initialization.
    //

    //
    // If that succeeded then call the device's construct method.  This
    // allows the device to create any queues or other structures that it
    // needs now that the corresponding fx device object has been created.
    //

    if (SUCCEEDED(hr))
    {
        hr = device->Configure();
    }

    //
    // Release the reference on the device callback object now that it's been
    // associated with an fx device object.
    //

    if (NULL != device)
    {
        device->Release();
    }

    return hr;
}

