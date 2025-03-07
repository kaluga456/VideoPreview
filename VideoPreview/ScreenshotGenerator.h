#pragma once

enum : UINT
{
    SNAPSHOTS_RESULT_SUCCESS = 0,
    SNAPSHOTS_RESULT_FAIL,
    SNAPSHOTS_RESULT_TERMINATED
};

LPCTSTR GetImageFileExt(int image_type);

//callback target interface
class IScreenshotsCallback
{
public:
    virtual bool IsTerminate() const = 0;
    virtual void SetProgress(size_t progress) = 0;
};

//int GenerateProfilePreview(const COutputProfile& output_profile, CString& result_string);
int GenerateScreenshots(LPCTSTR video_file_name, LPCTSTR output_dir, const COutputProfile& output_profile, CString& result_string, IScreenshotsCallback* callback = nullptr);