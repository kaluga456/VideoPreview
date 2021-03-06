#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

class CMainApp : public CWinAppEx
{
public:
	CMainApp();

	virtual BOOL InitInstance();
    int ExitInstance();
	virtual void PreLoadState();
	virtual void LoadCustomState();
	virtual void SaveCustomState();

    //profiles
    void ReadProfiles();
    void WriteProfile(LPCTSTR profile_name);
    void DeleteProfile(LPCTSTR profile_name);

	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CMainApp theApp;
