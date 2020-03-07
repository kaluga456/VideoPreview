#pragma once

//TODO: this is not required, all colors/fonts settings are on property grid
//view font control
class CFontView : public CStatic
{
public:
    COLORREF FontColor;
    COLORREF BackgroudColor;

    explicit CFontView();
    ~CFontView();

    void Subclass();
    void SetFont(const LOGFONT* font = NULL);
    void SetFont(const CFontData& font_data);

private:
    static const int FONT_HEIGHT = -20;
    static const int MAX_TEXT_SIZE = 64;
    static const int EDGE_SIZE = 2;

    //data
    HFONT Font;
    TCHAR Text[MAX_TEXT_SIZE];

    //window procedure
    static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
};

