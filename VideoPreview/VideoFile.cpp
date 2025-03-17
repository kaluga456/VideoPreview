#include "stdafx.h"
#include "app_com.h"
#include "app_gdi.h"
#pragma hdrstop
#include "VideoFile.h"
#include "VPError.h"

LPCTSTR GetRelativeFileName(LPCTSTR file_name)
{
    ASSERT(file_name);
    LPCTSTR pos = ::_tcsrchr(file_name, _T('\\'));
    return (nullptr == pos) ? file_name : pos + 1;
};
CString GetDurationString(int duration)
{
    LPCTSTR format_string = _T("%02u:%02u:%02u");

    const int seconds = duration % 60;
    const int minutes = (duration / 60) % 60;
    const int hours = duration / 3600;

    CString result;
    result.Format(format_string, hours, minutes, seconds);

    return result;
}
CString GetFileSizeString(const LARGE_INTEGER& file_size)
{
    CString size_string;
    uint32_t size_value = 0;
    const QWORD terabyte = QWORD(1024 * 1024) * QWORD(1024 * 1024);
    if (file_size.QuadPart >= terabyte)
    {
        size_value = static_cast<uint32_t>(file_size.QuadPart / terabyte);
        size_string.Format(_T("%u TB"), size_value);
    }
    else if (file_size.QuadPart >= 1024 * 1024 * 1024)
    {
        size_value = static_cast<uint32_t>(file_size.QuadPart / (1024 * 1024 * 1024));
        size_string.Format(_T("%u GB"), size_value);
    }
    else if (file_size.QuadPart >= 1024 * 1024)
    {
        size_value = static_cast<uint32_t>(file_size.QuadPart / (1024 * 1024));
        size_string.Format(_T("%u MB"), size_value);
    }
    else if (file_size.QuadPart >= 1024)
    {
        size_value = static_cast<uint32_t>(file_size.QuadPart / 1024);
        size_string.Format(_T("%u KB"), size_value);
    }
    else
    {
        size_value = static_cast<uint32_t>(file_size.QuadPart);
        size_string.Format(_T("%u B"), size_value);
    }

    CString bytes_string;
    if (file_size.HighPart)
        bytes_string.Format(_T(" (%I64u bytes)"), file_size.QuadPart);
    else
        bytes_string.Format(_T(" (%u bytes)"), file_size.LowPart);

    return size_string + bytes_string;
}

void CVideoFile::GetFileSize(LPCTSTR file_name)
{
    ASSERT(file_name);
    const HANDLE hFile = CreateFile(file_name, GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile == INVALID_HANDLE_VALUE)
        VP_THROW_WINAPI_LAST();

    if (FALSE == ::GetFileSizeEx(hFile, &FileSize))
    {
        ::CloseHandle(hFile);
        VP_THROW_WINAPI_LAST();
    }

    ::CloseHandle(hFile);
}

void CVideoFile::Open(LPCTSTR file_name)
{
    Close();
    VP_VERIFY(file_name);

    GetFileSize(file_name);

    //create graph builder
    VP_VERIFY_COM(GraphBuilder.create(CLSID_FilterGraph, IID_IGraphBuilder));

    //render file
    VP_VERIFY_DIRECT_SHOW(GraphBuilder->RenderFile(file_name, nullptr));

    //try to replace render filter with VMR9
    app::com_iface<IBaseFilter> render_filter;
    if(S_OK == GraphBuilder->FindFilterByName(L"Video Renderer", render_filter.pointer()))
    {
        //find decoder output pin
        app::com_iface<IPin> render_filter_pin;
        app::com_iface<IPin> decoder_filter_pin;
        app::com_iface<IEnumPins> enum_pins;
        VP_VERIFY_DIRECT_SHOW(render_filter->EnumPins(enum_pins.pointer()));
        VP_VERIFY_DIRECT_SHOW(enum_pins->Next(1, render_filter_pin.pointer(), nullptr));
        VP_VERIFY_DIRECT_SHOW(render_filter_pin->ConnectedTo(decoder_filter_pin.pointer()));

        //remove old render filter
        VP_VERIFY_DIRECT_SHOW(GraphBuilder->RemoveFilter(render_filter.get()));

        //add VMR9 filter
        app::com_iface<IBaseFilter> vmr9_filter(CLSID_VideoMixingRenderer9, IID_IBaseFilter);
        VP_VERIFY_DIRECT_SHOW(GraphBuilder->AddFilter(vmr9_filter, L"VMR9"));

        //connect VMR9 with decoder
        render_filter_pin.reset();
        enum_pins.reset();
        VP_VERIFY_DIRECT_SHOW(vmr9_filter->EnumPins(enum_pins.pointer()));
        VP_VERIFY_DIRECT_SHOW(enum_pins->Next(1, render_filter_pin.pointer(), nullptr));
        VP_VERIFY_DIRECT_SHOW(decoder_filter_pin->Connect(render_filter_pin.get(), nullptr));
    } 

    //query interfaces
    app::com_iface<IVideoWindow> video_window;
    app::com_iface<IMediaControl> media_control;
    VP_VERIFY_COM(GraphBuilder->QueryInterface(IID_IBasicVideo, BasicVideo.void_pointer()));
    VP_VERIFY_COM(GraphBuilder->QueryInterface(IID_IMediaControl, media_control.void_pointer()));
    VP_VERIFY_COM(GraphBuilder->QueryInterface(IID_IMediaSeeking, MediaSeeking.void_pointer()));
    VP_VERIFY_COM(GraphBuilder->QueryInterface(IID_IVideoWindow, video_window.void_pointer()));

    //don`t show video window
    VP_VERIFY_DIRECT_SHOW(video_window->put_AutoShow(OAFALSE));

    //set media time format
    VP_VERIFY_DIRECT_SHOW(MediaSeeking->SetTimeFormat(&TIME_FORMAT_MEDIA_TIME));

    //go to paused state
    const HRESULT result = media_control->Pause();
    if(FAILED(result)) //partial success, treat as success here
        VP_VERIFY_DIRECT_SHOW(result);

    //get duration and video size
    VP_VERIFY_DIRECT_SHOW(MediaSeeking->GetDuration(&Duration));
    VP_VERIFY_DIRECT_SHOW(BasicVideo->GetVideoSize(&VideoWidth, &VideoHeight));
}
void CVideoFile::Close()
{
    BasicVideo.reset();
    MediaSeeking.reset();
    GraphBuilder.reset();
}
bool CVideoFile::GetFrameImage(size_t offset, PBitmap& bitmap, std::vector<BYTE>& image_buffer)
{
    if (false == GraphBuilder.valid())
        return false;

    try
    {
        //seek
        REFERENCE_TIME position = static_cast<REFERENCE_TIME>(offset * 1e7);
        VP_VERIFY_DIRECT_SHOW(MediaSeeking->SetPositions(&position, AM_SEEKING_AbsolutePositioning,
            nullptr, AM_SEEKING_NoPositioning));

        //allocate required space for image
        long buffer_size = 0;
        VP_VERIFY_DIRECT_SHOW(BasicVideo->GetCurrentImage(&buffer_size, nullptr));
        image_buffer.reserve(buffer_size);

        //get image
        VP_VERIFY_DIRECT_SHOW(BasicVideo->GetCurrentImage(&buffer_size, reinterpret_cast<long*>(image_buffer.data())));
        bitmap.reset(new Gdiplus::Bitmap(reinterpret_cast<BITMAPINFO*>(image_buffer.data()), image_buffer.data() + sizeof(BITMAPINFO)));

        //operation succeeded
        return true;
    }
    catch (CVPExc&)
    {
        //TODO:
        //result_string = exc.GetFullText();
    }
    catch (...)
    {
        //result_string = VP_UNKNOWN_ERROR_STRING;
        
    }

    Close();
    return false;
}