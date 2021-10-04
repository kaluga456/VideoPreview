#pragma once

class CMainFrame : public CFrameWndEx
{
private:
    //processing
    bool IsProcessing; //true - while there are files to process and command 'stop' not executed
    PProcessingItem CurrentItem; //current item in processing thread
    CProcessingThread ProcessingThread;
    bool ProcessNextItem();
    bool IsProceedOnError();
    void SetProcessingState(bool Value); //true - processing items now

    //output profiles
    const COutputProfile* GetCurrentProfile();

    //file list
    CFileListView* GetFileListView();
    void AddItem(PProcessingItem item);
    void RemoveItem(PProcessingItem item);
    void RemoveAllItems();
    void AddFolder(CString root_dir);

protected:
	CMFCMenuBar MainMenu;
	CMFCToolBar ToolBar;
    CMFCToolBarComboBoxButton* CBProfile;
	CProfilePane ProfilePane;
    CMFCStatusBar StatusBar; //TODO: need status bar?

    //int CBProfileIndex;
    CMFCToolBarComboBoxButton* GetProfileCombo();

    //message handlers
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnDestroy();
	afx_msg void OnViewPropertiesWindow();
    afx_msg void OnAddFiles();
    afx_msg void OnAddFolder();
    afx_msg void OnStartProcessing();
    afx_msg void OnStopProcessing();
    afx_msg void OnRemoveFailed();
    afx_msg void OnRemoveCompleted();  
    afx_msg void OnRemoveAll();
	afx_msg void OnOptions();
    afx_msg void OnProfileSave();
    afx_msg void OnProfileDelete();
    afx_msg void OnProfilePreview();
    afx_msg void OnProfileCombo();
    afx_msg void OnEditTest(); //TEST:
    afx_msg void OnResetToolbar();
    afx_msg void OnAppAbout();
    afx_msg void OnContextOpenVideo();
    afx_msg void OnContextOpenPreview();
    afx_msg void OnContextProcessItem();
    afx_msg void OnContextResetItem();
    afx_msg void OnContextRemoveItem();
    afx_msg void OnUpdateUI(CCmdUI* pCmdUI);
    afx_msg void OnUpdateContextMenu(CCmdUI* pCmdUI);
    afx_msg LRESULT OnResetToolbar(WPARAM wp,LPARAM lp);
    afx_msg LRESULT OnProcessingThread(WPARAM wp,LPARAM lp);
	DECLARE_MESSAGE_MAP()

	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

    BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);

public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual ~CMainFrame();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
};


