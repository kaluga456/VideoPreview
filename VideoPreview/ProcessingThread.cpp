#include "stdafx.h"
#pragma hdrstop
#include "app_thread.h"
#include "OutputProfile.h"
#include "ProcessingItem.h"
#include "ProcessingThread.h"

//DEBUG:
#ifdef _DEBUG
#define SHALLOW_PROCESSING
#endif //_DEBUG

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
void CProcessingThread::NotifyResult(WPARAM message_type, LPCTSTR error_description)
{
    LPCTSTR message_data = NULL;

    //TODO:
    if(error_description != NULL)
        message_data = ::_wcsdup(error_description);
    NotifyMessageTarget(message_type, reinterpret_cast<LPARAM>(message_data));
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
      
    //TODO: exception class
    try
    {
        ProcessItem();
    }
    catch(...)
    {
        //TODO:
    }
#endif //SHALLOW_PROCESSING

    return 0;
}

void CProcessingThread::ProcessItem()
{
    //TODO: processing procedure



}