#include "stdafx.h"
#pragma hdrstop
#include "app_thread.h"
#include "Options.h"
#include "OutputProfile.h"
#include "ScreenshotGenerator.h"
#include "ProcessingItem.h"
#include "ProcessingThread.h"

//DEBUG:
#ifdef _DEBUG
#define SHALLOW_PROCESSING
#endif //_DEBUG

extern COptions Options;
CProcessingThread ProcessingThread;

DWORD CProcessingThread::Start(HWND message_target, const COutputProfile* output_profile, LPCTSTR source_file_name)
{
    ASSERT(message_target);
    ASSERT(source_file_name);
    ASSERT(output_profile);

    //wait for previous thread instance if any
    Stop(); 

    //init
    MessageTarget = message_target;
    SourceFileName = source_file_name;
    OutputProfile = *output_profile; 

    //run
    return Thread.create(this);
}
void CProcessingThread::Stop()
{
    TerminateSignal = true; //signal thread to terminate
    Thread.join();          //terminate thread
}
void CProcessingThread::NotifyMessageTarget(WPARAM message_type, LPARAM message_data /*= 0*/)
{
    ::PostMessage(MessageTarget, WM_PROCESSING_THREAD, message_type, message_data);
}
void CProcessingThread::NotifyResult(WPARAM message_type, LPCTSTR result_string)
{
    LPCTSTR message_data = result_string ? ::_wcsdup(result_string) : NULL;
    NotifyMessageTarget(message_type, reinterpret_cast<LPARAM>(message_data));
}
void CProcessingThread::SetProgress(size_t progress)
{
    NotifyMessageTarget(PTM_PROGRESS, progress);
}
DWORD CProcessingThread::Run()
{
    TerminateSignal = false;

#ifdef SHALLOW_PROCESSING
    //TEST: shallow procedure
    ::srand(::GetTickCount());
    NotifyMessageTarget(PTM_PROGRESS, 0);
    for(int i = 1; i < 11; ++i)
    {
        //check terminate signal
        if(TerminateSignal)
        {
            NotifyResult(PTM_STOP, NULL);
            return 0;
        }

        //error simulation
        if(0 == ::rand() % 100)
        {
            NotifyResult(PTM_FAILED, _T("<test error happen>"));
            return 0;
        }

        //process
        NotifyMessageTarget(PTM_PROGRESS, i*10);
        ::Sleep(300);
    }
    NotifyResult(PTM_DONE, _T("<output file name>"));
#else //SHALLOW_PROCESSING
    //TODO: processing procedure
    CString result_string;
    const int result = GenerateScreenshots(SourceFileName, OutputProfile, Options.UseSourceFileLocation ? NULL : Options.OutputDirectory, result_string, this);
    if(SNAPSHOTS_RESULT_SUCCESS == result)
        NotifyResult(PTM_DONE, result_string);
    else if(SNAPSHOTS_RESULT_TERMINATED == result)
        NotifyResult(PTM_STOP, NULL);
    else
        NotifyResult(PTM_FAILED, result_string);
#endif //SHALLOW_PROCESSING

    return 0;
}
