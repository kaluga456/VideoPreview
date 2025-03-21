#pragma once

//CMainToolbar - disabled default CMFCToolBar::AdjustLayout()
class CMainToolbar : public CMFCToolBar
{
public:
    virtual void AdjustLayout();
};

class CComboOutputDirs : public CMFCToolBarComboBoxButton
{
    DECLARE_SERIAL(CComboOutputDirs)
public:
    CComboOutputDirs();
    explicit CComboOutputDirs(UINT uiID);

    void AddDir(CString new_dir);
    void Update(LPCTSTR selected_dir = nullptr);
    void InitialUpdate();

    LPCTSTR GetCurrent();
    void SetCurrent(LPCTSTR selected_dir);

    virtual void Serialize(CArchive& archive);

private:
    static const size_t MAX_DIRS_COUNT = 5;
    int SelectedIndex;
    std::list<CString> Dirs; //queue
    bool HasDir(LPCTSTR dir) const;
};

class CMainFrame : public CFrameWndEx
{
public:
	virtual ~CMainFrame();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
    //output profiles
    COutputProfile TempProfile;
    COutputProfile* GetCurrentProfile(); //selected profile or TempProfile

    //controls
	CMFCMenuBar MainMenu;
    CMainToolbar ToolBar;
	CSettingsPane SettingsPane;
    CMFCStatusBar StatusBar;

    //output dir
    CComboOutputDirs* CBOutputDir{nullptr};
    CComboOutputDirs* GetOutputDirCombo();

    //overrides
    virtual void AdjustDockingLayout(HDWP hdwp /*= nullptr*/);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    virtual void OnUpdateFrameTitle(BOOL bAddToTitle);

    //message handlers
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnDestroy();
    afx_msg void OnClose();
    afx_msg void OnCmdAddFiles();
    afx_msg void OnCmdAddFolder();
    afx_msg void OnCmdPasteFiles();
    afx_msg void OnCmdProcessAll();
    afx_msg void OnCmdStopProcessing();
    afx_msg void OnCmdRemoveFailed();
    afx_msg void OnCmdRemoveCompleted();  
    afx_msg void OnCmdRemoveAll();
	afx_msg void OnCmdSettings();
    afx_msg void OnCmdProfileAdd();
    afx_msg void OnCmdProfileSave();
    afx_msg void OnCmdProfileDelete();
    afx_msg void OnCmdProfilePreview();
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
    afx_msg void OnCmdSelectOutputDir();
    afx_msg void OnUpdateUI(CCmdUI* pCmdUI);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
    afx_msg LRESULT OnResetToolbar(WPARAM wp,LPARAM lp);
    afx_msg LRESULT OnProcessingThread(WPARAM wp,LPARAM lp);
	DECLARE_MESSAGE_MAP()

	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

private:
    //processing
    PProcessingItem CurrentItem; //current item in processing thread
    CProcessingThread ProcessingThread;
    void ProcessNextItem();

    //file list
    CFileListView* GetFileListView();
    void AddFolder(CString root_dir);
 
    //settings
    LPCTSTR GetCurrentOutputDir();
};


