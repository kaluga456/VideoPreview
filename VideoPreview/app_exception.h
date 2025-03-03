#ifndef _APP_EXCEPTION_H_
#define _APP_EXCEPTION_H_

#include "app.h"
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
    virtual const TCHAR* string() const noexcept {return nullptr;}
    virtual const size_t string(TCHAR* buffer, DWORD buffer_size) const noexcept {return 0;}

    //source file information
    virtual const TCHAR* source_info() const noexcept {return nullptr;}
};

//generic exception with source file info
class exception_debug : public exception
{
public:
    //ctor/dtor
    explicit exception_debug(const TCHAR* src_info = nullptr) noexcept
    {
        if(src_info != nullptr)
            SourceInfo.assign(src_info);
    }

    //source file information
    const TCHAR* source_info() const noexcept override {return SourceInfo.c_str();}

private:
    std::wstring SourceInfo;
};

//generic exception with source file info
class exception_string : public exception_debug
{
public:
    //ctor/dtor
    exception_string(const TCHAR* string_data, const TCHAR* src_info) noexcept : exception_debug(src_info)
    {
        if(string_data != nullptr)
            StringData.assign(string_data);
    }

    //string data
    const TCHAR* string() const noexcept override {return StringData.c_str();}
    const size_t string(TCHAR* buffer, DWORD buffer_size) const noexcept override {return 0;}

private:
    std::wstring StringData;
};

//generic exception with source file info
class exception_winapi_error : public exception_debug
{
public:
    //ctor/dtor
    exception_winapi_error(DWORD error_code, const TCHAR* src_info) noexcept :
      exception_debug(src_info), ErrorCode(error_code) {}

    //string data
    virtual const size_t string(TCHAR* buffer, DWORD buffer_size) const noexcept 
    {
        return app::winapi_error_string(ErrorCode, buffer, buffer_size);
    }
    DWORD code() const noexcept {return ErrorCode;}

private:
    DWORD ErrorCode;
};

//heplers
#define STRINGIZE(x) STRINGIZE2(x)
#define STRINGIZE2(x) #x
#define SRC_LINE_STRING STRINGIZE(__LINE__)
#define SRC_INFO_STRING TEXT("[" __FILE__ ":" SRC_LINE_STRING "]")

#define APP_VERIFY(expression) \
    {if(false == (expression)) \
        throw app::exception_string(_T("APP_VERIFY(") _T(#expression) _T(")"), SRC_INFO_STRING);}

#define APP_VERIFY_WINAPI(error_code) \
    {const DWORD r(error_code); \
    if(r != ERROR_SUCCESS) \
        throw app::exception_winapi_error(r, SRC_INFO_STRING);}

#define APP_VERIFY_WINAPI_BOOL(ret_val) \
    {if(FALSE == ret_val) \
        throw app::exception_winapi_error(::GetLastError(), SRC_INFO_STRING);}

#define APP_THROW_WINAPI_LAST() \
    {throw app::exception_winapi_error(::GetLastError(), SRC_INFO_STRING);}
} //namespace app

#endif //_APP_EXCEPTION_H_
