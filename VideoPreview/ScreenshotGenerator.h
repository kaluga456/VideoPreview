#pragma once

enum
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

//screenlist generators
int GenerateScreenlist(const COutputProfile& output_profile, CString& result_string,
    CString video_file_name = CString(), CString output_dir = CString(), IScreenshotsCallback* callback = nullptr);