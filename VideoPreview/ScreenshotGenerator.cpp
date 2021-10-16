#include "stdafx.h"
#include "app.h"
#include "app_buffer.h"
#include "app_com.h"
#include "app_gdi.h"
#include "app_direct_show.h"
#pragma hdrstop
#include "VPError.h"
#include "Settings.h"
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

static CString GenerateOutputFileName(LPCTSTR video_file_name, LPCTSTR output_dir, const COutputProfile& output_profile) 
{
    ASSERT(video_file_name);

    if(output_dir)
    {
        CString file_name(GetRelativeFileName(video_file_name));
        file_name += _T(".");
        file_name += GetImageFileExt(output_profile.OutputFileFormat);

        CString result(output_dir);
        result += _T("\\");
        result += file_name;
        return result;
    }

    CString result(video_file_name);
    result += _T(".");
    result += GetImageFileExt(output_profile.OutputFileFormat);
    return result;
}

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

int GenerateScreenshots(LPCTSTR video_file_name, LPCTSTR output_dir, const COutputProfile& output_profile, CString& result_string, IScreenshotsCallback* callback /*= NULL*/)
{
    if(NULL == video_file_name)
        return SNAPSHOTS_RESULT_FAIL;

    try
    {
        //check output file
        CString output_file_name(GenerateOutputFileName(video_file_name, output_dir, output_profile));
        if(false == Settings.OverwriteOutputFiles)
        {
            const DWORD file_attr = ::GetFileAttributes(output_file_name);
            if(file_attr != INVALID_FILE_ATTRIBUTES)
            {
                result_string = _T("Output file already exists: ");
                result_string += output_file_name;
                return SNAPSHOTS_RESULT_FAIL;
            }            
        }

        //init COM
        app::com com(COINIT_MULTITHREADED);

        //init gdi
        app::gdi gdi;

        //init video file
        CVideoFile video_file;
        VP_VERIFY(video_file.Open(video_file_name));

        //TODO: write header
        //TODO: file size


        //get video file attr
        const size_t duration = video_file.GetDuration();
        const size_t video_width = video_file.GetVideoWidth();
        const size_t video_height = video_file.GetVideoHeight();
        VP_VERIFY(duration > 0);
        VP_VERIFY(video_width > 0);
        VP_VERIFY(video_height > 0);
        const float aspect_ratio = float(video_width) / float(video_height);
        
        //get snapshots count
        const size_t snapshots_count = output_profile.FrameRows * output_profile.FrameColumns;

        //init output image size
        size_t tile_width = 0;
        size_t tile_height = 0;
        size_t output_width = 0;
        size_t output_height = 0;
        switch(output_profile.OutputSizeMethod)
        {
        case OUTPUT_IMAGE_WIDTH_BY_ORIGINAL_FRAME:
        {
            tile_width = video_width;
            tile_height = video_height;
            output_width = tile_width * output_profile.FrameColumns;
            output_height = tile_height * output_profile.FrameRows;
            break;
        }
        case OUTPUT_IMAGE_WIDTH_AS_IS:
        {
            tile_width = static_cast<size_t>(float(output_profile.OutputImageSize) / float(output_profile.FrameColumns));
            tile_height = static_cast<size_t>(float(tile_width) / aspect_ratio);
            output_width = output_profile.OutputImageSize;
            output_height = tile_height * output_profile.FrameRows;
            break;
        }
        case OUTPUT_IMAGE_WIDTH_BY_FRAME_WIDTH:
        {
            tile_width = output_profile.OutputImageSize;
            tile_height = static_cast<size_t>(float(tile_width) / aspect_ratio);
            output_width = tile_width * output_profile.FrameColumns;
            output_height = tile_height * output_profile.FrameRows;
            break;
        }
        case OUTPUT_IMAGE_WIDTH_BY_FRAME_HEIGHT:
        {
            tile_height = output_profile.OutputImageSize;
            tile_width = static_cast<size_t>(float(tile_height) * aspect_ratio);
            output_width = tile_width * output_profile.FrameColumns;
            output_height = tile_height * output_profile.FrameRows;
            break;
        }
        default:
            VP_THROW(_T("Uknown output_profile.OutputSizeMethod"));
        }

        //TODO:
        VP_VERIFY(0 < tile_width && tile_width < 4096);
        VP_VERIFY(0 < tile_height && tile_height < 4096);
        VP_VERIFY(0 < output_width && output_width < 4096);
        VP_VERIFY(0 < output_height && output_height < 4096);

        Gdiplus::Bitmap output_image(output_width, output_height, PixelFormat24bppRGB);
        Gdiplus::Graphics graphics(&output_image);

        //get image encoder CLSID
        app::gdi_encoders encoders;
        app::verify_gdi(encoders.initialize());
        CLSID encoder_clsid;
        VP_VERIFY(true == encoders.encoder_clsid(get_mime_type_string(output_profile.OutputFileFormat), encoder_clsid));

        //build output image
        size_t interval = duration / (snapshots_count + 1);
        size_t current_position = interval;
        size_t snapshot_index = 0;
        for (int y = 0; y < output_profile.FrameRows; ++y)
        {
            for (int x = 0; x < output_profile.FrameColumns; ++x)
            {
                if(callback && true == callback->IsTerminate())
                    return SNAPSHOTS_RESULT_TERMINATED;

                if(callback)
                    callback->SetProgress(static_cast<size_t>(snapshot_index * 100.f / snapshots_count)); 

                //get frame
                app::byte_buffer image_buffer;
                PBitmap snapshot;
                VP_VERIFY(true == video_file.GetSnapshot(current_position, snapshot, image_buffer));

                //write frame
                const INT thumb_x = x * tile_width;
                const INT thumb_y = y * tile_height;
                app::verify_gdi(graphics.DrawImage(snapshot.get(), thumb_x, thumb_y, tile_width, tile_height));

                current_position += interval;
                ++snapshot_index;
            }
        }

        //save
        app::verify_gdi(output_image.Save(output_file_name, &encoder_clsid));
        result_string = output_file_name;
        return SNAPSHOTS_RESULT_SUCCESS;
    }

    //DEPRECATE:
    catch(app::exception& exc)
    {
        LPCTSTR error_string = exc.string();
        if(NULL == error_string)
        {
            TCHAR buf[2*KILOBYTE];
            exc.string(buf, 2*KILOBYTE);
            result_string = buf;
        }
    }

    catch(CVPExc& exc)
    {
        result_string = exc.GetFullText();
    }
    catch(...)
    {
        result_string = VP_UNKNOWN_ERROR_STRING;
    }

    return SNAPSHOTS_RESULT_FAIL;
}
int GenerateProfilePreview(LPCTSTR video_file_name, LPCTSTR output_dir, const COutputProfile& output_profile, CString& result_string)
{
    //TODO:
    if(NULL == video_file_name)
        return SNAPSHOTS_RESULT_FAIL;

    try
    {

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
    return SNAPSHOTS_RESULT_FAIL;
}