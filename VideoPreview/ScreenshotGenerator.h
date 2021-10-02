#pragma once

typedef int CImageType;
enum
{
    IMAGE_TYPE_BMP,
    IMAGE_TYPE_JPEG,
    IMAGE_TYPE_GIF,
    IMAGE_TYPE_TIFF,
    IMAGE_TYPE_PNG,

    IMAGE_TYPES_COUNT
};

inline LPCTSTR GetImageFileExt(int image_type) throw()
{
    switch(image_type)
    {
    case IMAGE_TYPE_BMP: return _T("bmp");
    case IMAGE_TYPE_JPEG: return _T("jpeg");
    case IMAGE_TYPE_GIF: return _T("gif");
    case IMAGE_TYPE_TIFF: return _T("tiff");
    case IMAGE_TYPE_PNG: return _T("png");
    default: return _T("");
    }
}

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

//output type
enum
{
    OUTPUT_FILE_ALL_IN_ONE,
    OUTPUT_FILE_SEPARATED
};

enum SNAPSHOTS_RESULT
{
    SNAPSHOTS_RESULT_SUCCESS,
    SNAPSHOTS_RESULT_FAIL,
    SNAPSHOTS_RESULT_TERMINATED
};

//callback target interface
class ISnaphotsCallback
{
public:
    virtual bool IsTerminate() const = 0;
    virtual void SetProgress(size_t progress) = 0;
};

SNAPSHOTS_RESULT GenerateScreenshots(LPCTSTR video_file_name, const COutputProfile& options, ISnaphotsCallback* callback = NULL);