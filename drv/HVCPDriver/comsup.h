#pragma once


typedef class CUnknown *PCUnknown;
typedef class CClassFactory *PCClassFactory;


class CUnknown : public IUnknown
{

private:

    LONG m_ReferenceCount;

protected:


    CUnknown(VOID);

    virtual ~CUnknown(VOID)
    {
    }

public:

    IUnknown * QueryIUnknown(VOID);

public:

    virtual  ULONG  STDMETHODCALLTYPE  AddRef(VOID);

    virtual  ULONG  STDMETHODCALLTYPE  Release(VOID);

    virtual  HRESULT STDMETHODCALLTYPE  QueryInterface(__in REFIID InterfaceId,__out PVOID *Object);
};


class CClassFactory : public CUnknown, public IClassFactory
{

private:


    static LONG s_LockCount;

public:

    IClassFactory * QueryIClassFactory(VOID);


public:

    virtual ULONG STDMETHODCALLTYPE AddRef(VOID)
    {
        return __super::AddRef();
    }

    __drv_arg(this, __drv_freesMem(object))
    virtual ULONG STDMETHODCALLTYPE Release(VOID)
    {
        return __super::Release();
    }

    virtual HRESULT STDMETHODCALLTYPE  QueryInterface(__in REFIID InterfaceId,__out PVOID *Object);

    virtual HRESULT STDMETHODCALLTYPE  CreateInstance(__in_opt IUnknown *OuterObject,__in REFIID InterfaceId,__out PVOID *Object);

    virtual HRESULT STDMETHODCALLTYPE  LockServer(__in BOOL Lock);
};

