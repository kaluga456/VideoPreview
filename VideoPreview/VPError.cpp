#include "stdafx.h"
#pragma hdrstop
#include "VPError.h"

LPCTSTR VP_UNKNOWN_ERROR_STRING = _T("Unknown Error");

CString VPGetErrorStr(DWORD error_code)
{
    const int buffer_size = 2048;
    TCHAR buffer[buffer_size];
    DWORD error_string_size = ::FormatMessageW( FORMAT_MESSAGE_FROM_SYSTEM |
                                                FORMAT_MESSAGE_IGNORE_INSERTS, 
                                                NULL, 
                                                error_code, 
                                                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), //default language
                                                buffer, 
                                                buffer_size, 
                                                NULL);

    if(error_string_size < 2) //treat as error
    {
        *buffer = 0;
        return CString(VP_UNKNOWN_ERROR_STRING);
    }

    //ok
    error_string_size -= 2; //exclude trailing "\r\n" symbols
    buffer[error_string_size] = 0; 
    return CString(buffer);
}
CString VPGetLastErrorStr()
{
    return VPGetErrorStr(::GetLastError());
}
void VPExcMsgBox(const CVPExc* exc, LPCTSTR msg /*= NULL*/)
{
    CString text(msg ? msg : _T(""));
    CString error_text(exc ? exc->GetErrorString() : _T(""));

    if(false == text.IsEmpty())
        text += _T("\n");
    if(false == error_text.IsEmpty())
        text += error_text; 
    if(text.IsEmpty()) text = VP_UNKNOWN_ERROR_STRING;
    ::AfxMessageBox(text, MB_OK | MB_ICONWARNING);
}