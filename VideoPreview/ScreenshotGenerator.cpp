#include "stdafx.h"
#include "app.h"
#include "app_buffer.h"
#include "app_string.h"
#include "app_error.h"
#include "app_exception.h"
#include "app_com.h"
#include "app_gdi.h"
#include "app_direct_show.h"
#pragma hdrstop
#include "OutputProfile.h"
#include "VideoFile.h"
#include "ScreenshotGenerator.h"

LPCTSTR GetImageFileExt(int image_type)
{
    switch(image_type)
    {
    case OUTPUT_FILE_FORMAT_BMP: return _T("bmp");
    case OUTPUT_FILE_FORMAT_JPG: return _T("jpg");
    case OUTPUT_FILE_FORMAT_PNG: return _T("png");
    }

    ASSERT(FALSE);
    return NULL;
}

static LPCTSTR get_mime_type_string(int image_type)
{
    switch(image_type)
    {
    case OUTPUT_FILE_FORMAT_BMP:
        return _T("image/bmp");
    case OUTPUT_FILE_FORMAT_PNG:
        return _T("image/png");
    }

    //use JPG format by default (OUTPUT_FILE_FORMAT_JPG)
    return _T("image/jpeg");
}

static LPCTSTR GetRelativeFileName(LPCTSTR file_name)
{
    if(NULL == file_name)
        return NULL;

    LPCTSTR pos = ::_tcsrchr(file_name, _T('\\'));
    return (NULL == pos) ? file_name : pos + 1;
};

static CString GenerateHeaderText(LPCTSTR video_file_name, const COutputProfile& output_profile, const CVideoFile& video_file)
{
    //TODO:
    ASSERT(video_file_name);
    CString result; 
    LPCTSTR file_name = GetRelativeFileName(video_file_name);
    const size_t duration = video_file.GetDuration();
    const size_t video_width = video_file.GetVideoWidth();
    const size_t video_height = video_file.GetVideoHeight();

    result.Format(_T("%s\r\nResolution: %ux&u | Duration: %u"), file_name, video_width, video_height, duration);
    return result;
}

static CString GenerateOutputFileName(LPCTSTR video_file_name, const COutputProfile& output_profile, LPCTSTR output_dir) 
{
    ASSERT(video_file_name);

    //TODO: force create dirs
    //if(output_dir)
    //{        
    //}

    //TODO: generate from COutputProfile::OutputFileFormat
    CString result(video_file_name);
    result += _T(".");
    result += GetImageFileExt(output_profile.OutputFileFormat);
    //result.Format(_T("%s.%s")), video_file_name, GetImageFileExt(output_profile.OutputFileFormat);
    return result; 
}

int GenerateScreenshots(LPCTSTR video_file_name, const COutputProfile& output_profile, LPCTSTR output_dir, CString& result_string, IScreenshotsCallback* callback /*= NULL*/)
{
    if(NULL == video_file_name)
        return SNAPSHOTS_RESULT_FAIL;

    try
    {
        //TODO: check output dir, check output file
        //init COM
        app::com com(COINIT_MULTITHREADED);

        //init gdi
        app::gdi gdi;

        //init video file
        CVideoFile video_file;
        APP_VERIFY(video_file.Open(video_file_name));

        //get video file attr
        const size_t duration = video_file.GetDuration();
        const size_t video_width = video_file.GetVideoWidth();
        const size_t video_height = video_file.GetVideoHeight();
        APP_VERIFY(video_height != 0);
        const float aspect_ratio = float(video_width) / float(video_height);
        
        //get snapshots count
        const size_t snapshots_count = output_profile.FrameRows * output_profile.FrameColumns;

        //TODO: init output image
        const size_t tile_width = output_profile.OutputImageSize;
        const size_t tile_height = static_cast<size_t>(tile_width / aspect_ratio); //(0 == output_profile.TileHeight) ? static_cast<size_t>(tile_width / aspect_ratio) : output_profile.TileHeight;
        const size_t output_width = tile_width * output_profile.FrameRows;
        const size_t output_height = tile_height * output_profile.FrameColumns;
        Gdiplus::Bitmap output_image(output_width, output_height, PixelFormat24bppRGB);
        Gdiplus::Graphics graphics(&output_image);

        //get image encoder CLSID
        app::gdi_encoders encoders;
        app::verify_gdi(encoders.initialize());
        CLSID encoder_clsid;
        APP_VERIFY(true == encoders.encoder_clsid(get_mime_type_string(output_profile.OutputFileFormat), encoder_clsid));

        //make
        size_t interval = duration / (snapshots_count + 1);
        size_t current_position = interval;
        for(size_t snapshot_index = 0; snapshot_index < snapshots_count; ++snapshot_index, current_position += interval)
        {
            if(callback && true == callback->IsTerminate())
                return SNAPSHOTS_RESULT_TERMINATED;

            if(callback)
                callback->SetProgress(static_cast<size_t>(snapshot_index * 100.f / snapshots_count)); 

            //get snapshot
            app::byte_buffer image_buffer;
            PBitmap snapshot;
            APP_VERIFY(true == video_file.GetSnapshot(current_position, snapshot, image_buffer));

            //draw thumnail
            const INT thumb_x = (snapshot_index % output_profile.FrameRows) * tile_width;
            const INT thumb_y = (snapshot_index / output_profile.FrameColumns) * tile_height;
            app::verify_gdi(graphics.DrawImage(snapshot.get(), thumb_x, thumb_y, tile_width, tile_height));
        }

        //TODO:
        //make output file name
        //app::string output_file_name;
        //if(NULL == output_dir || 0 == *output_dir)
        //    output_file_name = video_file_name;
        //else
        //{
        //    output_file_name.assign(output_dir);
        //    output_file_name.append(_T("\\"));
        //    output_file_name.append(GetRelativeFileName(video_file_name));
        //}   
        //output_file_name.append(_T("."));
        //output_file_name.append(GetImageFileExt(output_profile.OutputFileFormat));

        //save
        CString output_file_name = GenerateOutputFileName(video_file_name, output_profile, output_dir);
        app::verify_gdi(output_image.Save(output_file_name, &encoder_clsid));
        result_string = output_file_name;
    }
    catch(app::exception& exc)
    {
        LPCTSTR error_string = exc.string();
        if(NULL == error_string)
        {
            TCHAR buf[2*KILOBYTE];
            exc.string(buf, 2*KILOBYTE);
            result_string = buf;
        }
        return SNAPSHOTS_RESULT_FAIL;
    }
    catch(...)
    {
        result_string = _T("Unknown error");
        return SNAPSHOTS_RESULT_FAIL;
    }

    return SNAPSHOTS_RESULT_SUCCESS;
}