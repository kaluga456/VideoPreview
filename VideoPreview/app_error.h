#ifndef _APP_ERROR_H_
#define _APP_ERROR_H_

const DWORD ERROR_FAILED = 0xFFFFFFFF; //undefined error

namespace app
{

//WinAPI error string
inline DWORD winapi_error_stringa(DWORD error_code, LPSTR buffer, DWORD buffer_size)
{
    DWORD error_string_size = ::FormatMessageA( FORMAT_MESSAGE_FROM_SYSTEM |
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
        return 0;
    }

    //ok
    error_string_size -= 2; //exclude trailing "\r\n" symbols
    buffer[error_string_size] = 0; 
    return error_string_size;
}
inline DWORD winapi_error_stringw(DWORD error_code, LPWSTR buffer, DWORD buffer_size)
{
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
        return 0;
    }

    //ok
    error_string_size -= 2; //exclude trailing "\r\n" symbols
    buffer[error_string_size] = 0; 
    return error_string_size;
}

#ifndef _UNICODE
#define winapi_error_string winapi_error_stringa
#else
#define winapi_error_string winapi_error_stringw
#endif //_UNICODE

} //namespace app

#endif //_APP_ERROR_H_
