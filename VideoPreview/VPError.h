#pragma once

CString VPGetErrorStr(DWORD error_code);
CString VPGetLastErrorStr();

class CVPExc
{
public:
    CVPExc() throw() {}
    virtual ~CVPExc() throw() {}

    //message
    virtual CString GetText() const throw() = 0;

    //error data
    virtual DWORD GetErrorCode() const throw() = 0;
    virtual CString GetErrorString() const throw() = 0;
};

class CVPExcStr : public CVPExc
{
public:
    explicit CVPExcStr(LPCTSTR msg = NULL) throw() : Text(msg ? msg : _T("")) {}

    virtual CString GetText() const throw() {return Text;}

    //error data
    virtual DWORD GetErrorCode() const throw() {return 0;}
    virtual CString GetErrorString() const throw() {return _T("");}

private:
    CString Text;
};

class CVPExcWinApi : public CVPExcStr
{
public:
    explicit CVPExcWinApi(DWORD error_code, LPCTSTR msg = NULL) throw() : CVPExcStr(msg), ErrorCode(error_code) {}
    virtual ~CVPExcWinApi() throw() {}

    virtual DWORD GetErrorCode() const throw() {return ErrorCode;}
    virtual CString GetErrorString() const throw() {return VPGetErrorStr(ErrorCode);}

protected:
    DWORD ErrorCode;
};

class CVPExcWinApiLast : public CVPExcWinApi
{
public:
    CVPExcWinApiLast() throw() : CVPExcWinApi(::GetLastError()) {}
};

void VPExcMsgBox(const CVPExc* exc, LPCTSTR msg = NULL);

//heplers
#define VP_VERIFY(expression) {if(NULL == (expression)) throw CVPExcStr(_T("VP_VERIFY(") _T(#expression) _T(") failed"));}
#define VP_VERIFY_WINAPI(error_code) {if(r != ERROR_SUCCESS) throw CVPExcWinApi(DWORD(error_code));}
#define VP_VERIFY_WINAPI_BOOL(bool_result) {if(FALSE == bool_result) throw CVPExcWinApi(::GetLastError());}
#define VP_THROW_WINAPI_LAST() {throw CVPExcWinApiLast();}