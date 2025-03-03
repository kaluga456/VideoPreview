#ifndef _APP_THREAD_H_
#define _APP_THREAD_H_

namespace app
{

//critical_section - critical section object for recursive locking strategy
class critical_section
{
    friend class critical_section_lock;
public:
    //init
    critical_section() {::InitializeCriticalSection(&cs);} //NOTE: can raise STATUS_NO_MEMORY exception
    ~critical_section() {::DeleteCriticalSection(&cs);}

private:
    CRITICAL_SECTION cs;

    //operation
    void lock() {::EnterCriticalSection(&cs);} //NOTE: can raise SEH exceptions
    void unlock() {::LeaveCriticalSection(&cs);}
};

//critical_section_lock - locks critical section during critical_section_lock object lifetime
class critical_section_lock
{
public:
    critical_section_lock(critical_section& critical_section) : cs(critical_section) {cs.lock();}
    ~critical_section_lock() {cs.unlock();}

private:
    critical_section& cs;
};

//event - event object
//class event
//{
//public:
//    //ctor/dtor
//    explicit event(bool initial_state = false, bool manual_reset = false, LPCTSTR name = nullptr)
//    {
//        Handle = ::CreateEvent(nullptr, manual_reset ? TRUE : FALSE, initial_state ? TRUE : FALSE, name);
//        if(nullptr == Handle)
//            APP_VERIFY_WINAPI(::GetLastError());
//    }
//    ~event() {::CloseHandle(Handle);}
//
//    HANDLE handle() const {return Handle;}
//
//    //operation
//    void set() const {APP_VERIFY(::SetEvent(Handle));}        //set event in signaled state
//    void reset() const {APP_VERIFY(::ResetEvent(Handle));}    //set event in nonsignaled state
//    bool signaled() const {return WAIT_OBJECT_0 == ::WaitForSingleObject(Handle, 0);}
//    DWORD wait(DWORD timeout = INFINITE) const {return ::WaitForSingleObject(Handle, timeout);}
//
//protected:
//    HANDLE Handle;
//};

//thread - generic thread
//NOTE: type 'T' must provide Run() method as thread procedure
template<typename T> class thread
{
public:
    //ctor/dtor
    thread() : Handle(nullptr) {}
    ~thread() {join();}

    //init
    DWORD create(T* thread_procedure)
    {
        join(); //wait for previous thread instance if any

        Handle = ::CreateThread(nullptr, 0, ThreadProcedure, thread_procedure, 0, nullptr);
        return (nullptr == Handle) ? ::GetLastError() : ERROR_SUCCESS;
    }
    void join()
    {
        if(nullptr == Handle)
            return;

        //wait for thread termination
        ::WaitForSingleObject(Handle, INFINITE);
        ::CloseHandle(Handle);
        Handle = nullptr;
    }

    //attr
    bool is_valid() const {return (Handle != nullptr);}

protected:
    HANDLE Handle;

    //thread procedure
    static DWORD WINAPI ThreadProcedure(void* param)
    {
        T* thread_procedure = static_cast<T*>(param);
        return thread_procedure->Run();
    }
}; 

} //namespace app

#endif //_APP_THREAD_H_