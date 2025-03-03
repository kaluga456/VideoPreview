#ifndef _APP_COM_H_
#define _APP_COM_H_

#include "app_exception.h"

namespace app
{

#define APP_VERIFY_COM(error_code) \
    {const DWORD r = (error_code); \
    if(r != S_OK) \
        throw app::exception_winapi_error(HRESULT_CODE(r), SRC_INFO_STRING);}

//com initializer
class com
{
public:
    //ctor/dtor
    explicit com(DWORD options = COINIT_MULTITHREADED) {APP_VERIFY_COM(::CoInitializeEx(nullptr, options));}
    ~com() noexcept {::CoUninitialize();}
};

//COM interface
template<typename T> class com_iface
{
public:
    //ctor/dtor
    explicit com_iface(T* iface = nullptr) noexcept : Interface(iface) {}
    com_iface(REFCLSID clsid, REFIID iid)
    {
        APP_VERIFY_COM(::CoCreateInstance(clsid, nullptr, CLSCTX_INPROC_SERVER, iid, 
            reinterpret_cast<void **>(&Interface)));
    }
    ~com_iface() noexcept 
    {
        if(Interface != nullptr)
            Interface->Release();
    }

    //init
    HRESULT create(REFCLSID clsid, REFIID iid) noexcept
    {
        if(Interface != nullptr)
        {
            Interface->Release();
            Interface = nullptr;
        }
        return ::CoCreateInstance(clsid, nullptr, CLSCTX_INPROC_SERVER, iid, reinterpret_cast<void **>(&Interface));
    }
    void reset(T* iface = nullptr) noexcept
    {
        if(Interface != nullptr)
            Interface->Release();
        Interface = iface;
    }
    T** pointer() noexcept //WARNING: use only when initializing iface in outer COM function call
    {
        if(Interface != nullptr)
        {
            Interface->Release();
            Interface = nullptr;
        }
        return &Interface;
    }
    void** void_pointer() noexcept //WARNING: use only when initializing iface in outer COM function call
    {
        return reinterpret_cast<void **>(pointer());
    }
    

    //access
    operator T*() noexcept {return Interface;}
    operator const T*() const noexcept {return Interface;}
    T* operator->() noexcept {return Interface;}
    const T* operator->() const noexcept {return Interface;}
    T* get() noexcept {return Interface;}
    const T* get() const noexcept {return Interface;}
    bool valid() const noexcept {return (Interface != nullptr);}

private:
    T* Interface;
};

} //namespace app

#endif //_APP_COM_H_
