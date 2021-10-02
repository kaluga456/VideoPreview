#include "stdafx.h"
#pragma hdrstop
#include "OutputProfile.h"
#include "ScreenshotGenerator.h"

static LPCTSTR get_mime_type_string(CImageType image_type)
{
    switch(image_type)
    {
    case IMAGE_TYPE_JPEG:
        return L"image/jpeg";
    case IMAGE_TYPE_GIF:
        return L"image/gif";
    case IMAGE_TYPE_TIFF:
        return L"image/tiff";
    case IMAGE_TYPE_PNG:
        return L"image/png";
    }

    //use BMP format by default
    return L"image/bmp";
}

static LPCTSTR get_relative_file_name(LPCTSTR file_name)
{
    if(NULL == file_name)
        return NULL;

    LPCTSTR pos = ::_tcsrchr(file_name, _T('\\'));
    return (NULL == pos) ? file_name : pos + 1;
};

SNAPSHOTS_RESULT GenerateScreenshots(LPCTSTR video_file_name, const COutputProfile& options, ISnaphotsCallback* callback /*= NULL*/)
{
    if(NULL == video_file_name)
        return SNAPSHOTS_RESULT_FAIL;

    //try
    //{
    //    //init COM
    //    app::com com(COINIT_MULTITHREADED);

    //    //init gdi
    //    app::gdi gdi;

    //    //init video file
    //    CVideoFile video_file;
    //    APP_VERIFY(video_file.Open(video_file_name));

    //    //get video file attr
    //    const size_t duration = video_file.GetDuration();
    //    const size_t video_width = video_file.GetVideoWidth();
    //    const size_t video_height = video_file.GetVideoHeight();
    //    APP_VERIFY(video_height != 0);
    //    const float aspect_ratio = float(video_width) / float(video_height);
    //    
    //    //get snapshots count
    //    const size_t snapshots_count = options.XTilesCount * options.YTilesCount;

    //    //init output image
    //    const size_t tile_width = options.TileWidth;
    //    const size_t tile_height = (0 == options.TileHeight) ? 
    //        static_cast<size_t>(tile_width / aspect_ratio) : options.TileHeight;
    //    const size_t output_width = tile_width * options.XTilesCount;
    //    const size_t output_height = tile_height * options.YTilesCount;
    //    Gdiplus::Bitmap output_image(output_width, output_height, PixelFormat24bppRGB);
    //    Gdiplus::Graphics graphics(&output_image);

    //    //get image encoder CLSID
    //    app::gdi_encoders encoders;
    //    app::verify_gdi(encoders.initialize());
    //    CLSID encoder_clsid;
    //    APP_VERIFY(true == encoders.encoder_clsid(get_mime_type_string(options.OutputImageType), encoder_clsid));

    //    //make
    //    size_t interval = duration / (snapshots_count + 1);
    //    size_t current_position = interval;
    //    for(size_t snapshot_index = 0; snapshot_index < snapshots_count; ++snapshot_index, current_position += interval)
    //    {
    //        if(callback != NULL && true == callback->IsTerminate())
    //            return SNAPSHOTS_RESULT_TERMINATED;

    //        if(callback != NULL)
    //            callback->SetProgress(static_cast<size_t>(snapshot_index * 100.f / snapshots_count)); 

    //        //get snapshot
    //        app::byte_buffer image_buffer;
    //        PBitmap snapshot;
    //        APP_VERIFY(true == video_file.GetSnapshot(current_position, snapshot, image_buffer));

    //        //draw thumnail
    //        const INT thumb_x = (snapshot_index % options.XTilesCount) * tile_width;
    //        const INT thumb_y = (snapshot_index / options.YTilesCount) * tile_height;
    //        app::verify_gdi(graphics.DrawImage(snapshot.get(), thumb_x, thumb_y, tile_width, tile_height));
    //    }

    //    //make output file name
    //    app::string output_file_name;
    //    if(true == options.UseSourceFileDir)
    //        output_file_name = video_file_name;
    //    else
    //    {
    //        output_file_name.assign(options.OutputDirectory);
    //        output_file_name.append(_T("\\"));
    //        output_file_name.append(get_relative_file_name(video_file_name));
    //    }   
    //    output_file_name.append(_T("."));
    //    output_file_name.append(GetImageFileExt(options.OutputImageType));

    //    //save
    //    app::verify_gdi(output_image.Save(output_file_name.c_str(), &encoder_clsid));
    //}
    //catch(app::exception& exc)
    //{
    //    LPCTSTR error_string = exc.string();
    //    if(NULL == error_string)
    //    {
    //        TCHAR buf[2*KILOBYTE];
    //        exc.string(buf, 2*KILOBYTE);
    //        error_string = buf;
    //    }
    //    ::OutputDebugStr(error_string);
    //    return SNAPSHOTS_RESULT_FAIL;
    //}
    //catch(...)
    //{
    //    return SNAPSHOTS_RESULT_FAIL;
    //}

    return SNAPSHOTS_RESULT_SUCCESS;
}