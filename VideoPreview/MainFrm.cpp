#include "stdafx.h"
#pragma hdrstop
#include "app_error.h"
#include "app_thread.h"
#include "Resource.h"
#include "About.h"
#include "Options.h"
#include "SourceFileTypes.h"
#include "OutputProfile.h"
#include "OutputProfileList.h"
#include "ProcessingItem.h"
#include "ScreenshotGenerator.h"
#include "ProcessingThread.h"
#include "VideoPreview.h"
#include "DialogAbout.h"
#include "DialogSettings.h"
#include "DialogOutputProfile.h"
#include "ProfilePane.h"
#include "VideoPreviewDoc.h"
#include "FileListView.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CProcessingItemList ProcessingItemList;
//////////////////////////////////////////////////////////////////////////////
//CComboOutputDirs
IMPLEMENT_SERIAL(CComboOutputDirs, CObject, VERSIONABLE_SCHEMA | 1)
CComboOutputDirs::CComboOutputDirs() : CMFCToolBarComboBoxButton()
{
}
CComboOutputDirs::CComboOutputDirs(UINT uiID) : CMFCToolBarComboBoxButton(uiID, 0, CBS_DROPDOWNLIST, 0), SelectedIndex(0)
{
}
bool CComboOutputDirs::HasDir(LPCTSTR dir) const
{
    for(auto i : Dirs)
    {
        if(0 == i.CompareNoCase(dir))
            return true;
    }
    return false;
}
void CComboOutputDirs::AddDir(CString new_dir)
{
    new_dir.Trim();
    if(new_dir.IsEmpty()) return;
    if(HasDir(new_dir)) 
    {
        SetCurrent(new_dir);
        return;
    }

    Dirs.push_front(new_dir);
    if(Dirs.size() > MAX_DIRS_COUNT)
        Dirs.pop_back();

    Update(new_dir);
}
void CComboOutputDirs::Update(LPCTSTR selected_dir /*= NULL*/)
{
    RemoveAllItems();
    AddItem(_T("<Use video file directory for output>"), NULL);

    CString sel_dir(selected_dir);
    for(auto i : Dirs)
    {
        const int index = AddItem(i);
        if(index >= MAX_DIRS_COUNT)
            break;
    }
    SetCurrent(selected_dir);
}
void CComboOutputDirs::InitialUpdate()
{
    Update();
    if(FALSE == SelectItem(SelectedIndex))
        SelectItem(0);
}
LPCTSTR CComboOutputDirs::GetCurrent()
{
    return (0 == GetCurSel()) ? NULL : GetItem();
}
void CComboOutputDirs::SetCurrent(LPCTSTR selected_dir)
{
    if(selected_dir && FALSE == SelectItem(selected_dir))
        SelectItem(0);
}
void CComboOutputDirs::Serialize(CArchive& archive)
{
    CObject::Serialize(archive);
    if(archive.IsStoring())
    {
        const int sel_index = GetCurSel();
        const int count = Dirs.size();      
        archive << sel_index;
        archive << count;
        for(auto i : Dirs)
            archive << i;
    }
    else
    {
        int count = 0;
        archive >> SelectedIndex;
        archive >> count;
        if(count > MAX_DIRS_COUNT)
            count = MAX_DIRS_COUNT;

        CString dir;
        Dirs.clear();
        for(int i = 0; i < count; ++i)
        {
            archive >> dir;
            Dirs.push_back(dir);
        }
    }
}
//////////////////////////////////////////////////////////////////////////////
//CMainToolbar
void CMainToolbar::AdjustLayout()
{
    static int combo_button_index = -1;
    if(combo_button_index < 0) combo_button_index = CommandToIndex(ID_COMBO_OUTPUT_DIR);
    CMFCToolBarComboBoxButton* cbb = static_cast<CMFCToolBarComboBoxButton*>(GetButton(combo_button_index));
    ASSERT(cbb);
    if(NULL == cbb) return;

    //WORKAROUND: this call restores default width of CMFCToolBarComboBoxButton
    //CMFCToolBar::AdjustLayout();

    CRect rectClient;
	GetClientRect(rectClient);  
    CRect cb_rect(cbb->Rect());
    cb_rect.right = rectClient.right;
    cbb->SetRect(cb_rect);

    Invalidate();
}
//////////////////////////////////////////////////////////////////////////////
//items list state CMainFrame::ItemsListState
class CItemsListState
{
public:
    CItemsListState() : State(NULL) {}
    void Update();

    int HasReady() const {return State & PILS_HAS_READY;}
    int HasDone() const {return State & PILS_HAS_DONE;}
    int HasFailed() const {return State & PILS_HAS_FAILED;}

    void SetReady(bool value) {SetBit(PILS_HAS_READY, value);}
    void SetDone(bool value) {SetBit(PILS_HAS_DONE, value);}
    void SetFailed(bool value) {SetBit(PILS_HAS_FAILED, value);}

private:
    enum
    {
        PILS_HAS_READY = 0x00000001,
        PILS_HAS_DONE = 0x00000002,
        PILS_HAS_FAILED = 0x00000004
    };

    int State;
    void SetBit(int bit, bool value = true);
} ItemsListState;

void CItemsListState::SetBit(int bit, bool value /*= true*/)
{
    if(value) State |= bit;
    else State &= ~bit;
}
void CItemsListState::Update()
{
    State = 0;
    for(CProcessingItemList::iterator i = ProcessingItemList.begin(); i != ProcessingItemList.end(); ++i)
    {
        PProcessingItem pi = i->second;
        if(PIS_WAIT == pi->State) SetBit(PILS_HAS_READY);
        if(PIS_DONE == pi->State) SetBit(PILS_HAS_DONE);
        if(PIS_FAILED == pi->State) SetBit(PILS_HAS_FAILED);
    }
}
//////////////////////////////////////////////////////////////////////////////
static CString GetLParamString(LPARAM value)
{
    if(NULL == value) return _T("");
    CString result(reinterpret_cast<LPCTSTR>(value));
    ::free(reinterpret_cast<void*>(value));
    return result;
}

static void ShellSelectFile(LPCTSTR file_name)
{
    ASSERT(file_name);
    if(NULL == file_name) return;

    SHELLEXECUTEINFO shei;
    ::ZeroMemory(&shei, sizeof(shei));
    shei.cbSize = sizeof(shei);
    shei.fMask = SEE_MASK_UNICODE | SEE_MASK_FLAG_NO_UI;
    shei.lpVerb = _T("open");
    shei.lpFile = _T("explorer");
    CString params;
    params.Format(_T("/select,\"%s\""), file_name);
    shei.lpParameters = params;
    shei.nShow = SW_SHOWNORMAL;
    const BOOL result = ::ShellExecuteEx(&shei);
    if(TRUE == result) return;

    const DWORD error_code = ::GetLastError();
    TCHAR buffer[2048];
    app::winapi_error_string(error_code, buffer, 2048);
    CString message;
    message.Format(_T("Error opening file\r\n%s\r\n\r\n%s"), file_name, buffer);
    ::AfxMessageBox(message, MB_OK | MB_ICONSTOP);
}
static void ShellOpenFile(LPCTSTR file_name, HWND hwnd = NULL)
{
    ASSERT(file_name);
    if(NULL == file_name) return;

    SHELLEXECUTEINFO shei;
    ::ZeroMemory(&shei, sizeof(shei));
    shei.cbSize = sizeof(shei);
    shei.fMask = SEE_MASK_UNICODE | SEE_MASK_FLAG_NO_UI;
    shei.hwnd = hwnd;
    shei.lpVerb = _T("open");
    shei.lpFile = file_name;
    shei.nShow = SW_SHOWNORMAL;
    const BOOL result = ::ShellExecuteEx(&shei);
    if(TRUE == result) return;

    const DWORD error_code = ::GetLastError();
    TCHAR buffer[2048];
    app::winapi_error_string(error_code, buffer, 2048);
    CString message;
    message.Format(_T("Error opening file\r\n%s\r\n\r\n%s"), file_name, buffer);
    ::AfxMessageBox(message, MB_OK | MB_ICONSTOP);
}

//TODO: need status bar?
static UINT SBIndicators[] =
{
	ID_SEPARATOR           // status line indicator
};

//////////////////////////////////////////////////////////////////////////////
//CMainFrame
IMPLEMENT_DYNCREATE(CMainFrame, CFrameWndEx)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWndEx)
	ON_WM_CREATE()
    ON_WM_CLOSE()
    ON_WM_DESTROY()
    ON_WM_SIZE()
    ON_REGISTERED_MESSAGE(AFX_WM_RESETTOOLBAR, &CMainFrame::OnResetToolbar)
    ON_MESSAGE(WM_PROCESSING_THREAD, &CMainFrame::OnProcessingThread)

    //commands    
    ON_COMMAND(ID_CMD_OPTIONS, &CMainFrame::OnCmdSettings)
    ON_COMMAND(ID_CMD_GITHUB, &CMainFrame::OnCmdGitHub)
    ON_COMMAND(ID_CMD_ABOUT, &CMainFrame::OnCmdAbout)
    ON_COMMAND(ID_CMD_TEST, &CMainFrame::OnCmdTest) //TEST

    ON_COMMAND(ID_CMD_ADD_FILES, &CMainFrame::OnCmdAddFiles)
    ON_COMMAND(ID_CMD_ADD_FOLDER, &CMainFrame::OnCmdAddFolder)
    
    //output profiles
    ON_COMMAND(ID_CMD_PROFILE_ADD, &CMainFrame::OnCmdProfileAdd)
    ON_COMMAND(ID_CMD_PROFILE_SAVE, &CMainFrame::OnCmdProfileSave)
    ON_COMMAND(ID_CMD_PROFILE_DELETE, &CMainFrame::OnCmdProfileDelete)
    ON_COMMAND(ID_CMD_PROFILE_PREVIEW, &CMainFrame::OnProfilePreview)
    ON_CBN_SELCHANGE(ID_COMBO_OUTPUT_DIR, &CMainFrame::OnProfileCombo)

    //processing
    ON_COMMAND(ID_CMD_PROCESS_ALL, &CMainFrame::OnCmdProcessAll)
    ON_COMMAND(ID_CMD_PROCESS_SELECTED, &CMainFrame::OnCmdProcessSelected)
    ON_COMMAND(ID_CMD_STOP_PROCESSING, &CMainFrame::OnCmdStopProcessing)   

    ON_COMMAND(ID_CMD_OPEN_VIDEO, &CMainFrame::OnCmdOpenVideo)
    ON_COMMAND(ID_CMD_OPEN_PREVIEW, &CMainFrame::OnCmdOpenPreview)
    ON_COMMAND(ID_CMD_BROWSE_TO_VIDEO, &CMainFrame::OnCmdBrowseToVideo)
    ON_COMMAND(ID_CMD_BROWSE_TO_PREVIEW, &CMainFrame::OnCmdBrowseToPreview)
    
    ON_COMMAND(ID_CMD_RESET_SELECTED, &CMainFrame::OnCmdResetSelected)
    ON_COMMAND(ID_CMD_RESET_ALL, &CMainFrame::OnCmdResetAll)
    
    ON_COMMAND(ID_CMD_REMOVE_FAILED, &CMainFrame::OnCmdRemoveFailed)
    ON_COMMAND(ID_CMD_REMOVE_COMPLETED, &CMainFrame::OnCmdRemoveCompleted)
    ON_COMMAND(ID_CMD_REMOVE_SELECTED, &CMainFrame::OnCmdRemoveSelected)
    ON_COMMAND(ID_CMD_REMOVE_ALL, &CMainFrame::OnCmdRemoveAll)

    ON_COMMAND(ID_CMD_SELECT_OUTPUT_DIR, &CMainFrame::OnCmdSelectOutputDir)

    //update UI
    ON_UPDATE_COMMAND_UI(ID_COMBO_OUTPUT_DIR, &CMainFrame::OnUpdateUI)
    //ON_UPDATE_COMMAND_UI(ID_CMD_PROFILE_ADD, OnUpdateUI)
    //ON_UPDATE_COMMAND_UI(ID_CMD_PROFILE_SAVE, OnUpdateUI)
    //ON_UPDATE_COMMAND_UI(ID_CMD_PROFILE_DELETE, OnUpdateUI)
    //ON_UPDATE_COMMAND_UI(ID_CMD_PROFILE_PREVIEW, OnUpdateUI)

    ON_UPDATE_COMMAND_UI(ID_CMD_OPEN_VIDEO, &CMainFrame::OnUpdateUI)
    ON_UPDATE_COMMAND_UI(ID_CMD_OPEN_PREVIEW, &CMainFrame::OnUpdateUI)
    ON_UPDATE_COMMAND_UI(ID_CMD_BROWSE_TO_VIDEO, &CMainFrame::OnUpdateUI)
    ON_UPDATE_COMMAND_UI(ID_CMD_BROWSE_TO_PREVIEW, &CMainFrame::OnUpdateUI)

    ON_UPDATE_COMMAND_UI(ID_CMD_PROCESS_ALL, &CMainFrame::OnUpdateUI)
    ON_UPDATE_COMMAND_UI(ID_CMD_PROCESS_SELECTED, &CMainFrame::OnUpdateUI)
    ON_UPDATE_COMMAND_UI(ID_CMD_STOP_PROCESSING, &CMainFrame::OnUpdateUI)

    ON_UPDATE_COMMAND_UI(ID_CMD_RESET_SELECTED, &CMainFrame::OnUpdateUI)
    ON_UPDATE_COMMAND_UI(ID_CMD_RESET_ALL, &CMainFrame::OnUpdateUI)

    ON_UPDATE_COMMAND_UI(ID_CMD_REMOVE_FAILED, &CMainFrame::OnUpdateUI)
    ON_UPDATE_COMMAND_UI(ID_CMD_REMOVE_COMPLETED, &CMainFrame::OnUpdateUI)
    ON_UPDATE_COMMAND_UI(ID_CMD_REMOVE_SELECTED, &CMainFrame::OnUpdateUI)
    ON_UPDATE_COMMAND_UI(ID_CMD_REMOVE_ALL, &CMainFrame::OnUpdateUI)

    ON_UPDATE_COMMAND_UI(ID_CMD_SELECT_OUTPUT_DIR, OnUpdateUI)
END_MESSAGE_MAP()

CMainFrame::CMainFrame()
{
    ::IsProcessing = false;
}
CMainFrame::~CMainFrame()
{
}
int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if(CFrameWndEx::OnCreate(lpCreateStruct) == -1)
		return -1;

	BOOL bNameValid;

	if(!MainMenu.Create(this))
	{
		TRACE0("Failed to create menubar\n");
		return -1;      // fail to create
	}

    DWORD menu_bar_sytle = MainMenu.GetPaneStyle();
    menu_bar_sytle &= ~(CBRS_SIZE_DYNAMIC | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_FLOATING | CBRS_FLOAT_MULTI | CBRS_GRIPPER);
    MainMenu.SetPaneStyle(menu_bar_sytle);
    MainMenu.SetRecentlyUsedMenus(FALSE);
    MainMenu.SetShowAllCommands(TRUE);

    //TODO: need that?
	//MainMenu.SetPaneStyle(MainMenu.GetPaneStyle() /*| CBRS_SIZE_DYNAMIC | CBRS_TOOLTIPS | CBRS_FLYBY*/);

	// prevent the menu bar from taking the focus on activation
	CMFCPopupMenu::SetForceMenuFocus(FALSE);

	if(!ToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) || !ToolBar.LoadToolBar(ID_TOOLBAR_MAIN))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	CString strToolBarName;
	bNameValid = strToolBarName.LoadString(IDS_TOOLBAR_STANDARD);
	ASSERT(bNameValid);
	ToolBar.SetWindowText(strToolBarName);  

    //DEPRECATE:
	CString strCustomize;
	bNameValid = strCustomize.LoadString(IDS_TOOLBAR_CUSTOMIZE);
	ASSERT(bNameValid);

    //TODO: need status bar?
	if(!StatusBar.Create(this))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}
	StatusBar.SetIndicators(SBIndicators, sizeof(SBIndicators)/sizeof(UINT));

	//create profile window
	CString strPropertiesWnd;
	bNameValid = strPropertiesWnd.LoadString(IDS_PROPERTIES_WND);
	ASSERT(bNameValid);
    if(!SettingsPane.Create(strPropertiesWnd, this, CRect(0, 0, Options.ProfilePaneWidth, 200), FALSE, ID_SETTINGS_PANE, 
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |  CBRS_LEFT, AFX_CBRS_REGULAR_TABS, AFX_CBRS_RESIZE)) 
	{
		TRACE0("Failed to create SettingsPane window\n");
		return FALSE; // failed to create
	}

    CDockState state;
    state.Clear();
    SetDockState(state);
    GetDockingManager()->DisableRestoreDockState(TRUE);
    GetDockingManager()->EnableDocking(CBRS_NOALIGN);

    //NOTE: keep this order
    DockPane(&MainMenu);
    DockPane(&SettingsPane);
    DockPane(&ToolBar);

    TempProfile.SetDefault();
    COutputProfile* profile = OutputProfiles.GetSelectedProfile();
    if(NULL == profile) profile = OutputProfiles.SelectFirst();
    if(NULL == profile) profile = &TempProfile;
    SettingsPane.SetOutputProfile(profile);

    theApp.GetObject(_T("OutputDirs"), *CBOutputDir);
    CBOutputDir->InitialUpdate();

    ItemsListState.Update();
    SettingsPane.SetSettings();

    return 0;
}
void CMainFrame::OnClose()
{
    //BUG:
    //SettingsPane.PromtSaveCurrentProfile();

    CFrameWndEx::OnClose();
}
void CMainFrame::OnDestroy()
{
    ProcessingThread.Stop();

    CRect rect;
    SettingsPane.GetClientRect(&rect);
    Options.ProfilePaneWidth = rect.Width();

    theApp.WriteObject(_T("OutputDirs"), *CBOutputDir);

    CFrameWndEx::OnDestroy();
}
void CMainFrame::OnSize(UINT nType, int cx, int cy)
{
    //TODO:
    CFrameWndEx::OnSize(nType, cx, cy);
}
void CMainFrame::AdjustDockingLayout(HDWP hdwp /*= NULL*/)
{
    CFrameWndEx::AdjustDockingLayout(hdwp);
    if(ToolBar.GetSafeHwnd()) ToolBar.AdjustLayout();
}
void CMainFrame::OnUpdateFrameTitle(BOOL bAddToTitle)
{
    //NOTE: in default method frame title is first name from IDR_MAINFRAME
    //CFrameWndEx::OnUpdateFrameTitle(bAddToTitle);
    ::AfxSetWindowText(m_hWnd, APP_NAME _T(" (build: ") APP_BUILD _T(")"));
}
BOOL CMainFrame::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
    LPNMHDR nmhdr = reinterpret_cast<LPNMHDR>(lParam);

    //file list view events
    CView* flv = GetActiveView(); 
    if(flv && nmhdr->hwndFrom == flv->m_hWnd)
    {
        if(NM_DBLCLK == nmhdr->code)
            OnCmdOpenVideo();
        else if(LVN_COLUMNCLICK == nmhdr->code) 
        {
            LPNMLISTVIEW nmlv = reinterpret_cast<LPNMLISTVIEW>(lParam);
            GetFileListView()->OnColumnClick(nmlv->iSubItem);
        }
    }

    return CFrameWndEx::OnNotify(wParam, lParam, pResult);
}
BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if(!CFrameWndEx::PreCreateWindow(cs))
		return FALSE;
    cs.style &= ~FWS_ADDTOTITLE;
	return TRUE;
}
LRESULT CMainFrame::OnResetToolbar(WPARAM wp,LPARAM lp)
{
    if(ID_TOOLBAR_MAIN == wp)
    {
        //CMFCToolBarComboBoxButton profile_combo(ID_COMBO_OUTPUT_DIR, 0, CBS_DROPDOWNLIST, 0);
        CComboOutputDirs profile_combo(ID_COMBO_OUTPUT_DIR);
        ToolBar.ReplaceButton(ID_COMBO_OUTPUT_DIR, profile_combo);
        ToolBar.AdjustLayout();

        const int out_dir_combo_index = ToolBar.CommandToIndex(ID_COMBO_OUTPUT_DIR);
        CBOutputDir = static_cast<CComboOutputDirs*>(ToolBar.GetButton(out_dir_combo_index));
        ASSERT(CBOutputDir);
    }
    else if(ID_TOOLBAR_SETTINGS == wp)
    {
        CMFCToolBarComboBoxButton profile_combo(ID_PROFILE_COMBO, 0, CBS_DROPDOWNLIST, 0);
        SettingsPane.ToolBar.ReplaceButton(ID_PROFILE_COMBO, profile_combo);
    }
    return 0;
}
CComboOutputDirs* CMainFrame::GetOutputDirCombo()
{
    static int profile_combo_index = -1;
    if(profile_combo_index < 0) profile_combo_index = ToolBar.CommandToIndex(ID_COMBO_OUTPUT_DIR);
    return static_cast<CComboOutputDirs*>(ToolBar.GetButton(profile_combo_index));
}
// CMainFrame diagnostics
#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWndEx::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWndEx::Dump(dc);
}
#endif //_DEBUG

void CMainFrame::OnViewPropertiesWindow()
{
	// Show or activate the pane, depending on current state.  The
	// pane can only be closed via the [x] button on the pane frame.
	//SettingsPane.ShowPane(TRUE, FALSE, TRUE);
	//SettingsPane.SetFocus();
}
void CMainFrame::OnCmdGitHub()
{
    ShellOpenFile(APP_URL, m_hWnd);
}
void CMainFrame::OnCmdAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}
void CMainFrame::OnCmdAddFiles()
{
    CString filter = SourceFileTypes.GetFilterString();
    CFileDialog fd(TRUE, NULL, NULL, OFN_ALLOWMULTISELECT | OFN_ENABLESIZING | OFN_EXPLORER, filter, this, sizeof(OPENFILENAME), TRUE);
    OPENFILENAME& ofn = fd.GetOFN();
    ofn.lpstrTitle = _T("Add Video Files");
    if(IDCANCEL == fd.DoModal())
        return;

    //root directory
    CString root_dir = ofn.lpstrFile;
    int name_size = root_dir.GetLength();
    if(0 == name_size)
        return;
    const int items_count = GetFileListView()->GetListCtrl().GetItemCount();

    //multiple files were selected, so first string is root dir
    if(ofn.nFileOffset > name_size)
    {
        root_dir += _T("\\");
        for(LPCTSTR file_names = ofn.lpstrFile + name_size + 1; ;file_names += name_size + 1)
        {
            CString file_name = file_names;
            name_size = file_name.GetLength();
            if(0 == name_size)
                break;

            PProcessingItem pi(new CProcessingItem(PIS_WAIT, root_dir + file_name));
            AddItem(pi);
        }
    }

    //one file selected
    else
    {
        PProcessingItem pi(new CProcessingItem(PIS_WAIT, ofn.lpstrFile));
        AddItem(pi);
    }

    //if has new items in list
    if(items_count < GetFileListView()->GetListCtrl().GetItemCount())
    {
        GetFileListView()->Sort();
        ItemsListState.SetReady(true);
        UpdateDialogControls(this, FALSE);
    }
}
void CMainFrame::OnCmdAddFolder()
{
    CFolderPickerDialog fpd(NULL, OFN_ALLOWMULTISELECT | OFN_ENABLESIZING | OFN_EXPLORER, this, sizeof(OPENFILENAME));
    OPENFILENAME& ofn = fpd.GetOFN();
    ofn.lpstrTitle = _T("Add Video Files");
    if(IDCANCEL == fpd.DoModal())
        return;

    //root directory
    CString root_dir = ofn.lpstrFile;
    int name_size = root_dir.GetLength();
    if(0 == name_size)
        return;

    //if only one directory selected
    if(0 == *(ofn.lpstrFile + name_size + 1))
    {
        AddFolder(root_dir);
        return;
    }

    root_dir += _T("\\");
    const int items_count = GetFileListView()->GetListCtrl().GetItemCount();
    for(LPCTSTR file_names = ofn.lpstrFile + name_size + 1; ;file_names += name_size + 1)
    {
        CString file_name = file_names;
        name_size = file_name.GetLength();
        if(0 == name_size)
            break;

        AddFolder(root_dir + file_name);
    }

    //if has new items in list
    if(items_count < GetFileListView()->GetListCtrl().GetItemCount())
    {
        GetFileListView()->Sort();
        ItemsListState.SetReady(true);
        UpdateDialogControls(this, FALSE);
    }
}
void CMainFrame::AddFolder(CString root_dir)
{
    CFileFind finder;
    BOOL result = finder.FindFile(root_dir + _T("\\*.*"));
    while(result)
    {
        result = finder.FindNextFile();
        if(finder.IsDots()) continue;

        //recursive search
        CString found_name = finder.GetFileName();
        if(finder.IsDirectory())
        {
            AddFolder(root_dir + _T("\\") + found_name);
            continue;
        }

        //check file extension
        LPCTSTR dot_pos = ::wcsrchr(found_name, _T('.'));
        if(NULL == dot_pos) continue;
        CString ext = dot_pos + 1;
        ext.MakeLower();
        if(SourceFileTypes.HasType(ext))
        {
            PProcessingItem pi(new CProcessingItem(PIS_WAIT, root_dir + _T("\\") + found_name));
            AddItem(pi);
        }
    }
}
void CMainFrame::OnCmdSelectOutputDir()
{
    CFolderPickerDialog fpd(NULL, OFN_ENABLESIZING | OFN_EXPLORER, this, sizeof(OPENFILENAME));
    OPENFILENAME& ofn = fpd.GetOFN();
    ofn.lpstrTitle = _T("Select Ouput Directory");
    if(IDCANCEL == fpd.DoModal())
        return;

    CString dir(ofn.lpstrFile);
    CBOutputDir->AddDir(dir);
}
LRESULT CMainFrame::OnProcessingThread(WPARAM wp, LPARAM lp)
{
    ASSERT(CurrentItem.get());

    CFileListView* file_list_view = GetFileListView();
    const int message_type = wp;
    switch(message_type)
    {
    case PTM_PROGRESS:   //LPARAM - progress (0..100)
        ASSERT(0 <= lp && lp <= 100);
        CurrentItem->State = PIS_MIN_PROCESSING + lp;
        file_list_view->UpdateItem(CurrentItem.get());
        return 0;
    case PTM_DONE:       //LPARAM - result text, including output file name (LPTSTR)
        CurrentItem->State = PIS_DONE;
        CurrentItem->ResultString = GetLParamString(lp);
        file_list_view->UpdateItem(CurrentItem.get());
        ItemsListState.SetDone(true);
        CurrentItem.reset();
        ProcessNextItem();
        break;
    case PTM_STOP:       //LPARAM - NULL
        CurrentItem->State = PIS_WAIT;
        CurrentItem->ResultString = _T("");
        file_list_view->UpdateItem(CurrentItem.get());
        ItemsListState.SetReady(true);
        CurrentItem.reset();        
        break;
    case PTM_FAILED:      //LPARAM - error description (LPTSTR)
        CurrentItem->State = PIS_FAILED;
        CurrentItem->ResultString = GetLParamString(lp);
        file_list_view->UpdateItem(CurrentItem.get());
        ItemsListState.SetFailed(true);
        if(IsProceedOnError())
        {
            CurrentItem.reset();
            ProcessNextItem();
        }
        break;
    default:
        ASSERT(FALSE);
        CurrentItem.reset();
        IsProcessing = false;
    }

    UpdateDialogControls(this, FALSE);
    return 0;
}
void CMainFrame::SetProcessingState(bool Value)
{
    if(Value == IsProcessing) return;
    if(Value)
    {       
        IsProcessing = true;
    }
    else
    {
        //TODO: get actual state from thread message
        ProcessingThread.Stop();
        IsProcessing = false;
    }
    UpdateDialogControls(this, FALSE);
}
bool CMainFrame::IsProceedOnError()
{
    if(COptions::ACTION_ON_ERROR_SKIP == Options.ActionOnError)
        return true;
    if(COptions::ACTION_ON_ERROR_PROMT == Options.ActionOnError)
        return true; //TODO: promt
    return false;
}
bool CMainFrame::ProcessNextItem()
{
    for(;;)
    {
        ASSERT(NULL == CurrentItem.get());
        if(CurrentItem.get()) 
            break;

        PProcessingItem pi = GetFileListView()->GetUnprocessedItem();
        if(NULL == pi) 
            break; //no more items to process

        SettingsPane.GetOutputProfile(&TempProfile);
        const DWORD result = ProcessingThread.Start(m_hWnd, &TempProfile, pi->SourceFileName);
        if(ERROR_SUCCESS == result)
        {
            CurrentItem = pi;
            SetProcessingState(true);
            return true;
        }
        else
        {
            //TODO: message
            //::AfxMessageBox(_T(""), MB_ICONWARNING | MB_OK
        }

        if(false == IsProceedOnError())
            break;
    }

    CurrentItem.reset();
    SetProcessingState(false);
    return false;
}
void CMainFrame::OnCmdProcessSelected()
{
    //TODO:
}
void CMainFrame::OnCmdProcessAll()
{
    ProcessNextItem();
}
void CMainFrame::OnCmdStopProcessing()
{
    //TODO: confirm
    SetProcessingState(false);
}
void CMainFrame::OnCmdRemoveFailed()
{
    const int result = ::AfxMessageBox(_T("Remove failed files from list?"), MB_OKCANCEL | MB_ICONEXCLAMATION | MB_DEFBUTTON1);
    if(result != IDOK) return;

    for(CProcessingItemList::iterator i = ProcessingItemList.begin(); i != ProcessingItemList.end();)
    {
        PProcessingItem pi = i->second;
        if(PIS_FAILED == pi->State) 
            i = ProcessingItemList.erase(i);
        else
            ++i;     
    }
    CFileListView* flv = GetFileListView();
    flv->UpdateItems();
    ItemsListState.SetFailed(false);
    UpdateDialogControls(this, FALSE);
}
void CMainFrame::OnCmdRemoveCompleted()
{
    const int result = ::AfxMessageBox(_T("Remove completed files from list?"), MB_OKCANCEL | MB_ICONEXCLAMATION | MB_DEFBUTTON1);
    if(result != IDOK) return;

    for(CProcessingItemList::iterator i = ProcessingItemList.begin(); i != ProcessingItemList.end();)
    {
        PProcessingItem pi = i->second;
        if(PIS_DONE == pi->State) 
            i = ProcessingItemList.erase(i);
        else
            ++i;     
    }
    CFileListView* flv = GetFileListView();
    flv->UpdateItems();
    ItemsListState.SetDone(false);
    UpdateDialogControls(this, FALSE);
}
void CMainFrame::RemoveAllItems()
{
    //remove all items...
    CFileListView* flv = GetFileListView();
    flv->GetListCtrl().DeleteAllItems();
    ProcessingItemList.clear();

    //...excluding current item
    if(CurrentItem.get())
    {
        ProcessingItemList[CurrentItem.get()] = CurrentItem;
        flv->UpdateItem(CurrentItem.get());
    }

    ItemsListState.Update();
    UpdateDialogControls(this, FALSE);
}
void CMainFrame::OnCmdRemoveAll()
{
    if(IDOK != ::AfxMessageBox(_T("Remove all files from list?"), MB_OKCANCEL | MB_ICONEXCLAMATION | MB_DEFBUTTON1)) return;
    RemoveAllItems();
}
void CMainFrame::OnCmdSettings()
{
    CDialogSettings dialog(this);
    dialog.DoModal();
}
void CMainFrame::OnProfileCombo()
{
    //COutputProfile* old_profile = OutputProfiles.GetSelectedProfile();
    //COutputProfile* new_profile = GetCurrentOutputDir();
    //if(new_profile == old_profile) return;

    //SettingsPane.PromtSaveCurrentProfile();

    //OutputProfiles.SetSelectedProfile(new_profile);
    //SettingsPane.SetOutputProfile(new_profile);
    //SettingsPane.ResetProfileChanged();
}
void CMainFrame::OnCmdProfileAdd()
{
    SettingsPane.PromtSaveCurrentProfile();

    CDialogOutputProfile dialog(this, true);
    if(IDOK != dialog.DoModal())
        return;

    POutputProfile new_profile(new COutputProfile);
    if(dialog.IsCopyFrom && false == dialog.CopyFrom.IsEmpty())
    {
        const COutputProfile* op = OutputProfiles.GetProfile(dialog.CopyFrom);  
        if(NULL == op)
        {
            ASSERT(FALSE);
            new_profile->SetDefault();
        }
        else
        {
            *new_profile = *op;
        }
    }
    else
    {
        new_profile->SetDefault();
    }

    OutputProfiles.AddProfile(theApp, dialog.ProfileName, new_profile);
    SettingsPane.SetOutputProfile(GetCurrentProfile());
    SettingsPane.UpdateProfileCombo();
}
void CMainFrame::OnCmdProfileSave()
{
    CDialogOutputProfile dialog(this, false);
    if(IDOK != dialog.DoModal())
        return;

    COutputProfile* profile_to_save = OutputProfiles.GetProfile(dialog.ProfileName);
    if(profile_to_save)
    {
        SettingsPane.GetOutputProfile(profile_to_save);
        OutputProfiles.WriteProfile(theApp, dialog.ProfileName);
    }
    else
    {
        POutputProfile new_profile(new COutputProfile);
        SettingsPane.GetOutputProfile(new_profile.get());
        OutputProfiles.AddProfile(theApp, dialog.ProfileName, new_profile);
    }

    SettingsPane.SetOutputProfile(GetCurrentProfile());
    SettingsPane.UpdateProfileCombo();
}
void CMainFrame::OnCmdProfileDelete()
{
    OutputProfiles.DeleteSelectedProfile(theApp);
    SettingsPane.SetOutputProfile(GetCurrentProfile());
    SettingsPane.UpdateProfileCombo();
}
void CMainFrame::OnProfilePreview()
{
    //TODO:
    int i = 0;
}
void CMainFrame::OnCmdTest()
{
    //TEST:
    //CBOutputDir->AddDir(_T("test"));
    //CMFCToolBarButton* cmb = MainMenu.GetMenuItem(6);
    //cmb->SetImage(2);
}
void CMainFrame::OnUpdateUI(CCmdUI* pCmdUI)
{
    switch(pCmdUI->m_nID)
    {
    case ID_CMD_SELECT_OUTPUT_DIR:
    case ID_COMBO_OUTPUT_DIR:
    case ID_CMD_OPTIONS:
    //case ID_CMD_PROFILE_ADD: 
        pCmdUI->Enable(false == IsProcessing);
        break;
    //case ID_CMD_PROFILE_PREVIEW:
    //case ID_CMD_PROFILE_SAVE:
    //case ID_CMD_PROFILE_DELETE:
    //    pCmdUI->Enable(false == IsProcessing && false == OutputProfiles.IsEmpty() && OutputProfiles.GetSelectedProfile());
    //    break;

    //src and out files
    case ID_CMD_OPEN_VIDEO:
    case ID_CMD_BROWSE_TO_VIDEO:
    {
        pCmdUI->Enable(GetFileListView()->GetFocusedItem() != NULL);
        break;
    }
    case ID_CMD_OPEN_PREVIEW:
    case ID_CMD_BROWSE_TO_PREVIEW:
    {
        CProcessingItem* pi = GetFileListView()->GetFocusedItem();
        pCmdUI->Enable(pi != NULL && PIS_DONE == pi->State);
        break;
    }
    
    //processing
    case ID_CMD_PROCESS_ALL:     
        pCmdUI->Enable(ItemsListState.HasReady() && false == IsProcessing);
        break;
    case ID_CMD_PROCESS_SELECTED:     
        pCmdUI->Enable(ItemsListState.HasReady() && false == IsProcessing && GetFileListView()->GetListCtrl().GetSelectedCount());
        break;
    case ID_CMD_STOP_PROCESSING:
        pCmdUI->Enable(IsProcessing);
        break;

    //reset items
    case ID_CMD_RESET_SELECTED:
        pCmdUI->Enable((ItemsListState.HasDone() || ItemsListState.HasFailed()) && GetFileListView()->GetListCtrl().GetSelectedCount());
        break;

    //remove
    case ID_CMD_REMOVE_COMPLETED:
        pCmdUI->Enable(ItemsListState.HasDone());
        break;
    case ID_CMD_REMOVE_FAILED:
        pCmdUI->Enable(ItemsListState.HasFailed());
        break;
    case ID_CMD_REMOVE_SELECTED:
        pCmdUI->Enable(GetFileListView()->GetListCtrl().GetSelectedCount());
        break;
    case ID_CMD_REMOVE_ALL:
        pCmdUI->Enable(GetFileListView()->GetListCtrl().GetItemCount());
        break;
    }
}
LPCTSTR CMainFrame::GetCurrentOutputDir()
{
    return CBOutputDir->GetItem();
}
COutputProfile* CMainFrame::GetCurrentProfile()
{
    COutputProfile* profile = OutputProfiles.GetSelectedProfile();
    return profile ? profile : &TempProfile;
}
CFileListView* CMainFrame::GetFileListView()
{
    CFileListView* result = static_cast<CFileListView*>(GetActiveView());
    ASSERT(result);
    return result;
}
void CMainFrame::AddItem(PProcessingItem item)
{
    //ignore duplicates
    for(CProcessingItemList::iterator i = ProcessingItemList.begin(); i != ProcessingItemList.end(); ++i)
    {
        PProcessingItem pi = i->second;
        if(0 == pi->SourceFileName.CompareNoCase(item->SourceFileName))
            return;
    }
    ProcessingItemList[item.get()] = item;
    GetFileListView()->UpdateItem(item.get());
}
void CMainFrame::RemoveItem(PProcessingItem item)
{
    //TODO: confirm
    GetFileListView()->RemoveItem(item.get());
}
void CMainFrame::OnCmdOpenVideo()
{
    CProcessingItem* pi = GetFileListView()->GetFocusedItem();
    if(NULL == pi) return;
    ShellOpenFile(pi->SourceFileName, m_hWnd);
}
void CMainFrame::OnCmdOpenPreview()
{
    CProcessingItem* pi = GetFileListView()->GetFocusedItem();
    if(NULL == pi) return;
    ASSERT(PIS_DONE == pi->State);
    ShellOpenFile(pi->ResultString, m_hWnd);
}
void CMainFrame::OnCmdBrowseToVideo()
{
    CProcessingItem* pi = GetFileListView()->GetFocusedItem();
    if(NULL == pi) return;
    ShellSelectFile(pi->SourceFileName);
}
void CMainFrame::OnCmdBrowseToPreview()
{
    CProcessingItem* pi = GetFileListView()->GetFocusedItem();
    if(NULL == pi) return;
    ASSERT(PIS_DONE == pi->State);
    ShellSelectFile(pi->ResultString);
}
void CMainFrame::OnCmdResetSelected()
{
    CFileListView* flv = GetFileListView();
    CListCtrl& lc = flv->GetListCtrl();
    const int sel_count = lc.GetSelectedCount();
    if(0 == sel_count)
        return;

    const int result = ::AfxMessageBox(_T("Reset selected files?"), MB_OKCANCEL | MB_ICONEXCLAMATION | MB_DEFBUTTON1);
    if(result != IDOK) 
        return;

    for(int found_index = -1;;)
    {
        found_index = lc.GetNextItem(found_index, LVNI_SELECTED);
        if(found_index < 0)
            break;

        CProcessingItem* pi = flv->FindItem(found_index);
        if(pi == CurrentItem.get()) continue;

        pi->Reset(false);
    }

    ItemsListState.Update();
    flv->UpdateItems();
    UpdateDialogControls(this, FALSE);
}
void CMainFrame::OnCmdResetAll()
{
    CFileListView* flv = GetFileListView();
    CListCtrl& lc = flv->GetListCtrl();
    const int sel_count = lc.GetItemCount();
    if(0 == sel_count)
        return;

    const int result = ::AfxMessageBox(_T("Reset all files?"), MB_OKCANCEL | MB_ICONEXCLAMATION | MB_DEFBUTTON1);
    if(result != IDOK) 
        return;

    for(int found_index = -1;;)
    {
        found_index = lc.GetNextItem(found_index, LVNI_ALL);
        if(found_index < 0)
            break;

        CProcessingItem* pi = flv->FindItem(found_index);
        if(pi == CurrentItem.get()) continue;

        pi->Reset(false);
    }

    ItemsListState.Update();
    flv->UpdateItems();
    UpdateDialogControls(this, FALSE);
}
void CMainFrame::OnCmdRemoveSelected()
{
    CFileListView* flv = GetFileListView();
    CListCtrl& lc = flv->GetListCtrl();
    const int sel_count = lc.GetSelectedCount();
    if(0 == sel_count)
        return;

    const int result = ::AfxMessageBox(_T("Remove selected files from list?"), MB_OKCANCEL | MB_ICONEXCLAMATION | MB_DEFBUTTON1);
    if(result != IDOK) 
        return;

    for(int found_index = -1;;)
    {
        found_index = lc.GetNextItem(found_index, LVNI_SELECTED);
        if(found_index < 0)
            break;

        CProcessingItem* pi = flv->FindItem(found_index);
        if(pi == CurrentItem.get()) continue;

        CProcessingItemList::iterator i = ProcessingItemList.find(pi);
        if(i != ProcessingItemList.end())
            ProcessingItemList.erase(i);
    }

    ItemsListState.Update();
    flv->UpdateItems();
    UpdateDialogControls(this, FALSE);
}
//////////////////////////////////////////////////////////////////////////////
