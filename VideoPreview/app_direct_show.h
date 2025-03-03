#ifndef _APP_DIRECT_SHOW_H_
#define _APP_DIRECT_SHOW_H_

#include "app_exception.h"

namespace app
{

//direct show error
class exception_direct_show : public exception_debug
{
public:
    //ctor/dtor
    exception_direct_show(HRESULT error_code, const TCHAR* src_file_name) : 
        exception_debug(src_file_name), ErrorCode(error_code) {}

    //string data
    const size_t string(TCHAR* buffer, DWORD buffer_size) const noexcept override
    {
        return ::AMGetErrorText(ErrorCode, buffer, buffer_size);
    }

private:
    HRESULT ErrorCode;
};

#define APP_VERIFY_DSHOW(error_code) \
    {const DWORD r = (error_code); \
    if(r != S_OK) \
        throw app::exception_direct_show(HRESULT_CODE(r), SRC_INFO_STRING);}

} //namespace app

#endif //_APP_DIRECT_SHOW_H_
