#ifndef _APP_COM_H_
#define _APP_COM_H_

#include "app_exception.h"

namespace app
{

#define APP_VERIFY_COM(error_code) \
    {const DWORD r = (error_code); \
    if(r != S_OK) \
        throw app::exception_winapi_error(HRESULT_CODE(r), TEXT(__FILE__), __LINE__);}

//com initializer
class com
{
public:
    //ctor/dtor
    explicit com(DWORD options = COINIT_MULTITHREADED) {APP_VERIFY_COM(::CoInitializeEx(NULL, options));}
    ~com() throw() {::CoUninitialize();}
};

//COM interface
template<typename T> class com_iface : private boost::noncopyable
{
public:
    //ctor/dtor
    explicit com_iface(T* iface = NULL) throw() : Interface(iface) {}
    com_iface(REFCLSID clsid, REFIID iid)
    {
        APP_VERIFY_COM(::CoCreateInstance(clsid, NULL, CLSCTX_INPROC_SERVER, iid, 
            reinterpret_cast<void **>(&Interface)));
    }
    ~com_iface() throw() 
    {
        if(Interface != NULL)
            Interface->Release();
    }

    //init
    HRESULT create(REFCLSID clsid, REFIID iid) throw()
    {
        if(Interface != NULL)
        {
            Interface->Release();
            Interface = NULL;
        }
        return ::CoCreateInstance(clsid, NULL, CLSCTX_INPROC_SERVER, iid, reinterpret_cast<void **>(&Interface));
    }
    void reset(T* iface = NULL) throw()
    {
        if(Interface != NULL)
            Interface->Release();
        Interface = iface;
    }
    T** pointer() throw() //WARNING: use only when initializing iface in outer COM function call
    {
        if(Interface != NULL)
        {
            Interface->Release();
            Interface = NULL;
        }
        return &Interface;
    }
    void** void_pointer() throw() //WARNING: use only when initializing iface in outer COM function call
    {
        return reinterpret_cast<void **>(pointer());
    }
    

    //access
    operator T*() throw() {return Interface;}
    operator const T*() const throw() {return Interface;}
    T* operator->() throw() {return Interface;}
    const T* operator->() const throw() {return Interface;}
    T* get() throw() {return Interface;}
    const T* get() const throw() {return Interface;}
    bool valid() const throw() {return (Interface != NULL);}

private:
    T* Interface;
};

} //namespace app

#endif //_APP_COM_H_
