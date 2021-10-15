#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

//processing thread state
enum
{
    PTS_NONE = 0,   //processing thread not running
    PTS_RUNNING,    //process next item after processing thread teminates
    PTS_WAITNG_STOP //stop after processing thread teminates
};
extern int ProcessingState;

//dialog control helper
template<class T> class CDlgItem : public T
{
public:
    CDlgItem(HWND dialog_wnd, int control_id) : T() {Attach(::GetDlgItem(dialog_wnd, control_id));}
    ~CDlgItem() {Detach();}
};

class CMainApp : public CWinAppEx
{
    DECLARE_MESSAGE_MAP()
public:
	CMainApp();

	virtual BOOL InitInstance();
    int ExitInstance();
	virtual void PreLoadState();

    //common font for controls
    CFont* GetAppFont() {return &AppFont;}

private:
    CFont AppFont;
};

extern CMainApp theApp;
