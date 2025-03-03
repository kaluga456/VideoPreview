#pragma once

LPCTSTR const VP_UNKNOWN_ERROR_STRING = _T("Unknown Error");

CString VPGetErrorStr(DWORD error_code);
CString VPGetLastErrorStr();

class CVPExc
{
public:
    CVPExc() noexcept {}
    virtual ~CVPExc() noexcept {}

    //message
    virtual CString GetText() const noexcept = 0;

    //error data
    virtual DWORD GetErrorCode() const noexcept = 0;
    virtual CString GetErrorString() const noexcept = 0;
    virtual CString GetFullText() const noexcept = 0;
};

class CVPExcStr : public CVPExc
{
public:
    explicit CVPExcStr(LPCTSTR msg = NULL) noexcept : Text(msg ? msg : _T("")) {}

    virtual CString GetText() const noexcept {return Text;}

    //error data
    virtual DWORD GetErrorCode() const noexcept {return ERROR_SUCCESS;}
    virtual CString GetErrorString() const noexcept {return _T("");}
    virtual CString GetFullText() const noexcept {return Text;}

protected:
    CString Text;
};

class CVPExcWinApi : public CVPExcStr
{
public:
    explicit CVPExcWinApi(DWORD error_code, LPCTSTR msg = NULL) noexcept : CVPExcStr(msg), ErrorCode(error_code) {}
    virtual ~CVPExcWinApi() noexcept {}

    virtual DWORD GetErrorCode() const noexcept {return ErrorCode;}
    virtual CString GetErrorString() const noexcept {return VPGetErrorStr(ErrorCode);}
    virtual CString GetFullText() const noexcept;

protected:
    DWORD ErrorCode;
};

class CVPExcWinApiLast : public CVPExcWinApi
{
public:
    CVPExcWinApiLast() noexcept : CVPExcWinApi(::GetLastError()) {}
};

void VPExcMsgBox(const CVPExc* exc, LPCTSTR msg = NULL);

//heplers
#define VP_VERIFY(expression) {if(NULL == (expression)) throw CVPExcStr(_T("VP_VERIFY(") _T(#expression) _T(") failed"));}
#define VP_VERIFY_WINAPI(error_code) {if(r != ERROR_SUCCESS) throw CVPExcWinApi(DWORD(error_code));}
#define VP_VERIFY_WINAPI_BOOL(bool_result) {if(FALSE == bool_result) throw CVPExcWinApi(::GetLastError());}

#define VP_THROW(msg) {throw CVPExcStr(msg);}
#define VP_THROW_WINAPI_LAST() {throw CVPExcWinApiLast();}