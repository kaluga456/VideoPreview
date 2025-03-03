#ifndef _VIDEO_FILE_H_
#define _VIDEO_FILE_H_

//TODO:
const UINT ERROR_UNDEFINED = 0xFFFFFFFF;

typedef std::shared_ptr<Gdiplus::Bitmap> PBitmap; //smart pointer for GDI bitmap
typedef std::shared_ptr<Gdiplus::Image> PImage; //smart pointer for GDI image

//video file
class CVideoFile
{
public:
    //ctor/dtor
    CVideoFile() {}
    ~CVideoFile() {Close();}

    //operations
    bool Open(const TCHAR* file_name);
    void Close();
    bool GetSnapshot(size_t offset, PBitmap& bitmap, app::byte_buffer& image_buffer);
    size_t GetVideoWidth() const {return static_cast<size_t>(VideoWidth);}
    size_t GetVideoHeight() const {return static_cast<size_t>(VideoHeight);}
    size_t GetDuration() const {return static_cast<size_t>(Duration / 1e7);}

private:
    //original video size
    long VideoWidth;  //pixels
    long VideoHeight; //pixels
    REFERENCE_TIME Duration;    //seconds

    //interfaces
    app::com_iface<IGraphBuilder> GraphBuilder;
    app::com_iface<IBasicVideo> BasicVideo;
    app::com_iface<IMediaSeeking> MediaSeeking;
};

#endif //_VIDEO_FILE_H_
