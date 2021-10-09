#pragma once

//CMainToolbar - disabled default CMFCToolBar::AdjustLayout()
class CMainToolbar : public CMFCToolBar
{
public:
    virtual void AdjustLayout();
};

class CMainFrame : public CFrameWndEx
{
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    virtual void OnUpdateFrameTitle(BOOL bAddToTitle);
	virtual ~CMainFrame();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

private:
    //processing
    PProcessingItem CurrentItem; //current item in processing thread
    CProcessingThread ProcessingThread;
    bool ProcessNextItem();
    bool IsProceedOnError();
    void SetProcessingState(bool Value); //true - processing items now

    //output profiles
    COutputProfile* GetCurrentProfile(); //selected profile on toolbar or TempProfile

    //file list
    CFileListView* GetFileListView();
    void AddItem(PProcessingItem item);
    void AddFolder(CString root_dir);
    void RemoveItem(PProcessingItem item);
    void RemoveAllItems();

    //settings
    LPCTSTR GetCurrentOutputDir();

protected:
    COutputProfile TempProfile;

    //controls
	CMFCMenuBar MainMenu;

    //TEST:
    CMainToolbar ToolBar;
	//CMFCToolBar ToolBar;

	CProfilePane SettingsPane;
    CMFCStatusBar StatusBar; //TODO: need status bar?

    //output dir combobox
    CMFCToolBarComboBoxButton* CBOutputDir;
    CMFCToolBarComboBoxButton* GetOutputDirCombo();
    void UpdateOutputDirCombo();

    //overrides
    virtual void AdjustDockingLayout(HDWP hdwp /*= NULL*/);

    //message handlers
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnDestroy();
    afx_msg void OnClose();
	afx_msg void OnViewPropertiesWindow();
    afx_msg void OnCmdAddFiles();
    afx_msg void OnCmdAddFolder();
    afx_msg void OnCmdProcessAll();
    afx_msg void OnCmdStopProcessing();
    afx_msg void OnCmdRemoveFailed();
    afx_msg void OnCmdRemoveCompleted();  
    afx_msg void OnCmdRemoveAll();
	afx_msg void OnCmdSettings();
    afx_msg void OnCmdProfileAdd();
    afx_msg void OnCmdProfileSave();
    afx_msg void OnCmdProfileDelete();
    afx_msg void OnProfilePreview();
    afx_msg void OnProfileCombo();
    afx_msg void OnCmdTest(); //TEST:
    afx_msg void OnCmdGitHub();
    afx_msg void OnCmdAbout();
    afx_msg void OnCmdOpenVideo();
    afx_msg void OnCmdOpenPreview();
    afx_msg void OnCmdBrowseToVideo();
    afx_msg void OnCmdBrowseToPreview();
    afx_msg void OnCmdProcessSelected();
    afx_msg void OnCmdResetSelected();
    afx_msg void OnCmdResetAll();
    afx_msg void OnCmdRemoveSelected();
    afx_msg void OnUpdateUI(CCmdUI* pCmdUI);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
    afx_msg LRESULT OnResetToolbar(WPARAM wp,LPARAM lp);
    afx_msg LRESULT OnProcessingThread(WPARAM wp,LPARAM lp);
	DECLARE_MESSAGE_MAP()

	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)
};


