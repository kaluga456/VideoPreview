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
    CMFCToolBarComboBoxButton* CBProfile;
	CProfilePane ProfilePane;
    CMFCStatusBar StatusBar; //TODO: need status bar?

    //int CBProfileIndex;
    CMFCToolBarComboBoxButton* GetProfileCombo();

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnDestroy();
	afx_msg void OnViewPropertiesWindow();
    afx_msg void OnAddFiles();
    afx_msg void OnAddFolder();
    afx_msg void OnStartProcessing();
    afx_msg void OnStopProcessing();
    afx_msg void OnRemoveCompleted();  
    afx_msg void OnRemoveAll();
	afx_msg void OnOptions();
    afx_msg void OnProfileSave();
    afx_msg void OnProfileDelete();
    afx_msg void OnProfilePreview();
    afx_msg void OnProfileCombo();
    afx_msg void OnEditTest(); //TEST:
    afx_msg void OnResetToolbar();
    afx_msg LRESULT OnResetToolbar(WPARAM wp,LPARAM lp);
    afx_msg void OnAppAbout();
    afx_msg void OnUpdateUI(CCmdUI* pCmdUI);
    afx_msg void OnUpdateProfileCombo(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

    BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
};


