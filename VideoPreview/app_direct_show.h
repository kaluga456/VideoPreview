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
    exception_direct_show(HRESULT error_code, const TCHAR* src_file_name, size_t line_number) throw() : 
      exception_debug(src_file_name, line_number), ErrorCode(error_code) {}

    //string data
    virtual const size_t string(TCHAR* buffer, size_t buffer_size) const throw() 
    {
        return ::AMGetErrorText(ErrorCode, buffer, buffer_size);
    }

private:
    HRESULT ErrorCode;
};

#define APP_VERIFY_DSHOW(error_code) \
    {const DWORD r = (error_code); \
    if(r != S_OK) \
        throw app::exception_direct_show(HRESULT_CODE(r), TEXT(__FILE__), __LINE__);}

} //namespace app

#endif //_APP_DIRECT_SHOW_H_
