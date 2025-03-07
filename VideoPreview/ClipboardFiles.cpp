#include "stdafx.h"
#pragma hdrstop
#include "ClipboardFiles.h"

CString CDropFiles::GetFileName(UINT index)
{
    if(nullptr == Dropfiles) return CString();
    const UINT buffer_size = MAX_PATH + 1;
    TCHAR buffer[buffer_size];
    buffer[buffer_size - 1] = 0;
    if(0 == ::DragQueryFile(reinterpret_cast<HDROP>(Dropfiles), index, buffer, buffer_size))
        return CString();
    return CString(buffer);
}
LPCTSTR CDropFiles::GetFileName(UINT index, LPTSTR buffer, UINT buffer_size)
{
    if(nullptr == Dropfiles) return nullptr;
    ASSERT(buffer);
    ASSERT(buffer_size);
    if(0 == ::DragQueryFile(reinterpret_cast<HDROP>(Dropfiles), index, buffer, buffer_size))
        return nullptr;
    buffer[buffer_size - 1] = 0;
    return buffer;
}

CClipboardFiles::CClipboardFiles(HWND hwnd) : CDropFiles(), Clipboard(nullptr) //, Dropfiles(nullptr)
{
    ASSERT(hwnd);
    if(nullptr == hwnd) 
        return;

    if(FALSE == ::OpenClipboard(hwnd))
        return;

    do
    {
        Clipboard = ::GetClipboardData(CF_HDROP);
        if(nullptr == Clipboard)
            break;

        Dropfiles = reinterpret_cast<DROPFILES*>(::GlobalLock(Clipboard));
        if(nullptr == Dropfiles)
            break;

        //const UINT files_count = ::DragQueryFile(reinterpret_cast<HDROP>(Clipboard), 0xFFFFFFFF, nullptr, 0);

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
        Clipboard = nullptr;
    }
    ::CloseClipboard();
}

COleObjectFiles::COleObjectFiles(COleDataObject* ole_object) : CDropFiles(), ObjectData(ole_object)
{
    ASSERT(ole_object);
    if(nullptr == ole_object)
        return;

    do
    {
        ObjectData = ole_object->GetGlobalData(CF_HDROP);
        if(nullptr == ObjectData)
            break;

        Dropfiles = reinterpret_cast<DROPFILES*>(::GlobalLock(ObjectData));
        if(nullptr == Dropfiles)
            break;

        //const UINT files_count = ::DragQueryFile(reinterpret_cast<HDROP>(ObjectData), 0xFFFFFFFF, nullptr, 0);

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
        ObjectData = nullptr;
    }
}