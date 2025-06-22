#include "stdafx.h"
#include "app_com.h"
#include "app_gdi.h"
#pragma hdrstop
#include "Resource.h"
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
CString GetFileSizeString(const __int64& file_size)
{
    //common sizes
    enum : unsigned __int64
    {
        KILOBYTE = 1024ll,
        MEGABYTE = KILOBYTE * KILOBYTE,
        GIGABYTE = MEGABYTE * KILOBYTE,
        TERABYTE = GIGABYTE * KILOBYTE
    };

    CString size_string;
    uint32_t size_value = 0;
    if (file_size >= TERABYTE)
    {
        size_value = static_cast<uint32_t>(file_size / TERABYTE);
        size_string.Format(_T("%u TB"), size_value);
    }
    else if (file_size >= GIGABYTE)
    {
        size_value = static_cast<uint32_t>(file_size / GIGABYTE);
        size_string.Format(_T("%u GB"), size_value);
    }
    else if (file_size >= MEGABYTE)
    {
        size_value = static_cast<uint32_t>(file_size / MEGABYTE);
        size_string.Format(_T("%u MB"), size_value);
    }
    else if (file_size >= KILOBYTE)
    {   
        size_value = static_cast<uint32_t>(file_size / KILOBYTE);
        size_string.Format(_T("%u KB"), size_value);
    }
    else
    {
        size_value = static_cast<uint32_t>(file_size);
        size_string.Format(_T("%u B"), size_value);
    }

    CString bytes_string;
    bytes_string.Format(_T(" (%I64u bytes)"), file_size);

    return size_string + bytes_string;
}

CVideoFile::CVideoFile(CString file_name /*= CString()*/) : FileName(file_name)
{
    //profile preview
    if(FileName.IsEmpty())
    {
        Duration = SAMPLE_FRAME_DURATION;
        VideoWidth = SAMPLE_FRAME_WIDTH;
        VideoHeight = SAMPLE_FRAME_HEIGHT;
        FileSize = SAMPLE_VIDEO_SIZE;
        SampleFrame = PBitmap(new Gdiplus::Bitmap(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_SAMPLE_FRAME)));
        return;
    }

    //try to open file
    const HANDLE hFile = CreateFile(FileName, GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile == INVALID_HANDLE_VALUE)
        VP_THROW_WINAPI_LAST();

    //get file size
    LARGE_INTEGER li{};
    if (FALSE == ::GetFileSizeEx(hFile, &li))
    {
        ::CloseHandle(hFile);
        VP_THROW_WINAPI_LAST();
    }
    ::CloseHandle(hFile);
    FileSize = li.QuadPart;

    //create graph builder
    VP_VERIFY_COM(GraphBuilder.create(CLSID_FilterGraph, IID_IGraphBuilder));

    //render file
    VP_VERIFY_DIRECT_SHOW(GraphBuilder->RenderFile(file_name, nullptr));

    //try to replace render filter with VMR9
    app::com_iface<IBaseFilter> render_filter;
    if (S_OK == GraphBuilder->FindFilterByName(L"Video Renderer", render_filter.pointer()))
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
    if (FAILED(result)) //partial success, treat as success here
        VP_VERIFY_DIRECT_SHOW(result);

    //get duration and video size
    VP_VERIFY_DIRECT_SHOW(MediaSeeking->GetDuration(&Duration));
    VP_VERIFY_DIRECT_SHOW(BasicVideo->GetVideoSize(&VideoWidth, &VideoHeight));
}
PBitmap CVideoFile::GetFrameImage(size_t offset)
{
    if (IsPreview())
        return SampleFrame;

    ASSERT(GraphBuilder.valid());

    //seek
    REFERENCE_TIME position = static_cast<REFERENCE_TIME>(offset * 1e7);
    VP_VERIFY_DIRECT_SHOW(MediaSeeking->SetPositions(&position, AM_SEEKING_AbsolutePositioning,
        nullptr, AM_SEEKING_NoPositioning));

    //allocate required space for image
    long buffer_size = 0;
    VP_VERIFY_DIRECT_SHOW(BasicVideo->GetCurrentImage(&buffer_size, nullptr));
    FrameBuffer.reserve(buffer_size);

    //get image
    VP_VERIFY_DIRECT_SHOW(BasicVideo->GetCurrentImage(&buffer_size, reinterpret_cast<long*>(FrameBuffer.data())));
    return PBitmap(new Gdiplus::Bitmap(reinterpret_cast<BITMAPINFO*>(FrameBuffer.data()), FrameBuffer.data() + sizeof(BITMAPINFO)));
}
