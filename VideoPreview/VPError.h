#pragma once

LPCTSTR const VP_UNKNOWN_ERROR_STRING = L"Unknown Error";

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

    CString GetText() const noexcept override {return Text;}

    //error data
    DWORD GetErrorCode() const noexcept override {return ERROR_SUCCESS;}
    CString GetErrorString() const noexcept override {return _T("");}
    CString GetFullText() const noexcept override {return Text;}

protected:
    CString Text; //app defined description
};

class CVPExcWinApi : public CVPExcStr
{
public:
    explicit CVPExcWinApi(DWORD error_code, LPCTSTR msg = NULL) noexcept : CVPExcStr(msg), ErrorCode(error_code) {}

    DWORD GetErrorCode() const noexcept override {return ErrorCode;}
    CString GetErrorString() const noexcept override {return VPGetErrorStr(ErrorCode);}
    CString GetFullText() const noexcept override;

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
#define VP_VERIFY(expression) {if(NULL == (expression)) throw CVPExcStr(_T("VP_VERIFY(") _T(#expression) _T(")"));}
#define VP_VERIFY_WINAPI(error_code) {if(r != ERROR_SUCCESS) throw CVPExcWinApi(DWORD(error_code));}
#define VP_VERIFY_WINAPI_BOOL(bool_result) {if(FALSE == bool_result) throw CVPExcWinApi(::GetLastError());}

#define VP_THROW(msg) {throw CVPExcStr(msg);}
#define VP_THROW_WINAPI_LAST() {throw CVPExcWinApiLast();}