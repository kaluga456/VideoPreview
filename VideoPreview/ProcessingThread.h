#pragma once

//thread - generic thread
//NOTE: type 'T' must provide Run() method as thread procedure
namespace app
{
    template<typename T> class thread
    {
    public:
        //ctor/dtor
        thread() : Handle(nullptr) {}
        ~thread() { join(); }

        //init
        DWORD create(T* thread_procedure)
        {
            join(); //wait for previous thread instance if any

            Handle = ::CreateThread(nullptr, 0, ThreadProcedure, thread_procedure, 0, nullptr);
            return (nullptr == Handle) ? ::GetLastError() : ERROR_SUCCESS;
        }
        void join()
        {
            if (nullptr == Handle)
                return;

            //wait for thread termination
            ::WaitForSingleObject(Handle, INFINITE);
            ::CloseHandle(Handle);
            Handle = nullptr;
        }

        //attr
        bool is_valid() const { return (Handle != nullptr); }

    protected:
        HANDLE Handle;

        //thread procedure
        static DWORD WINAPI ThreadProcedure(void* param)
        {
            T* thread_procedure = static_cast<T*>(param);
            return thread_procedure->Run();
        }
    };
}

//messages from processing thread
//WPARAM - message type
enum
{
    PTM_PROGRESS,   //LPARAM - progress (0..100)
    PTM_DONE,       //LPARAM - result text, including output file name (LPTSTR)
    PTM_STOP,       //LPARAM - 0
    PTM_FAILED,     //LPARAM - error description (LPTSTR)
    PTM_CRIT_FAIL   //TODO: LPARAM - error description (LPTSTR)
};
const UINT WM_PROCESSING_THREAD = WM_APP + 1; 

//DEBUG:
#ifdef _DEBUG
//#define SHALLOW_PROCESSING
#endif //_DEBUG

//processing thread
class CProcessingThread : public IScreenshotsCallback
{
public:
    //ctor/dtor
    CProcessingThread() : TerminateSignal(false), MessageTarget(nullptr) {}
    ~CProcessingThread() {Stop();}

    //init
    DWORD Start(HWND message_target, const COutputProfile* output_profile, LPCTSTR source_file_name, LPCTSTR output_dir);
    void Stop();

private:
    friend class app::thread<CProcessingThread>;
    app::thread<CProcessingThread> Thread;

    //params
    HWND MessageTarget;
    CString SourceFileName;
    CString OutputDir;
    COutputProfile OutputProfile;

    volatile bool TerminateSignal;

    void NotifyMessageTarget(WPARAM message_type, LPARAM message_data = 0);
    void NotifyResult(WPARAM message_type, LPCTSTR error_description);
    DWORD Run(); //thread procedure
    void ProcessItem();

#ifdef SHALLOW_PROCESSING
    void ShallowProcedure();
#endif //SHALLOW_PROCESSING

    //callbacks
    virtual bool IsTerminate() const {return TerminateSignal;}
    virtual void SetProgress(size_t progress);
};

extern CProcessingThread ProcessingThread;