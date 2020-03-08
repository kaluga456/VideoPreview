#pragma once

class CMainFrame : public CFrameWndEx
{
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual ~CMainFrame();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	CMFCMenuBar MainMenu;
	CMFCToolBar ToolBar;
	CMFCStatusBar StatusBar;
	CProfilePane ProfilePane;

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnDestroy();
	afx_msg LRESULT OnToolbarCreateNew(WPARAM wp, LPARAM lp);
	afx_msg void OnViewPropertiesWindow();
    afx_msg void OnAddFiles();
    afx_msg void OnAddFolder();
    afx_msg void OnStartProcessing();
    afx_msg void OnStopProcessing();
    afx_msg void OnRemoveCompleted();  
    afx_msg void OnRemopeAll();
	afx_msg void OnOptions();
    afx_msg void OnEditTest(); //TEST:

    afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()

	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

    BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);

};


