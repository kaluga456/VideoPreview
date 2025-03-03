#include "stdafx.h"
#include "app.h"
#include "app_error.h"
#include "app_com.h"
#include "app_gdi.h"
#include "app_direct_show.h"
#include "app_buffer.h"
#pragma hdrstop
#include "VideoFile.h"

bool CVideoFile::Open(const TCHAR* file_name)
{
    Close();
    if(NULL == file_name)
        return false;

    try
    {
        //create graph builder
        APP_VERIFY_COM(GraphBuilder.create(CLSID_FilterGraph, IID_IGraphBuilder));

        //render file
        APP_VERIFY_DSHOW(GraphBuilder->RenderFile(file_name, NULL));

        //try to replace render filter with VMR9
        app::com_iface<IBaseFilter> render_filter;
        if(S_OK == GraphBuilder->FindFilterByName(L"Video Renderer", render_filter.pointer()))
        {
            //find decoder output pin
            app::com_iface<IPin> render_filter_pin;
            app::com_iface<IPin> decoder_filter_pin;
            app::com_iface<IEnumPins> enum_pins;
            APP_VERIFY_DSHOW(render_filter->EnumPins(enum_pins.pointer()));
            APP_VERIFY_DSHOW(enum_pins->Next(1, render_filter_pin.pointer(), NULL));
            APP_VERIFY_DSHOW(render_filter_pin->ConnectedTo(decoder_filter_pin.pointer()));

            //remove old render filter
            APP_VERIFY_DSHOW(GraphBuilder->RemoveFilter(render_filter.get()));

            //add VMR9 filter
            app::com_iface<IBaseFilter> vmr9_filter(CLSID_VideoMixingRenderer9, IID_IBaseFilter);
            APP_VERIFY_DSHOW(GraphBuilder->AddFilter(vmr9_filter, L"VMR9"));

            //connect VMR9 with decoder
            render_filter_pin.reset();
            enum_pins.reset();
            APP_VERIFY_DSHOW(vmr9_filter->EnumPins(enum_pins.pointer()));
            APP_VERIFY_DSHOW(enum_pins->Next(1, render_filter_pin.pointer(), NULL));
            APP_VERIFY_DSHOW(decoder_filter_pin->Connect(render_filter_pin.get(), NULL));
        } 

        //query interfaces
        app::com_iface<IVideoWindow> video_window;
        app::com_iface<IMediaControl> media_control;
        APP_VERIFY_COM(GraphBuilder->QueryInterface(IID_IBasicVideo, BasicVideo.void_pointer()));
        APP_VERIFY_COM(GraphBuilder->QueryInterface(IID_IMediaControl, media_control.void_pointer()));
        APP_VERIFY_COM(GraphBuilder->QueryInterface(IID_IMediaSeeking, MediaSeeking.void_pointer()));
        APP_VERIFY_COM(GraphBuilder->QueryInterface(IID_IVideoWindow, video_window.void_pointer()));

        //don`t show video window
        APP_VERIFY_DSHOW(video_window->put_AutoShow(OAFALSE));

        //set media time format
        APP_VERIFY_DSHOW(MediaSeeking->SetTimeFormat(&TIME_FORMAT_MEDIA_TIME));

        //go to paused state
        const HRESULT result = media_control->Pause();
        if(FAILED(result)) //partial success, treat as success here
            APP_VERIFY_DSHOW(result);

        //get duration and video size
        APP_VERIFY_DSHOW(MediaSeeking->GetDuration(&Duration));
        APP_VERIFY_DSHOW(BasicVideo->GetVideoSize(&VideoWidth, &VideoHeight));
    }
    catch(app::exception&) //operation failed
    {
        Close();
        //app::print_exception(exception);
        return false;
    }

    //operation succeeded
    return true;
}
void CVideoFile::Close()
{
    BasicVideo.reset();
    MediaSeeking.reset();
    GraphBuilder.reset();
}
bool CVideoFile::GetSnapshot(size_t offset, PBitmap& bitmap, app::byte_buffer& image_buffer)
{
    if(false == GraphBuilder.valid())
        return false;

    try
    {
        //seek
        REFERENCE_TIME position = static_cast<REFERENCE_TIME>(offset * 1e7);
        APP_VERIFY_DSHOW(MediaSeeking->SetPositions(&position, AM_SEEKING_AbsolutePositioning,
            NULL, AM_SEEKING_NoPositioning));
        
        //get required space for image
        long buffer_size = 0;
        APP_VERIFY_DSHOW(BasicVideo->GetCurrentImage(&buffer_size, NULL));

        //allocate buffer
        image_buffer.reset(buffer_size);
        if(NULL == image_buffer)
            return false;

        //get image
        APP_VERIFY_DSHOW(BasicVideo->GetCurrentImage(&buffer_size, reinterpret_cast<long*>(image_buffer.data())));
        bitmap.reset(new Gdiplus::Bitmap(reinterpret_cast<BITMAPINFO*>(image_buffer.data()), image_buffer.data() + sizeof(BITMAPINFO)));
    }
    catch(app::exception&) //operation failed
    {
        Close();
        //app::print_exception(exception);
        return false;
    }
    catch(std::exception&) //operation failed
    {
        return false;
    }

    //operation succeeded
    return true;
}
