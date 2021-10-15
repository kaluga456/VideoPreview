#include "stdafx.h"
#pragma hdrstop
#include "ClipboardFiles.h"

CString CDropFiles::GetFileName(UINT index)
{
    if(NULL == Dropfiles) return CString();
    const UINT buffer_size = MAX_PATH + 1;
    TCHAR buffer[buffer_size];
    buffer[buffer_size - 1] = 0;
    if(0 == ::DragQueryFile(reinterpret_cast<HDROP>(Dropfiles), index, buffer, buffer_size))
        return CString();
    return CString(buffer);
}
LPCTSTR CDropFiles::GetFileName(UINT index, LPTSTR buffer, UINT buffer_size)
{
    if(NULL == Dropfiles) return NULL;
    ASSERT(buffer);
    ASSERT(buffer_size);
    if(0 == ::DragQueryFile(reinterpret_cast<HDROP>(Dropfiles), index, buffer, buffer_size))
        return NULL;
    buffer[buffer_size - 1] = 0;
    return buffer;
}

CClipboardFiles::CClipboardFiles(HWND hwnd) : CDropFiles(), Clipboard(NULL) //, Dropfiles(NULL)
{
    ASSERT(hwnd);
    if(NULL == hwnd) 
        return;

    if(FALSE == ::OpenClipboard(hwnd))
        return;

    do
    {
        Clipboard = ::GetClipboardData(CF_HDROP);
        if(NULL == Clipboard)
            break;

        Dropfiles = reinterpret_cast<DROPFILES*>(::GlobalLock(Clipboard));
        if(NULL == Dropfiles)
            break;

        //const UINT files_count = ::DragQueryFile(reinterpret_cast<HDROP>(Clipboard), 0xFFFFFFFF, NULL, 0);

        //ok
        return;
    }
    while(false);

    Deinit();
}
CClipboardFiles::~CClipboardFiles()
{
    Deinit();
}
void CClipboardFiles::Deinit()
{
    if(Clipboard) 
    {
        ::GlobalUnlock(Clipboard);
        Clipboard = NULL;
    }
    ::CloseClipboard();
}

COleObjectFiles::COleObjectFiles(COleDataObject* ole_object) : CDropFiles(), ObjectData(ole_object)
{
    ASSERT(ole_object);
    if(NULL == ole_object)
        return;

    do
    {
        ObjectData = ole_object->GetGlobalData(CF_HDROP);
        if(NULL == ObjectData)
            break;

        Dropfiles = reinterpret_cast<DROPFILES*>(::GlobalLock(ObjectData));
        if(NULL == Dropfiles)
            break;

        //const UINT files_count = ::DragQueryFile(reinterpret_cast<HDROP>(ObjectData), 0xFFFFFFFF, NULL, 0);

        //ok
        return;
    }
    while(false);

    Deinit();
}
COleObjectFiles::~COleObjectFiles()
{
    Deinit();
}
void COleObjectFiles::Deinit()
{
    if(ObjectData) 
    {
        ::GlobalUnlock(ObjectData);
        ::GlobalFree(ObjectData);
        ObjectData = NULL;
    }
}