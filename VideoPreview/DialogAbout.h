#pragma once

// CAboutDlg dialog used for App About
class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();
	enum { IDD = IDD_ABOUTBOX };

private:
    HFONT URLFont;
    HCURSOR URLCursor;

    BOOL OnInitDialog();
    void OnDestroy();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	DECLARE_MESSAGE_MAP()
};
