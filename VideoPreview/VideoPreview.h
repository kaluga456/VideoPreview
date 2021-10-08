#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

//dialog control helper
template<class T> class CDlgItem : public T
{
public:
    CDlgItem(HWND dialog_wnd, int control_id) : T() {Attach(::GetDlgItem(dialog_wnd, control_id));}
    ~CDlgItem() {Detach();}
};

class CMainApp : public CWinAppEx
{
public:
	CMainApp();

	virtual BOOL InitInstance();
    int ExitInstance();
	virtual void PreLoadState();

	DECLARE_MESSAGE_MAP()
};

extern CMainApp theApp;
