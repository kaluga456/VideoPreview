#ifndef _VIDEO_FILE_H_
#define _VIDEO_FILE_H_

//TODO:
const UINT ERROR_UNDEFINED = 0xFFFFFFFF;

typedef boost::shared_ptr<Gdiplus::Bitmap> PBitmap; //smart pointer for GDI bitmap
typedef boost::shared_ptr<Gdiplus::Image> PImage; //smart pointer for GDI image

//video file
class CVideoFile : private boost::noncopyable
{
public:
    //ctor/dtor
    CVideoFile() throw() {}
    ~CVideoFile() throw() {Close();}

    //operations
    bool Open(const TCHAR* file_name) throw();
    void Close() throw();
    bool GetSnapshot(size_t offset, PBitmap& bitmap, app::byte_buffer& image_buffer);
    size_t GetVideoWidth() const throw() {return static_cast<size_t>(VideoWidth);}
    size_t GetVideoHeight() const throw() {return static_cast<size_t>(VideoHeight);}
    size_t GetDuration() const throw() {return static_cast<size_t>(Duration / 1e7);}

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
