#ifndef _APP_EXCEPTION_H_
#define _APP_EXCEPTION_H_

#include "app.h"
#include "app_string.h"
#include "app_error.h"

namespace app {

//generic exception
class exception
{
public:
    //ctor/dtor
    exception() noexcept {}
    virtual ~exception() noexcept {}

    //string data
    virtual const TCHAR* string() const noexcept {return NULL;}
    virtual const size_t string(TCHAR* buffer, size_t buffer_size) const noexcept {return 0;}

    //source file information
    virtual const TCHAR* source_file() const noexcept {return NULL;}
    virtual const size_t source_file_line() const noexcept {return 0;}
};

//generic exception with source file info
class exception_debug : public exception
{
public:
    //ctor/dtor
    exception_debug(const TCHAR* src_file_name, size_t line_number) noexcept : SourceFileLine(line_number)
    {
        if(src_file_name != NULL)
            SourceFileName.assign(src_file_name);
    }

    //source file information
    virtual const TCHAR* source_file() const noexcept {return SourceFileName.c_str();}
    virtual const size_t source_file_line() const noexcept {return SourceFileLine;}

private:
    app::string SourceFileName;
    size_t SourceFileLine;
};

//generic exception with source file info
class exception_string : public exception_debug
{
public:
    //ctor/dtor
    exception_string(const TCHAR* string_data, const TCHAR* src_file_name, size_t line_number) noexcept : 
      exception_debug(src_file_name, line_number)
    {
        if(string_data != NULL)
            StringData.assign(string_data);
    }

    //string data
    virtual const TCHAR* string() const noexcept {return StringData.c_str();}
    virtual const size_t string(TCHAR* buffer, size_t buffer_size) const noexcept {return 0;}

private:
    app::string StringData;
};

//generic exception with source file info
class exception_winapi_error : public exception_debug
{
public:
    //ctor/dtor
    exception_winapi_error(DWORD error_code, const TCHAR* src_file_name, size_t line_number) noexcept : 
      exception_debug(src_file_name, line_number), ErrorCode(error_code) {}

    //string data
    virtual const size_t string(TCHAR* buffer, size_t buffer_size) const noexcept 
    {
        return app::winapi_error_string(ErrorCode, buffer, buffer_size);
    }
    DWORD code() const noexcept {return ErrorCode;}

private:
    DWORD ErrorCode;
};

//heplers
#define APP_VERIFY(expression) \
    {if(false == (expression)) \
        throw app::exception_string(_T("APP_VERIFY(") _T(#expression) _T(") failed"), TEXT(__FILE__), __LINE__);}

#define APP_VERIFY_WINAPI(error_code) \
    {const DWORD r(error_code); \
    if(r != ERROR_SUCCESS) \
        throw app::exception_winapi_error(r, TEXT(__FILE__), __LINE__);}

#define APP_VERIFY_WINAPI_BOOL(ret_val) \
    {if(FALSE == ret_val) \
        throw app::exception_winapi_error(::GetLastError(), TEXT(__FILE__), __LINE__);}

#define APP_THROW_WINAPI_LAST() \
    {throw app::exception_winapi_error(::GetLastError(), TEXT(__FILE__), __LINE__);}
} //namespace app

#endif //_APP_EXCEPTION_H_
