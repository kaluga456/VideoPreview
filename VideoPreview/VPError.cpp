#include "stdafx.h"
#pragma hdrstop
#include "VPError.h"

CString VPGetErrorStr(DWORD error_code)
{
    const int buffer_size = 2048;
    TCHAR buffer[buffer_size];
    DWORD error_string_size = ::FormatMessageW( FORMAT_MESSAGE_FROM_SYSTEM |
                                                FORMAT_MESSAGE_IGNORE_INSERTS, 
                                                nullptr, 
                                                error_code, 
                                                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), //default language
                                                buffer, 
                                                buffer_size, 
        nullptr);

    if(error_string_size < 2) //treat as error
    {
        *buffer = 0;
        return CString(VP_UNKNOWN_ERROR_STRING);
    }

    //ok
    if (error_string_size >= 2) error_string_size -= 2; //exclude trailing "\r\n" symbols
    buffer[error_string_size] = 0; 
    return CString(buffer);
}

CString VPGetLastErrorStr()
{
    return VPGetErrorStr(::GetLastError());
}
CString CVPExcWinApi::GetFullText() const noexcept
{
    CString result(Text);

    if(ERROR_SUCCESS == ErrorCode)
        return result;

    CString error_text(VPGetErrorStr(ErrorCode));
    if(error_text.IsEmpty())
        return result;

    if(false == result.IsEmpty())
        result += _T("\n");
    result += error_text;
    return result;
}

CString CVPExcDirectShow::GetFullText() const noexcept
{
    const int buffer_size = MAX_ERROR_TEXT_LEN;
    TCHAR buffer[buffer_size];
    DWORD error_string_size = ::AMGetErrorText(ErrorCode, buffer, buffer_size);
    if(error_string_size >= 2) error_string_size -= 2; //exclude trailing "\r\n" symbols
    buffer[error_string_size] = 0;
    return CString(buffer);
}

void VPExcMsgBox(const CVPExc* exc, LPCTSTR msg /*= nullptr*/)
{
    CString text(msg ? msg : _T(""));
    CString error_text(exc ? exc->GetErrorString() : CString{});

    if (false == text.IsEmpty())
        text += _T("\n");
    if (false == error_text.IsEmpty())
        text += error_text;
    if (text.IsEmpty()) text = VP_UNKNOWN_ERROR_STRING;
    ::AfxMessageBox(text, MB_OK | MB_ICONWARNING);
}