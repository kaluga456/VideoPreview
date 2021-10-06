#include "stdafx.h"
#pragma hdrstop
#include "About.h"
#include "Resource.h"
#include "DialogAbout.h"

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
    ON_WM_DESTROY()
END_MESSAGE_MAP()

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD), URLFont(NULL), URLCursor(NULL)
{
}
BOOL CAboutDlg::OnInitDialog()
{
    SetWindowText(_T("About ") APP_NAME);

    CStatic* url_ctrl = static_cast<CStatic*>(GetDlgItem(IDC_STATIC_APP_URL));

    CFont* font = url_ctrl->GetFont();    
    LOGFONT lf;
    font->GetLogFont(&lf);
    lf.lfUnderline = TRUE;
    //TODO: font color
    URLFont = ::CreateFontIndirect(&lf);
    if(URLFont)
    {
        CFont* new_font = CFont::FromHandle(URLFont);
        url_ctrl->SetFont(new_font, TRUE);
    }

    //TODO:
    URLCursor = ::LoadCursor(NULL, IDC_HAND); 
    if(URLCursor) url_ctrl->SetCursor(URLCursor);

    return CDialogEx::OnInitDialog();
}
void CAboutDlg::OnDestroy()
{
    if(URLFont) ::DeleteObject(URLFont);
    //if(URLCursor) ::DeleteObject(URLCursor);
    CDialogEx::OnDestroy();
}
void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

    if(pDX->m_bSaveAndValidate) return;
    DDX_Text(pDX, IDC_STATIC_VERSION, CString(_T("Build: ") APP_BUILD));
    DDX_Text(pDX, IDC_STATIC_APP_URL, CString(APP_URL));
}
