#include "stdafx.h"
#include "app_com.h"
#include "app_gdi.h"
#pragma hdrstop
#include "Resource.h"
#include "VPError.h"
#include "Settings.h"
#include "OutputProfile.h"
#include "VideoFile.h"
#include "Draw.h"
#include "ScreenshotGenerator.h"

constexpr LPCTSTR SAMPLE_OUTPUT_FILE_NAME = _T("vp_profile_preview");

static LPCTSTR GetImageFileExt(int image_type)
{
    switch(image_type)
    {
    case OUTPUT_FILE_FORMAT_BMP: return _T("bmp");
    case OUTPUT_FILE_FORMAT_JPG: return _T("jpg");
    case OUTPUT_FILE_FORMAT_PNG: return _T("png");
    }

    ASSERT(FALSE);
    return _T("");
}

static LPCTSTR GetMIMETypeString(int image_type)
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

static CString GenerateOutputFileName(const COutputProfile& profile, CString video_file_name = CString(), CString output_dir = CString())
{
    CString result;

    //profile preview
    if (video_file_name.IsEmpty())
    {
        //save in TEMP dir
        CString temp_dir;
        if (temp_dir.GetEnvironmentVariable(L"TEMP"))
            temp_dir.AppendChar(L'\\');
        result = temp_dir + SAMPLE_OUTPUT_FILE_NAME;
    }

    //video file
    else
    {
        //save in video file dir
        if (output_dir.IsEmpty())
            result = video_file_name;

        //save in custom dir
        else
        {
            result = output_dir;
            result.AppendChar(L'\\');
            result.Append(GetRelativeFileName(video_file_name));
        }
    }

    //add file ext
    result.AppendChar(L'.');
    result += GetImageFileExt(profile.OutputFileFormat);

    return result;
}

class CEncoderCLSID
{
public:
    CEncoderCLSID(const COutputProfile& profile)
    {
        //get image encoder CLSID
        app::gdi_encoders encoders;
        app::verify_gdi(encoders.initialize());
        VP_VERIFY(true == encoders.encoder_clsid(GetMIMETypeString(profile.OutputFileFormat), EncoderCLSID));
    }

    const CLSID* Get() const { return &EncoderCLSID; }

private:
    CLSID EncoderCLSID;
};

//output sizes calculation
struct COutputImageSize
{
    COutputImageSize(const COutputProfile& profile, int video_width, int video_height) : HeaderHeight{0}
    {
        const float aspect_ratio = float(video_width) / float(video_height);
        FrameCount = profile.FrameRows * profile.FrameColumns;
        switch (profile.OutputSizeMethod)
        {
        case OUTPUT_IMAGE_WIDTH_BY_ORIGINAL_FRAME:
        {
            FrameWidth = video_width;
            FrameHeight = video_height;
            Width = FrameWidth * profile.FrameColumns;
            Height = FrameHeight * profile.FrameRows;
            break;
        }
        case OUTPUT_IMAGE_WIDTH_AS_IS:
        {
            FrameWidth = static_cast<int>(float(profile.OutputImageSize) / float(profile.FrameColumns));
            FrameHeight = static_cast<int>(FrameWidth / aspect_ratio);
            Width = profile.OutputImageSize;
            Height = FrameHeight * profile.FrameRows;
            break;
        }
        case OUTPUT_IMAGE_WIDTH_BY_FRAME_WIDTH:
        {
            FrameWidth = profile.OutputImageSize;
            FrameHeight = static_cast<int>(FrameWidth / aspect_ratio);
            Width = FrameWidth * profile.FrameColumns;
            Height = FrameHeight * profile.FrameRows;
            break;
        }
        case OUTPUT_IMAGE_WIDTH_BY_FRAME_HEIGHT:
        {
            FrameHeight = profile.OutputImageSize;
            FrameWidth = static_cast<int>(FrameHeight * aspect_ratio);
            Width = FrameWidth * profile.FrameColumns;
            Height = FrameHeight * profile.FrameRows;
            break;
        }
        default:
            VP_THROW(_T("Uknown profile.OutputSizeMethod"));
        }

        //TODO:
        VP_VERIFY(0 < FrameWidth && FrameWidth < 4096);
        VP_VERIFY(0 < FrameHeight && FrameHeight < 4096);
        VP_VERIFY(0 < Width && Width < 4096);
        VP_VERIFY(0 < Height && Height < 4096);

        //calculate header height
        if (profile.WriteHeader)
        {
            HeaderHeight = CHeaderDraw::CalculateHeight(profile);
            Height += HeaderHeight;
        }
    }

    //sizes
    int HeaderHeight{0};
    int FrameWidth{0};
    int FrameHeight{0};
    int Width{0};
    int Height{0};
    int FrameCount{0};
};

int GenerateScreenlist(const COutputProfile& output_profile, CString& result_string, 
    CString video_file_name /*= CString()*/, CString output_dir /*= CString()*/, IScreenshotsCallback* callback /*= nullptr*/)
{
    try
    {
        //profile preview
        const bool is_preview = video_file_name.IsEmpty();

        //check output file
        CString output_file_name(GenerateOutputFileName(output_profile, video_file_name, output_dir));
        if(false == is_preview && false == Settings.OverwriteOutputFiles)
        {
            const DWORD file_attr = ::GetFileAttributes(output_file_name);
            if(file_attr != INVALID_FILE_ATTRIBUTES)
            {
                result_string = _T("Output file already exists: ");
                result_string += output_file_name;
                return SCREENLIST_RESULT_FAIL;
            }            
        }

        //init COM
        //NOTE: profile preview generation runs in main thread
        app::com com(is_preview ? COINIT_APARTMENTTHREADED : COINIT_MULTITHREADED);

        //init gdi
        app::gdi gdi;

        //init video file
        CVideoFile video_file(video_file_name);

        //get video file attr
        const int duration = video_file.GetDuration();
        const int video_width = video_file.GetVideoWidth();
        const int video_height = video_file.GetVideoHeight();
        VP_VERIFY(video_width > 0);
        VP_VERIFY(video_height > 0);

        //calculate sizes
        COutputImageSize output_size(output_profile, video_width, video_height);

        //graphics
        Gdiplus::Bitmap output_image(output_size.Width, output_size.Height, PixelFormat24bppRGB);
        Gdiplus::Graphics graphics(&output_image);

        //draw background
        DrawBackgorund(graphics, output_profile, output_size.Width, output_size.Height);

        //write header
        if (output_profile.WriteHeader)
        {
            CHeaderDraw header_draw(graphics, output_profile);
            header_draw.Draw(video_file);
        }

        //write frames
        CTimeStampDraw timestamp_draw(graphics, output_profile);
        const int interval = duration / (output_size.FrameCount + 1);
        int current_position = interval;
        size_t frame_index = 0;
        for(int y = 0; y < output_profile.FrameRows; ++y)
        {
            for (int x = 0; x < output_profile.FrameColumns; ++x)
            {
                if(callback && true == callback->IsTerminate())
                    return SCREENLIST_RESULT_TERMINATED;

                if(callback)
                    callback->SetProgress(static_cast<size_t>(frame_index * 100.f / output_size.FrameCount));

                //get frame
                PBitmap frame = video_file.GetFrameImage(current_position);

                //write frame
                const INT frame_left = x * output_size.FrameWidth;
                const INT frame_top = output_size.HeaderHeight + y * output_size.FrameHeight;
                app::verify_gdi(graphics.DrawImage(frame.get(), frame_left, frame_top, output_size.FrameWidth, output_size.FrameHeight));

                //timestamp
                if (TIMESTAMP_TYPE_DISABLED != output_profile.TimestampType)
                {
                    timestamp_draw.Draw(current_position, static_cast<REAL>(frame_left), static_cast<REAL>(frame_top),
                        static_cast<REAL>(output_size.FrameWidth), static_cast<REAL>(output_size.FrameHeight));
                }

                //go to next frame
                current_position += interval;
                ++frame_index;
            }
        }

        //save
        CEncoderCLSID encoder_clsid(output_profile);
        app::verify_gdi(output_image.Save(output_file_name, encoder_clsid.Get()));
        result_string = output_file_name;
        return SCREENLIST_RESULT_SUCCESS;
    }
    catch(CVPExc& exc)
    {
        result_string = exc.GetFullText();
    }
    catch(...)
    {
        result_string = VP_UNKNOWN_ERROR_STRING;
    }

    return SCREENLIST_RESULT_FAIL;
}
