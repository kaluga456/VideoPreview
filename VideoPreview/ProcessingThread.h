#pragma once

//messages from processing thread
//WPARAM - message type
enum
{
    PTM_PROGRESS,   //LPARAM - progress (0..100)
    PTM_DONE,       //LPARAM - result text, including output file name (LPTSTR)
    PTM_STOP,       //LPARAM - NULL
    PTM_FAILED      //LPARAM - error description (LPTSTR)
};
const UINT WM_PROCESSING_THREAD = WM_APP + 1; 

//processing thread
class CProcessingThread : public IScreenshotsCallback, private boost::noncopyable
{
public:
    //ctor/dtor
    CProcessingThread() : TerminateSignal(false) {}
    ~CProcessingThread() {Stop();}

    //init
    DWORD Start(HWND message_target, const COutputProfile* output_profile, LPCTSTR source_file_name);
    void Stop();

private:
    friend class app::thread<CProcessingThread>;
    app::thread<CProcessingThread> Thread;

    //params
    HWND MessageTarget;
    CString SourceFileName;
    COutputProfile OutputProfile;

    volatile bool TerminateSignal;

    void NotifyMessageTarget(WPARAM message_type, LPARAM message_data = 0);
    void NotifyResult(WPARAM message_type, LPCTSTR error_description);
    DWORD Run(); //thread procedure
    void ProcessItem();

    //callbacks
    virtual bool IsTerminate() const {return TerminateSignal;}
    virtual void SetProgress(size_t progress);
};

extern CProcessingThread ProcessingThread;