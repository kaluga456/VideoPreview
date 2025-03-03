#include "stdafx.h"
#include "app.h"
#include "app_buffer.h"
#include "app_com.h"
#include "app_gdi.h"
#include "app_direct_show.h"
#pragma hdrstop
#include "Resource.h"
#include "VPError.h"
#include "Settings.h"
#include "OutputProfile.h"
#include "VideoFile.h"
#include "ScreenshotGenerator.h"

using Gdiplus::REAL;

const size_t HEADER_LINES_COUNT = 4;

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

static CString GetDurationString(int duration)
{
    LPCTSTR format_string = _T("%02u:%02u:%02u");

    const int seconds = duration % 60;
    const int minutes = (duration / 60) % 60;
    const int hours = duration / 3600;

    CString result;
    result.Format(format_string, hours, minutes, seconds);

    return result;
}

static CString GetFileSizeString(const LARGE_INTEGER& file_size)
{
    CString size_string;
    uint32_t size_value = 0;
    const QWORD terabyte = QWORD(1024 * 1024) * QWORD(1024 * 1024);
    if(file_size.QuadPart >= terabyte)
    {
        size_value = static_cast<uint32_t>(file_size.QuadPart / terabyte);
        size_string.Format(_T("%u TB"), size_value);
    }
    else if(file_size.QuadPart >= 1024 * 1024 * 1024)
    {
        size_value = static_cast<uint32_t>(file_size.QuadPart / (1024 * 1024 * 1024));
        size_string.Format(_T("%u GB"), size_value);
    }
    else if(file_size.QuadPart >= 1024 * 1024)
    {
        size_value = static_cast<uint32_t>(file_size.QuadPart / (1024 * 1024));
        size_string.Format(_T("%u MB"), size_value);
    }
    else if(file_size.QuadPart >= 1024)
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
    if(file_size.HighPart)
    {
        bytes_string.Format(_T(" (%I64u bytes)"), file_size.QuadPart);
    }
    else
    {
        bytes_string.Format(_T(" (%u bytes)"), file_size.LowPart);
    }

    return size_string + bytes_string;
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

void DrawTimestamp(Gdiplus::Graphics& graphics, Gdiplus::Font& font,  Gdiplus::Brush& brush, size_t timestamp, REAL frame_x, REAL frame_y, REAL frame_width, REAL frame_height, int position)
{
    CString timestamp_str(GetDurationString(timestamp));
    Gdiplus::RectF rect(frame_x, frame_y, frame_width, frame_height);

    Gdiplus::StringFormat str_format;
    Gdiplus::StringAlignment horz_alignment = Gdiplus::StringAlignmentFar;
    if(TIMESTAMP_TYPE_TOP_LEFT == position || TIMESTAMP_TYPE_BOTTOM_LEFT == position)
        horz_alignment = Gdiplus::StringAlignmentNear;
    else if(TIMESTAMP_TYPE_TOP_CENTER == position || TIMESTAMP_TYPE_BOTTOM_CENTER == position)
        horz_alignment = Gdiplus::StringAlignmentCenter;
    const Gdiplus::StringAlignment vert_alignment = (TIMESTAMP_TYPE_TOP_LEFT == position || 
                                TIMESTAMP_TYPE_TOP_CENTER == position || 
                                TIMESTAMP_TYPE_TOP_RIGHT == position) ?
                                Gdiplus::StringAlignmentNear : Gdiplus::StringAlignmentFar;

    str_format.SetAlignment(horz_alignment); //horz alignment
    str_format.SetLineAlignment(vert_alignment); //vert alignment

    //TEST:
    Gdiplus::Color timestamp_font_color_shadow(0, 0, 0);
    Gdiplus::SolidBrush timestamp_brush_shadow(timestamp_font_color_shadow);
    Gdiplus::RectF rect_shadow(frame_x + 2, frame_y + 2, frame_width, frame_height);
    app::verify_gdi(graphics.DrawString(timestamp_str, timestamp_str.GetLength(), &font, rect_shadow, &str_format, &timestamp_brush_shadow));

    app::verify_gdi(graphics.DrawString(timestamp_str, timestamp_str.GetLength(), &font, rect, &str_format, &brush));
}

//TODO:
class CTimeStampDraw
{
public:
    CTimeStampDraw(Gdiplus::Graphics& graphics, const COutputProfile& output_profile) : Graphics(graphics), Profile(output_profile), Font(NULL) , Brush(NULL), ShadowBrush(NULL)
    {
        if(TIMESTAMP_TYPE_DISABLED == Profile.TimestampType)
            return;

        LOGFONT ts_lf;
        Profile.TimestampFont.Get(ts_lf);
        VP_VERIFY(ts_lf.lfHeight);

        HDC hdc = graphics.GetHDC();
        Font = new Gdiplus::Font(hdc, &ts_lf);
        graphics.ReleaseHDC(hdc);

        Gdiplus::Color font_color;
        font_color.SetFromCOLORREF(output_profile.TimestampFont.Color);
   
        Brush = new Gdiplus::SolidBrush(font_color);

        //TODO: inverted color
        Gdiplus::Color shadow_color(0, 0, 0);
        ShadowBrush = new Gdiplus::SolidBrush(shadow_color);
    }
    ~CTimeStampDraw()
    {
        delete Font;
        delete Brush;
        delete ShadowBrush;
    }

    void Draw(size_t timestamp, REAL frame_x, REAL frame_y, REAL frame_width, REAL frame_height);

private:
    const COutputProfile& Profile;
    Gdiplus::Graphics& Graphics;

    Gdiplus::Font* Font;  
    Gdiplus::SolidBrush* Brush;
    Gdiplus::SolidBrush* ShadowBrush;
};
void CTimeStampDraw::Draw(size_t timestamp, REAL frame_x, REAL frame_y, REAL frame_width, REAL frame_height)
{
    if(TIMESTAMP_TYPE_DISABLED == Profile.TimestampType)
        return;

    CString timestamp_str(GetDurationString(timestamp));
    Gdiplus::RectF rect(frame_x, frame_y, frame_width, frame_height);

    Gdiplus::StringFormat str_format;
    Gdiplus::StringAlignment horz_alignment = Gdiplus::StringAlignmentFar;
    if(TIMESTAMP_TYPE_TOP_LEFT == Profile.TimestampType || TIMESTAMP_TYPE_BOTTOM_LEFT == Profile.TimestampType)
        horz_alignment = Gdiplus::StringAlignmentNear;
    else if(TIMESTAMP_TYPE_TOP_CENTER == Profile.TimestampType || TIMESTAMP_TYPE_BOTTOM_CENTER == Profile.TimestampType)
        horz_alignment = Gdiplus::StringAlignmentCenter;
    const Gdiplus::StringAlignment vert_alignment = (TIMESTAMP_TYPE_TOP_LEFT == Profile.TimestampType || 
                                TIMESTAMP_TYPE_TOP_CENTER == Profile.TimestampType || 
                                TIMESTAMP_TYPE_TOP_RIGHT == Profile.TimestampType) ?
                                Gdiplus::StringAlignmentNear : Gdiplus::StringAlignmentFar;

    str_format.SetAlignment(horz_alignment); //horz alignment
    str_format.SetLineAlignment(vert_alignment); //vert alignment

    //TODO:
    Gdiplus::RectF rect_shadow(frame_x + 2, frame_y + 2, frame_width, frame_height);
    app::verify_gdi(Graphics.DrawString(timestamp_str, timestamp_str.GetLength(), Font, rect_shadow, &str_format, ShadowBrush));

    app::verify_gdi(Graphics.DrawString(timestamp_str, timestamp_str.GetLength(), Font, rect, &str_format, Brush));
}

//TODO:
class CHeaderDraw
{
public:
private:
};

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
        const size_t frame_count = output_profile.FrameRows * output_profile.FrameColumns;

        //init output image size
        size_t frame_width = 0;
        size_t frame_height = 0;
        size_t output_width = 0;
        size_t output_height = 0;
        switch(output_profile.OutputSizeMethod)
        {
        case OUTPUT_IMAGE_WIDTH_BY_ORIGINAL_FRAME:
        {
            frame_width = video_width;
            frame_height = video_height;
            output_width = frame_width * output_profile.FrameColumns;
            output_height = frame_height * output_profile.FrameRows;
            break;
        }
        case OUTPUT_IMAGE_WIDTH_AS_IS:
        {
            frame_width = static_cast<size_t>(float(output_profile.OutputImageSize) / float(output_profile.FrameColumns));
            frame_height = static_cast<size_t>(float(frame_width) / aspect_ratio);
            output_width = output_profile.OutputImageSize;
            output_height = frame_height * output_profile.FrameRows;
            break;
        }
        case OUTPUT_IMAGE_WIDTH_BY_FRAME_WIDTH:
        {
            frame_width = output_profile.OutputImageSize;
            frame_height = static_cast<size_t>(float(frame_width) / aspect_ratio);
            output_width = frame_width * output_profile.FrameColumns;
            output_height = frame_height * output_profile.FrameRows;
            break;
        }
        case OUTPUT_IMAGE_WIDTH_BY_FRAME_HEIGHT:
        {
            frame_height = output_profile.OutputImageSize;
            frame_width = static_cast<size_t>(float(frame_height) * aspect_ratio);
            output_width = frame_width * output_profile.FrameColumns;
            output_height = frame_height * output_profile.FrameRows;
            break;
        }
        default:
            VP_THROW(_T("Uknown output_profile.OutputSizeMethod"));
        }

        //TODO:
        VP_VERIFY(0 < frame_width && frame_width < 4096);
        VP_VERIFY(0 < frame_height && frame_height < 4096);
        VP_VERIFY(0 < output_width && output_width < 4096);
        VP_VERIFY(0 < output_height && output_height < 4096);

        Gdiplus::Bitmap output_image(output_width, output_height, PixelFormat24bppRGB);
        Gdiplus::Graphics graphics(&output_image);

        //get image encoder CLSID
        app::gdi_encoders encoders;
        app::verify_gdi(encoders.initialize());
        CLSID encoder_clsid;
        VP_VERIFY(true == encoders.encoder_clsid(GetMIMETypeString(output_profile.OutputFileFormat), encoder_clsid));

        CTimeStampDraw timestamp_draw(graphics, output_profile);

        //build output image
        size_t interval = duration / (frame_count + 1);
        size_t current_position = interval;
        size_t frame_index = 0;
        for(int y = 0; y < output_profile.FrameRows; ++y)
        {
            for (int x = 0; x < output_profile.FrameColumns; ++x)
            {
                if(callback && true == callback->IsTerminate())
                    return SNAPSHOTS_RESULT_TERMINATED;

                if(callback)
                    callback->SetProgress(static_cast<size_t>(frame_index * 100.f / frame_count)); 

                //get frame
                app::byte_buffer image_buffer;
                PBitmap snapshot;
                VP_VERIFY(true == video_file.GetSnapshot(current_position, snapshot, image_buffer));

                //write frame
                const INT frame_left = x * frame_width;
                const INT frame_top = y * frame_height;
                app::verify_gdi(graphics.DrawImage(snapshot.get(), frame_left, frame_top, frame_width, frame_height));

                //timestamp
                timestamp_draw.Draw(current_position, static_cast<Gdiplus::REAL>(frame_left), static_cast<Gdiplus::REAL>(frame_top),
                    static_cast<Gdiplus::REAL>(frame_width), static_cast<Gdiplus::REAL>(frame_height));

                //ok - go to next frame
                current_position += interval;
                ++frame_index;
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
int GenerateProfilePreview(const COutputProfile& output_profile, CString& result_string)
{
    //sample preview settings
    LPCTSTR video_file_name = _T("dummy_frame.bmp");

    try
    {
        //TODO: already initialized in main thread
        //init COM
        //app::com com(COINIT_MULTITHREADED);

        //init gdi
        app::gdi gdi;

        //get video file attr
        const size_t duration = 0;
        const size_t video_width = 320;
        const size_t video_height = 200;
        const float aspect_ratio = float(video_width) / float(video_height);
        
        //get snapshots count
        const size_t frame_count = output_profile.FrameRows * output_profile.FrameColumns;

        //init output image size
        size_t frame_width = 0;
        size_t frame_height = 0;
        size_t output_width = 0;
        size_t output_height = 0;
        switch(output_profile.OutputSizeMethod)
        {
        case OUTPUT_IMAGE_WIDTH_BY_ORIGINAL_FRAME:
        {
            frame_width = video_width;
            frame_height = video_height;
            output_width = frame_width * output_profile.FrameColumns;
            output_height = frame_height * output_profile.FrameRows;
            break;
        }
        case OUTPUT_IMAGE_WIDTH_AS_IS:
        {
            frame_width = static_cast<size_t>(float(output_profile.OutputImageSize) / float(output_profile.FrameColumns));
            frame_height = static_cast<size_t>(float(frame_width) / aspect_ratio);
            output_width = output_profile.OutputImageSize;
            output_height = frame_height * output_profile.FrameRows;
            break;
        }
        case OUTPUT_IMAGE_WIDTH_BY_FRAME_WIDTH:
        {
            frame_width = output_profile.OutputImageSize;
            frame_height = static_cast<size_t>(float(frame_width) / aspect_ratio);
            output_width = frame_width * output_profile.FrameColumns;
            output_height = frame_height * output_profile.FrameRows;
            break;
        }
        case OUTPUT_IMAGE_WIDTH_BY_FRAME_HEIGHT:
        {
            frame_height = output_profile.OutputImageSize;
            frame_width = static_cast<size_t>(float(frame_height) * aspect_ratio);
            output_width = frame_width * output_profile.FrameColumns;
            output_height = frame_height * output_profile.FrameRows;
            break;
        }
        default:
            VP_THROW(_T("Uknown output_profile.OutputSizeMethod"));
        }

        //TODO:
        VP_VERIFY(0 < frame_width && frame_width < 4096);
        VP_VERIFY(0 < frame_height && frame_height < 4096);
        VP_VERIFY(0 < output_width && output_width < 4096);
        VP_VERIFY(0 < output_height && output_height < 4096);

        //use dummy frame for all frames in grid
        PBitmap snapshot(new Gdiplus::Bitmap(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_DUMMY_FRAME)));

        //TODO: write header
        //TODO: file size
        const size_t header_padding = 5; //vertical padding, px
        
        size_t header_height = 0;
        size_t header_font_height = 0;

        //calculate header height
        //TODO: how to calculate font height without temp Bitmap and Graphics?
        if(output_profile.WriteHeader)
        {
            Gdiplus::Bitmap temp_image(100, 100, PixelFormat24bppRGB);
            Gdiplus::Graphics temp_graphics(&temp_image);

            LOGFONT lf;
            output_profile.HeaderFont.Get(lf);
            VP_VERIFY(lf.lfHeight);

            HDC hdc = temp_graphics.GetHDC();
            Gdiplus::Font header_font(hdc, &lf);
            temp_graphics.ReleaseHDC(hdc);

            header_font_height = static_cast<size_t>(header_font.GetHeight(&temp_graphics));
            header_height = header_padding + header_font_height * HEADER_LINES_COUNT + header_padding;
        }

        //init output image
        Gdiplus::Bitmap output_image(output_width, header_height + output_height, PixelFormat24bppRGB);
        Gdiplus::Graphics graphics(&output_image);

        //TODO: write header
        //TODO: file size
        if(output_profile.WriteHeader)
        {
            CString header_text;

            //TODO:
            LPCTSTR const HEADER_FORMAT_STRING = _T("File Name: %s\nFile Size: %s\nResolution: %ux%u\nDuration: %s");
            LARGE_INTEGER li;
            li.QuadPart = 230454;
            CString file_size_str = GetFileSizeString(li);
            CString duration_str = GetDurationString(duration);
            header_text.Format(HEADER_FORMAT_STRING, video_file_name, file_size_str, 320, 240, duration_str);

            LOGFONT lf;
            output_profile.HeaderFont.Get(lf);
            VP_VERIFY(lf.lfHeight);
            
            HDC hdc = graphics.GetHDC();
            Gdiplus::Font header_font(hdc, &lf);
            graphics.ReleaseHDC(hdc);
           
            Gdiplus::Color header_font_color;
            header_font_color.SetFromCOLORREF(output_profile.HeaderFont.Color);
            Gdiplus::SolidBrush header_brush(header_font_color);
            Gdiplus::PointF pt(0, static_cast<Gdiplus::REAL>(header_padding));

            app::verify_gdi(graphics.DrawString(header_text, ::wcslen(header_text), &header_font, pt, &header_brush));
        }

        //TODO:
        //if(output_profile.TimestampType != TIMESTAMP_TYPE_DISABLED)
        //{
            //LOGFONT ts_lf;
            //output_profile.TimestampFont.Get(ts_lf);
            //VP_VERIFY(ts_lf.lfHeight);

            //HDC hdc = graphics.GetHDC();
            //Gdiplus::Font timestamp_font(hdc, &ts_lf);
            //graphics.ReleaseHDC(hdc);

            //Gdiplus::Color timestamp_font_color;
            //timestamp_font_color.SetFromCOLORREF(output_profile.TimestampFont.Color);
            //Gdiplus::SolidBrush timestamp_brush(timestamp_font_color);
        //}
        
        CTimeStampDraw timestamp_draw(graphics, output_profile);

        //draw frames grid
        size_t interval = duration / (frame_count + 1);
        size_t current_position = interval;
        size_t frame_index = 0;
        for (int y = 0; y < output_profile.FrameRows; ++y)
        {
            for (int x = 0; x < output_profile.FrameColumns; ++x)
            {
                //get frame
                //app::byte_buffer image_buffer;
                //VP_VERIFY(true == video_file.GetSnapshot(current_position, snapshot, image_buffer));

                //frame image
                const INT frame_left = x * frame_width;
                const INT frame_top = header_height + y * frame_height;
                app::verify_gdi(graphics.DrawImage(snapshot.get(), frame_left, frame_top, frame_width, frame_height));

                //timestamp
                timestamp_draw.Draw(current_position, static_cast<REAL>(frame_left), static_cast<REAL>(frame_top),
                                    static_cast<REAL>(frame_width), static_cast<REAL>(frame_height));

                //ok - go to next frames
                current_position += interval;
                ++frame_index;
            }
        }

        //get output file name
        CString output_dir;
        output_dir.GetEnvironmentVariable(_T("TEMP"));
        CString output_file_name;
        if(output_dir.IsEmpty())
            output_file_name = _T("vp_profile_preview.");
        else
            output_file_name = output_dir + _T("\\vp_profile_preview.");
        output_file_name += GetImageFileExt(output_profile.OutputFileFormat);

        //get image encoder CLSID
        app::gdi_encoders encoders;
        app::verify_gdi(encoders.initialize());
        CLSID encoder_clsid;
        VP_VERIFY(true == encoders.encoder_clsid(GetMIMETypeString(output_profile.OutputFileFormat), encoder_clsid));

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

    catch(...)
    {
        result_string = _T("Unknown error");
    }

    return SNAPSHOTS_RESULT_FAIL;
}