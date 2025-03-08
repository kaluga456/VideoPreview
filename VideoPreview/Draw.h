#pragma once

using Gdiplus::REAL;

//background
void DrawBackgorund(Gdiplus::Graphics& graphics, const COutputProfile& output_profile, INT output_width, INT output_height);

//sample values for profile preview
constexpr int SAMPLE_FRAME_DURATION = 0; //sec
constexpr int SAMPLE_FRAME_WIDTH = 320;
constexpr int SAMPLE_FRAME_HEIGHT = 200;

//header painter
constexpr size_t HEADER_VERTICAL_PADDING = 5; //vertical padding, px
constexpr size_t HEADER_LINES_COUNT = 4;
class CHeaderDraw
{
public:
    CHeaderDraw(Gdiplus::Graphics& graphics, const COutputProfile& output_profile) : Graphics(graphics), Profile(output_profile) {}
    ~CHeaderDraw() {}

    void DrawPreview();
    void Draw(LPCTSTR video_file_name, const CVideoFile& video_file);
    static int CalculateHeight(const COutputProfile& output_profile);

private:
    static constexpr LPCTSTR HEADER_FORMAT_STRING = _T("File Name: %s\nFile Size: %s\nResolution: %ux%u\nDuration: %s");

    const COutputProfile& Profile;
    Gdiplus::Graphics& Graphics;
};

//timestamp painter
class CTimeStampDraw
{
public:
    CTimeStampDraw(Gdiplus::Graphics& graphics, const COutputProfile& output_profile);
    ~CTimeStampDraw();

    void Draw(int timestamp, REAL frame_x, REAL frame_y, REAL frame_width, REAL frame_height);

private:
    const COutputProfile& Profile;
    Gdiplus::Graphics& Graphics;

    Gdiplus::Font* Font;
    Gdiplus::SolidBrush* Brush;
    Gdiplus::SolidBrush* ShadowBrush;
};