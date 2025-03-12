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

void DrawBackgorund(Gdiplus::Graphics& graphics, const COutputProfile& output_profile, INT output_width, INT output_height)
{
    Gdiplus::Color bk_color{ };
    bk_color.SetFromCOLORREF(output_profile.BackgroundColor);
    std::unique_ptr<Gdiplus::SolidBrush> brush{ new Gdiplus::SolidBrush(bk_color) };
    app::verify_gdi(graphics.FillRectangle(brush.get(), Gdiplus::Rect(0, 0, output_width, output_height)));
}

//TODO: how to calculate font height without temp Bitmap and Graphics?
int CHeaderDraw::CalculateHeight(const COutputProfile& output_profile)
{
    Gdiplus::Bitmap temp_image(100, 100, PixelFormat24bppRGB);
    Gdiplus::Graphics temp_graphics(&temp_image);

    LOGFONT lf{};
    output_profile.HeaderFont.Get(lf);
    VP_VERIFY(lf.lfHeight);

    HDC hdc = temp_graphics.GetHDC();
    Gdiplus::Font header_font(hdc, &lf);
    temp_graphics.ReleaseHDC(hdc);

    const int header_font_height = static_cast<size_t>(header_font.GetHeight(&temp_graphics));
    return HEADER_VERTICAL_PADDING + header_font_height * HEADER_LINES_COUNT + HEADER_VERTICAL_PADDING;
}
void CHeaderDraw::DrawPreview()
{
    //sample values
    const int duration = SAMPLE_FRAME_DURATION;
    const int video_width = SAMPLE_FRAME_WIDTH;
    const int video_height = SAMPLE_FRAME_HEIGHT;

    LARGE_INTEGER li{};
    li.QuadPart = 230456; //sample value
    LPCTSTR file_name = _T("sample_frame.bmp"); //sample value
    CString file_size_str = GetFileSizeString(li);
    CString duration_str = GetDurationString(duration);

    CString header_text;
    header_text.Format(HEADER_FORMAT_STRING, file_name, file_size_str, video_width, video_height, duration_str);

    LOGFONT lf{};
    Profile.HeaderFont.Get(lf);
    VP_VERIFY(lf.lfHeight);

    HDC hdc = Graphics.GetHDC();
    Gdiplus::Font header_font(hdc, &lf);
    Graphics.ReleaseHDC(hdc);

    Gdiplus::Color header_font_color;
    header_font_color.SetFromCOLORREF(Profile.HeaderFont.Color);
    Gdiplus::SolidBrush header_brush(header_font_color);
    Gdiplus::PointF pt(0, static_cast<Gdiplus::REAL>(HEADER_VERTICAL_PADDING));

    app::verify_gdi(Graphics.DrawString(header_text, static_cast<INT>(::wcslen(header_text)), &header_font, pt, &header_brush));
}
void CHeaderDraw::Draw(LPCTSTR video_file_name, const CVideoFile& video_file)
{
    const UINT duration = video_file.GetDuration();
    const int video_width = video_file.GetVideoWidth();
    const int video_height = video_file.GetVideoHeight();
    VP_VERIFY(duration > 0);
    VP_VERIFY(video_width > 0);
    VP_VERIFY(video_height > 0);

    CString header_text;

    LARGE_INTEGER li{};
    video_file.GetSize(li);
    CString file_size_str = GetFileSizeString(li);
    CString duration_str = GetDurationString(duration);
    LPCTSTR file_name = GetRelativeFileName(video_file_name);
    header_text.Format(HEADER_FORMAT_STRING, file_name, file_size_str, video_width, video_height, duration_str);

    LOGFONT lf{};
    Profile.HeaderFont.Get(lf);
    VP_VERIFY(lf.lfHeight);

    HDC hdc = Graphics.GetHDC();
    Gdiplus::Font header_font(hdc, &lf);
    Graphics.ReleaseHDC(hdc);

    Gdiplus::Color header_font_color;
    header_font_color.SetFromCOLORREF(Profile.HeaderFont.Color);
    Gdiplus::SolidBrush header_brush(header_font_color);
    Gdiplus::PointF pt(0, static_cast<Gdiplus::REAL>(HEADER_VERTICAL_PADDING));

    app::verify_gdi(Graphics.DrawString(header_text, static_cast<INT>(::wcslen(header_text)), &header_font, pt, &header_brush));
}

CTimeStampDraw::CTimeStampDraw(Gdiplus::Graphics& graphics, const COutputProfile& output_profile) : Graphics(graphics), Profile(output_profile), Font(nullptr), Brush(nullptr), ShadowBrush(nullptr)
{
    if (TIMESTAMP_TYPE_DISABLED == Profile.TimestampType)
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

    //inverted color for shadow
    const COLORREF shadow = ~(output_profile.TimestampFont.Color);
    Gdiplus::Color shadow_color{};
    shadow_color.SetFromCOLORREF(shadow);
    ShadowBrush = new Gdiplus::SolidBrush(shadow_color);
}
CTimeStampDraw::~CTimeStampDraw()
{
    delete Font;
    delete Brush;
    delete ShadowBrush;
}
void CTimeStampDraw::Draw(int timestamp, REAL frame_x, REAL frame_y, REAL frame_width, REAL frame_height)
{
    //text alignment
    Gdiplus::StringFormat str_format;
    Gdiplus::StringAlignment horz_alignment = Gdiplus::StringAlignmentFar;
    if (TIMESTAMP_TYPE_TOP_LEFT == Profile.TimestampType || TIMESTAMP_TYPE_BOTTOM_LEFT == Profile.TimestampType)
        horz_alignment = Gdiplus::StringAlignmentNear;
    else if (TIMESTAMP_TYPE_TOP_CENTER == Profile.TimestampType || TIMESTAMP_TYPE_BOTTOM_CENTER == Profile.TimestampType)
        horz_alignment = Gdiplus::StringAlignmentCenter;
    const Gdiplus::StringAlignment vert_alignment = (TIMESTAMP_TYPE_TOP_LEFT == Profile.TimestampType ||
        TIMESTAMP_TYPE_TOP_CENTER == Profile.TimestampType ||
        TIMESTAMP_TYPE_TOP_RIGHT == Profile.TimestampType) ?
        Gdiplus::StringAlignmentNear : Gdiplus::StringAlignmentFar;
    str_format.SetAlignment(horz_alignment); //horz alignment
    str_format.SetLineAlignment(vert_alignment); //vert alignment

    //timestamp text
    CString timestamp_str(GetDurationString(timestamp));

    //draw timestamp shadow
    Gdiplus::RectF rect_shadow(frame_x + 2, frame_y + 2, frame_width, frame_height);
    app::verify_gdi(Graphics.DrawString(timestamp_str, timestamp_str.GetLength(), Font, rect_shadow, &str_format, ShadowBrush));

    //draw timestamp
    Gdiplus::RectF rect(frame_x, frame_y, frame_width, frame_height);
    app::verify_gdi(Graphics.DrawString(timestamp_str, timestamp_str.GetLength(), Font, rect, &str_format, Brush));
}