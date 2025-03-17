#ifndef _VIDEO_FILE_H_
#define _VIDEO_FILE_H_

using PBitmap = std::shared_ptr<Gdiplus::Bitmap>; //smart pointer for GDI bitmap

LPCTSTR GetRelativeFileName(LPCTSTR file_name);
CString GetFileSizeString(const LARGE_INTEGER& file_size);
CString GetDurationString(int duration);

//video file
class CVideoFile
{
public:
    //ctor/dtor
    CVideoFile() {}
    ~CVideoFile() {Close();}

    //operations
    void Open(LPCTSTR file_name);
    void Close();
    bool GetFrameImage(size_t offset, PBitmap& bitmap, std::vector<BYTE>& image_buffer);

    //access
    long GetVideoWidth() const {return VideoWidth;}
    long GetVideoHeight() const {return VideoHeight;}
    int GetDuration() const {return static_cast<int>(Duration / 1e7);}
    void GetSize(LARGE_INTEGER& size) const { size = FileSize; };

private:
    long VideoWidth{};              //pixels
    long VideoHeight{};             //pixels
    REFERENCE_TIME Duration{};      //seconds

    //interfaces
    app::com_iface<IGraphBuilder> GraphBuilder;
    app::com_iface<IBasicVideo> BasicVideo;
    app::com_iface<IMediaSeeking> MediaSeeking;

    LARGE_INTEGER FileSize{};
    void GetFileSize(LPCTSTR file_name);
};

#endif //_VIDEO_FILE_H_
