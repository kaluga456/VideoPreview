#pragma once

//snapshot size types
enum 
{
    SNAPSHOT_SIZE_WIDTH,
    SNAPSHOT_SIZE_HEIGHT,
    SNAPSHOT_SIZE_WIDTH_HEIGHT,
    SNAPSHOT_SIZE_ORIGINAL
};

//snapshot count types
enum 
{
    SNAPSHOT_COUNT_FIXED,
    SNAPSHOT_COUNT_VARIABLE
};
#pragma once

//output type
enum
{
    OUTPUT_FILE_ALL_IN_ONE,
    OUTPUT_FILE_SEPARATED
};

enum
{
    SNAPSHOTS_RESULT_SUCCESS,
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

int GenerateScreenshots(LPCTSTR video_file_name, const COutputProfile& output_profile, LPCTSTR output_dir, CString& result_string, IScreenshotsCallback* callback = NULL);