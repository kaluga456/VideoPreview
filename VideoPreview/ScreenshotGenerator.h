#pragma once

enum : UINT
{
    SCREENLIST_RESULT_SUCCESS = 0,
    SCREENLIST_RESULT_FAIL,
    SCREENLIST_RESULT_TERMINATED
};

//callback target interface
class IScreenshotsCallback
{
public:
    virtual bool IsTerminate() const = 0;
    virtual void SetProgress(size_t progress) = 0;
};

//scrrenlist generators
UINT GenerateScreenlistPreview(const COutputProfile& output_profile, CString& result_string);
UINT GenerateScreenlist(LPCTSTR video_file_name, LPCTSTR output_dir, const COutputProfile& output_profile, CString& result_string, IScreenshotsCallback* callback = nullptr);