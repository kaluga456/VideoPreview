#include "stdafx.h"
#pragma hdrstop
#include "OutputProfile.h"
#include "FontView.h"

LRESULT CALLBACK CFontView::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    switch(uMsg)
    {
    case WM_PAINT:
    {
        //init
        const CFontView* data = reinterpret_cast<const CFontView*>(dwRefData);
        ASSERT(data);
        PAINTSTRUCT ps;
        ::BeginPaint(hWnd, &ps);

        ::DrawEdge(ps.hdc, &ps.rcPaint, BDR_RAISED, BF_RECT);

        //draw background
        RECT rect = ps.rcPaint;
        ::InflateRect(&rect, -EDGE_SIZE, -EDGE_SIZE);
        CBrush background_brush;
        background_brush.CreateSolidBrush(data->BackgroudColor);
        ::FillRect(ps.hdc, &rect, background_brush);
        
        //draw font
        if(NULL == data->Font)
        {
            ::EndPaint(hWnd, &ps);
            return TRUE;
        }
        const int text_size = ::_tcslen(data->Text);
        if(text_size > 0)
        {
            ::SelectObject(ps.hdc, data->Font);
            ::SetTextColor(ps.hdc, data->FontColor);
            ::SetBkMode(ps.hdc, TRANSPARENT);
            ::DrawText(ps.hdc, data->Text, text_size, &rect, DT_CENTER | /*DT_NOCLIP |*/ DT_NOPREFIX | DT_SINGLELINE | DT_VCENTER);
        }

        ::EndPaint(hWnd, &ps);
        return TRUE;
    }
    } 
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

CFontView::CFontView() : Font(NULL), BackgroudColor(RGB(0x00, 0x00, 0x00))
{
}
CFontView::~CFontView()
{
    if(Font != NULL)
        ::DeleteObject(Font);
}
void CFontView::Subclass()
{
    ASSERT(::IsWindow(m_hWnd));
    SetWindowSubclass(m_hWnd, WindowProc, 0, reinterpret_cast<DWORD_PTR>(this));
}
void CFontView::SetFont(const LOGFONT* font /*= NULL*/)
{
    ASSERT(::IsWindow(m_hWnd));
    if(Font != NULL)
        ::DeleteObject(Font);

    Font = font ? ::CreateFontIndirect(font) : NULL;
    if(Font != NULL)
        ::_tcsncpy_s(Text, MAX_TEXT_SIZE, font->lfFaceName, _TRUNCATE);
}
void CFontView::SetFont(const CFontData& font_data)
{
    ASSERT(::IsWindow(m_hWnd));
    if(Font != NULL)
        ::DeleteObject(Font);

    FontColor = font_data.Color;

    LOGFONT font;
    font.lfHeight = FONT_HEIGHT;
    font.lfWidth = 0;
    font.lfEscapement = 0;
    font.lfOrientation = 0;
    font.lfWeight = font_data.Weight;
    font.lfItalic = FALSE;
    font.lfUnderline = FALSE;
    font.lfStrikeOut = FALSE;
    font.lfCharSet = font_data.Charset; 
    font.lfOutPrecision = OUT_DEFAULT_PRECIS;
    font.lfClipPrecision = CLIP_DEFAULT_PRECIS;
    font.lfQuality = DEFAULT_QUALITY;
    font.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
    ::_tcsncpy_s(font.lfFaceName, LF_FACESIZE, font_data.Face, _TRUNCATE);

    Font = ::CreateFontIndirect(&font);
    if(NULL == Font)
    {
        //TODO: handle error
        return;
    }

    //font description text
    const int font_point_size = -::MulDiv(72, font_data.Size, ::GetDeviceCaps(::GetDC(m_hWnd), LOGPIXELSY));
    ::swprintf_s(Text, MAX_TEXT_SIZE, _T("%s : %d"), font.lfFaceName, font_point_size);

    Invalidate(FALSE);
}
