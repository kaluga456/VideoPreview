#ifndef _VIDEO_FILE_H_
#define _VIDEO_FILE_H_

using PBitmap = std::shared_ptr<Gdiplus::Bitmap>; //smart pointer for GDI bitmap

LPCTSTR GetRelativeFileName(LPCTSTR file_name);
CString GetFileSizeString(const __int64& file_size);
CString GetDurationString(int duration);

//video file
class CVideoFile
{
public:
    //ctor/dtor
    explicit CVideoFile(CString file_name = CString());

    //operations
    PBitmap GetFrameImage(size_t offset);

    //access
    bool IsPreview() const { return FileName.IsEmpty(); }
    LPCTSTR GetName() const { return IsPreview() ? SAMPLE_VIDEO_NAME : FileName.GetString(); }
    long GetVideoWidth() const { return VideoWidth; }
    long GetVideoHeight() const { return VideoHeight; }
    int GetDuration() const {return static_cast<int>(Duration / 1e7);}
    __int64 GetFileSize() const { return FileSize; };

private:
    //sample values for profile preview
    static constexpr LPCTSTR SAMPLE_VIDEO_NAME = _T("sample_frame.bmp"); //sample value
    static constexpr REFERENCE_TIME SAMPLE_FRAME_DURATION = 0;
    static constexpr int SAMPLE_VIDEO_SIZE = 230456;
    static constexpr int SAMPLE_FRAME_WIDTH = 320;
    static constexpr int SAMPLE_FRAME_HEIGHT = 200;

    //file attr
    __int64 FileSize{};
    long VideoWidth{};              //pixels
    long VideoHeight{};             //pixels
    REFERENCE_TIME Duration{};      //seconds
    CString FileName; //full file name or empty for profille preview

    //frames
    PBitmap SampleFrame; //for profile preview
    std::vector<BYTE> FrameBuffer; //for video file

    //interfaces
    app::com_iface<IGraphBuilder> GraphBuilder;
    app::com_iface<IBasicVideo> BasicVideo;
    app::com_iface<IMediaSeeking> MediaSeeking;
};

#endif //_VIDEO_FILE_H_
