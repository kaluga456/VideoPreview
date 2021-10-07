#include "stdafx.h"
#pragma hdrstop
#include "app_error.h"
#include "app_thread.h"
#include "Resource.h"
#include "About.h"
#include "Options.h"
#include "SourceFileTypes.h"
#include "OutputProfile.h"
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

extern CSourceFileTypes SourceFileTypes;
CProcessingItemList ProcessingItemList;

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
        if(PIS_READY == pi->State) SetBit(PILS_HAS_READY);
        if(PIS_DONE == pi->State) SetBit(PILS_HAS_DONE);
        if(PIS_FAILED == pi->State) SetBit(PILS_HAS_FAILED);
    }
}

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

//CMainFrame
IMPLEMENT_DYNCREATE(CMainFrame, CFrameWndEx)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWndEx)
	ON_WM_CREATE()
    ON_WM_DESTROY()
    ON_REGISTERED_MESSAGE(AFX_WM_RESETTOOLBAR, OnResetToolbar)
    ON_MESSAGE(WM_PROCESSING_THREAD, OnProcessingThread)

    //////////////////////////////////////////////////////////////////////////
    //commands    
    ON_COMMAND(ID_CMD_OPTIONS, &CMainFrame::OnCmdSettings)
    ON_COMMAND(ID_CMD_GITHUB, &CMainFrame::OnCmdGitHub)
    ON_COMMAND(ID_CMD_ABOUT, &CMainFrame::OnCmdAbout)
    ON_COMMAND(ID_CMD_TEST, &CMainFrame::OnCmdTest) //TEST

    ON_COMMAND(ID_CMD_ADD_FILES, &CMainFrame::OnAddFiles)
    ON_COMMAND(ID_CMD_ADD_FOLDER, &CMainFrame::OnAddFolder)
    
    //output profiles
    ON_COMMAND(ID_CMD_PROFILE_ADD, &CMainFrame::OnCmdProfileAdd)
    ON_COMMAND(ID_CMD_PROFILE_SAVE, &CMainFrame::OnCmdProfileSave)
    ON_COMMAND(ID_CMD_PROFILE_DELETE, &CMainFrame::OnCmdProfileDelete)
    ON_COMMAND(ID_CMD_PROFILE_PREVIEW, &CMainFrame::OnProfilePreview)
    ON_CBN_SELCHANGE(ID_PROFILE_COMBO, OnProfileCombo)

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

    //////////////////////////////////////////////////////////////////////////
    //update UI
    ON_UPDATE_COMMAND_UI(ID_PROFILE_COMBO, OnUpdateUI)
    ON_UPDATE_COMMAND_UI(ID_CMD_PROFILE_ADD, OnUpdateUI)
    ON_UPDATE_COMMAND_UI(ID_CMD_PROFILE_SAVE, OnUpdateUI)
    ON_UPDATE_COMMAND_UI(ID_CMD_PROFILE_DELETE, OnUpdateUI)
    ON_UPDATE_COMMAND_UI(ID_CMD_PROFILE_PREVIEW, OnUpdateUI)

    ON_UPDATE_COMMAND_UI(ID_CMD_OPEN_VIDEO, OnUpdateUI)
    ON_UPDATE_COMMAND_UI(ID_CMD_OPEN_PREVIEW, OnUpdateUI)
    ON_UPDATE_COMMAND_UI(ID_CMD_BROWSE_TO_VIDEO, OnUpdateUI)
    ON_UPDATE_COMMAND_UI(ID_CMD_BROWSE_TO_PREVIEW, OnUpdateUI)

    ON_UPDATE_COMMAND_UI(ID_CMD_PROCESS_ALL, OnUpdateUI)
    ON_UPDATE_COMMAND_UI(ID_CMD_PROCESS_SELECTED, OnUpdateUI)
    ON_UPDATE_COMMAND_UI(ID_CMD_STOP_PROCESSING, OnUpdateUI)

    ON_UPDATE_COMMAND_UI(ID_CMD_RESET_SELECTED, OnUpdateUI)
    ON_UPDATE_COMMAND_UI(ID_CMD_RESET_ALL, OnUpdateUI)

    ON_UPDATE_COMMAND_UI(ID_CMD_REMOVE_FAILED, OnUpdateUI)
    ON_UPDATE_COMMAND_UI(ID_CMD_REMOVE_COMPLETED, OnUpdateUI)
    ON_UPDATE_COMMAND_UI(ID_CMD_REMOVE_SELECTED, OnUpdateUI)
    ON_UPDATE_COMMAND_UI(ID_CMD_REMOVE_ALL, OnUpdateUI)
END_MESSAGE_MAP()

// CMainFrame construction/destruction
CMainFrame::CMainFrame() : IsProcessing(false)
{
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

    //TODO:
    DWORD menu_bar_sytle = MainMenu.GetPaneStyle();
    menu_bar_sytle &= ~(CBRS_SIZE_DYNAMIC | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_FLOATING | CBRS_FLOAT_MULTI | CBRS_GRIPPER);
    MainMenu.SetPaneStyle(menu_bar_sytle);
    MainMenu.SetRecentlyUsedMenus(FALSE);
    MainMenu.SetShowAllCommands(TRUE);
	//MainMenu.SetPaneStyle(MainMenu.GetPaneStyle() /*| CBRS_SIZE_DYNAMIC | CBRS_TOOLTIPS | CBRS_FLYBY*/);

	// prevent the menu bar from taking the focus on activation
	CMFCPopupMenu::SetForceMenuFocus(FALSE);

    //TODO:
	if(!ToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) || !ToolBar.LoadToolBar(IDR_MAINFRAME_256))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

    ////////////////////////////////////////////////////////
    //TODO: create profile combo
    CRect rectDummy;
	rectDummy.SetRectEmpty();
    //CToolBarCtrl toolbar_ctrl = ToolBar.GetToolBarCtrl();
	const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_BORDER | CBS_SORT | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
    //CBProfile = new CMFCToolBarComboBoxButton(ID_PROFILE_COMBO, NULL, 3, 100);

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

    DockPane(&MainMenu);
    DockPane(&ToolBar);

    //TODO:
    CDockState state;
    state.Clear();
    SetDockState(state);
    GetDockingManager()->DisableRestoreDockState(TRUE);
    GetDockingManager()->EnableDocking(CBRS_NOALIGN);

    //TODO:
	// Outlook bar is created and docking on the left side should be allowed.
	//EnableDocking(CBRS_ALIGN_LEFT);

	// Load menu item image (not placed on any standard toolbars):
	CMFCToolBar::AddToolBarForImageCollection(IDR_MENU_IMAGES, IDB_MENU_IMAGES_24);

	//create profile window
	CString strPropertiesWnd;
	bNameValid = strPropertiesWnd.LoadString(IDS_PROPERTIES_WND);
	ASSERT(bNameValid);
    if(!ProfilePane.Create(strPropertiesWnd, this, CRect(0, 0, Options.ProfilePaneWidth, 200), FALSE, ID_VIEW_PROPERTIESWND, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |  CBRS_LEFT, AFX_CBRS_REGULAR_TABS, AFX_CBRS_RESIZE)) 
	{
		TRACE0("Failed to create Profile window\n");
		return FALSE; // failed to create
	}
    DockPane(&ProfilePane);

    //TODO:
    ProfilePane.SetOutputProfile(&DefaultProfile);

    CBProfile = GetProfileCombo();
    UpdateProfileCombo();

    ItemsListState.Update();

    return 0;
}
void CMainFrame::OnDestroy()
{
    ProcessingThread.Stop();

    //TEST:
    ProfilePane.GetOutputProfile(&DefaultProfile);

    CRect rect;
    ProfilePane.GetClientRect(&rect);
    Options.ProfilePaneWidth = rect.Width();

    CFrameWndEx::OnDestroy();
}
void CMainFrame::OnUpdateFrameTitle(BOOL bAddToTitle)
{
    //TODO: frame title is first name from IDR_MAINFRAME
    //CFrameWndEx::OnUpdateFrameTitle(bAddToTitle);
    ::AfxSetWindowText(m_hWnd, APP_NAME _T(" (build: ") APP_BUILD _T(")"));
}
BOOL CMainFrame::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
    LPNMHDR nmhdr = reinterpret_cast<LPNMHDR>(lParam);

    CView* flv = GetActiveView(); //file list view
    if(flv && nmhdr->hwndFrom == flv->m_hWnd)
    {
        if(NM_DBLCLK == nmhdr->code)
        {
            OnCmdOpenVideo();
        }
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
	if( !CFrameWndEx::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

    cs.style &= ~FWS_ADDTOTITLE;

	return TRUE;
}
LRESULT CMainFrame::OnResetToolbar(WPARAM wp,LPARAM lp)
{
    CMFCToolBarComboBoxButton profile_combo(ID_PROFILE_COMBO, 0, CBS_DROPDOWNLIST, 300);
    ToolBar.ReplaceButton(ID_PROFILE_COMBO, profile_combo);

    //TODO:
    CMFCToolBarComboBoxButton* cc = GetProfileCombo();

    return 0;
}

CMFCToolBarComboBoxButton* CMainFrame::GetProfileCombo()
{
    static int profile_combo_index = -1;

    if(profile_combo_index < 0)
    {
        for(int index = 0; index < ToolBar.GetCount(); ++index)
        {
            if(ID_PROFILE_COMBO == ToolBar.GetItemID(index))
            {
                profile_combo_index = index;
                break;
            }
        }
    }
    ASSERT(profile_combo_index >= 0);
    return static_cast<CMFCToolBarComboBoxButton*>(ToolBar.GetButton(profile_combo_index));
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
	//ProfilePane.ShowPane(TRUE, FALSE, TRUE);
	//ProfilePane.SetFocus();
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
void CMainFrame::OnAddFiles()
{
    const int FILE_LIST_BUFFER_SIZE = ((255 * (MAX_PATH + 1)) + 1);
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
    root_dir += _T("\\");
    
    const int items_count = GetFileListView()->GetListCtrl().GetItemCount();
    for(LPCTSTR file_names = ofn.lpstrFile + name_size + 1; ;file_names += name_size + 1)
    {
        CString file_name = file_names;
        name_size = file_name.GetLength();
        if(0 == name_size)
            break;

        PProcessingItem pi(new CProcessingItem(PIS_READY, root_dir + file_name));
        AddItem(pi);
    }

    //if has new items in list
    if(items_count < GetFileListView()->GetListCtrl().GetItemCount())
    {
        GetFileListView()->Sort();
        ItemsListState.SetReady(false);
        UpdateDialogControls(this, FALSE);
    }
}
void CMainFrame::OnAddFolder()
{
    CFolderPickerDialog fpd(NULL, OFN_ALLOWMULTISELECT | OFN_ENABLESIZING | OFN_EXPLORER, this, sizeof(OPENFILENAME));
    const int FILE_LIST_BUFFER_SIZE = ((255 * (MAX_PATH + 1)) + 1);
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
        ItemsListState.SetReady(false);
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
            PProcessingItem pi(new CProcessingItem(PIS_READY, root_dir + _T("\\") + found_name));
            AddItem(pi);
        }
    }
}
LRESULT CMainFrame::OnProcessingThread(WPARAM wp, LPARAM lp)
{
    //TODO: block current item and profile pane while processing
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
        CurrentItem->State = PIS_READY;
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

        const COutputProfile* current_profile = GetCurrentProfile();
        if(NULL == current_profile)
        {
            //TODO: message
            break;
        }

        const DWORD result = ProcessingThread.Start(m_hWnd, current_profile, pi->SourceFileName);
        if(ERROR_SUCCESS == result)
        {
            CurrentItem = pi;
            SetProcessingState(true);
            return true;
        }
        else
        {
            ; //TODO: message
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
    const int result = AfxMessageBox(_T("Remove failed files from list?"), MB_OKCANCEL | MB_ICONEXCLAMATION | MB_DEFBUTTON1);
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
    const int result = AfxMessageBox(_T("Remove completed files from list?"), MB_OKCANCEL | MB_ICONEXCLAMATION | MB_DEFBUTTON1);
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
    const int result = AfxMessageBox(_T("Remove all files from list?"), MB_OKCANCEL | MB_ICONEXCLAMATION | MB_DEFBUTTON1);
    if(result != IDOK) return;
    RemoveAllItems();
}
void CMainFrame::OnCmdSettings()
{
    CDialogSettings dialog(this);
    dialog.DoModal();
}
void CMainFrame::OnProfileCombo()
{
    COutputProfile* current_profile = GetCurrentProfile();
    SelectedOutputProfile = (current_profile == &DefaultProfile) ? NULL : current_profile;
    ProfilePane.SetOutputProfile(current_profile);
}
void CMainFrame::OnCmdProfileAdd()
{
    CDialogOutputProfile dialog(this, true);
    if(IDOK != dialog.DoModal())
        return;

    POutputProfile new_profile(new COutputProfile);
    if(dialog.IsCopyFrom && false == dialog.CopyFrom.IsEmpty())
    {
        const COutputProfile* op = ::GetOutputProfile(dialog.CopyFrom);  
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

    OutputProfiles[dialog.ProfileName] = new_profile;
    theApp.WriteProfile(dialog.ProfileName);

    //update controls
    UpdateProfileCombo();
    ProfilePane.SetOutputProfile(new_profile.get());
}
void CMainFrame::OnCmdProfileSave()
{
    CDialogOutputProfile dialog(this, false);
    if(IDOK != dialog.DoModal())
        return;

    COutputProfile* profile_to_save = ::GetOutputProfile(dialog.ProfileName);
    if(profile_to_save)
    {
        ProfilePane.GetOutputProfile(profile_to_save);
    }
    else
    {
        POutputProfile new_profile(new COutputProfile);
        OutputProfiles[dialog.ProfileName] = new_profile;
        profile_to_save = new_profile.get();
    }

    ProfilePane.GetOutputProfile(profile_to_save);
    theApp.WriteProfile(dialog.ProfileName);

    UpdateProfileCombo();
}
void CMainFrame::OnCmdProfileDelete()
{
    const COutputProfile* current_profile = GetCurrentProfile();

    //default profile
    if(current_profile == &DefaultProfile) 
        return;

    CString selected_profile_name = CBProfile->GetItem();
    CString msg = _T("Do you want to delete profile\r\n\"") + CString(selected_profile_name) + _T("\" ?");
    const int result = AfxMessageBox(msg, MB_OKCANCEL | MB_ICONEXCLAMATION | MB_DEFBUTTON1);
    if(result != IDOK) return;

    theApp.DeleteProfile(selected_profile_name);
    COutputProfiles::iterator profile_i = OutputProfiles.find(selected_profile_name);
    if(profile_i != OutputProfiles.end()) 
        OutputProfiles.erase(profile_i);

    SetSelectedOutputProfile(NULL);

    UpdateProfileCombo();
}
void CMainFrame::UpdateProfileCombo()
{
    CBProfile->RemoveAllItems();
    CBProfile->AddItem(CURRENT_OUTPUT_PROFILE_NAME, reinterpret_cast<DWORD_PTR>(&DefaultProfile));
    for(COutputProfiles::const_iterator profile_i = OutputProfiles.begin(); profile_i != OutputProfiles.end(); ++profile_i)
    {
        POutputProfile profile = profile_i->second;
        CBProfile->AddItem(profile_i->first, reinterpret_cast<DWORD_PTR>(profile.get()));
    }

    //set selected item
    BOOL result = FALSE;
    LPCTSTR profile_name = GetSelectedOutputProfileName();
    if(SelectedOutputProfile)
        result = CBProfile->SelectItem(reinterpret_cast<DWORD_PTR>(SelectedOutputProfile));
    else
        result = CBProfile->SelectItem(0, FALSE);

    //TODO:
    ASSERT(result);

    ToolBar.Invalidate();
}
void CMainFrame::OnProfilePreview()
{
    //TODO:
    int i = 0;
}
void CMainFrame::OnCmdTest()
{
    //TEST:
    //COutputProfile profile;
    //profile.SetDefault();
    //ProfilePane.SetOutputProfile(&profile);
    //UpdateDialogControls(this, FALSE);
    //MessageBox(L"OnCmdTest", L"DEBUG", MB_OK | MB_ICONINFORMATION);
    
    //TEST:
    RemoveAllItems();
    AddItem(PProcessingItem(new CProcessingItem(_T("d:\\projects\\VideoPreview\\videos\\Three Cute Golden Retrievers Hug Over Tennis Ball.mp4"))));
    AddItem(PProcessingItem(new CProcessingItem(_T("d:\\projects\\VideoPreview\\videos\\Chicken_Techno_by_Oli_Chang.mp4"))));
    AddItem(PProcessingItem(new CProcessingItem(_T("d:\\projects\\VideoPreview\\videos\\Frankie the pug walking on his front legs!.mp4"))));

    int id1 = GetFileListView()->GetDlgCtrlID(); //id1 = 59648
    int id2 = GetFileListView()->GetListCtrl().GetDlgCtrlID(); //id2 = 59648
}
void CMainFrame::OnUpdateUI(CCmdUI* pCmdUI)
{
    switch(pCmdUI->m_nID)
    {
    case ID_CMD_OPTIONS:
        pCmdUI->Enable(false == IsProcessing);
        break;
    case ID_PROFILE_COMBO:
    case ID_CMD_PROFILE_ADD:
    case ID_CMD_PROFILE_SAVE:
    case ID_CMD_PROFILE_PREVIEW:
        pCmdUI->Enable(false == IsProcessing);
        break;
    case ID_CMD_PROFILE_DELETE:
    {
        const COutputProfile* current_profile = GetCurrentProfile();
        ASSERT(current_profile);
        pCmdUI->Enable(current_profile != &DefaultProfile);
        break;
    }

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
COutputProfile* CMainFrame::GetCurrentProfile()
{
    COutputProfile* result = reinterpret_cast<COutputProfile*>(CBProfile->GetItemData());
    ASSERT(result);
    return result;
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
    //TODO:
    int index = -1;
    CProcessingItem* pi = GetFileListView()->GetFocusedItem(&index);
    if(NULL == pi) return;
    pi->Reset(false);
    GetFileListView()->UpdateItem(pi, index);
}
void CMainFrame::OnCmdResetAll()
{
    //TODO:
    //int index = -1;
    //CProcessingItem* pi = GetFileListView()->GetFocusedItem(&index);
    //if(NULL == pi) return;
    //pi->Reset(false);
    //GetFileListView()->UpdateItem(pi, index);
}
void CMainFrame::OnCmdRemoveSelected()
{
    CFileListView* flv = GetFileListView();
    CListCtrl& lc = flv->GetListCtrl();
    const int sel_count = lc.GetSelectedCount();
    if(0 == sel_count)
        return;

    const int result = AfxMessageBox(_T("Remove selected files from list?"), MB_OKCANCEL | MB_ICONEXCLAMATION | MB_DEFBUTTON1);
    if(result != IDOK) 
        return;

    for(;;)
    {
        const int found_index = lc.GetNextItem(-1, LVNI_SELECTED);
        if(found_index < 0)
            break;

        CProcessingItem* pi = flv->FindItem(found_index);
        if(pi == CurrentItem.get()) continue;

        lc.DeleteItem(found_index);
        CProcessingItemList::iterator i = ProcessingItemList.find(pi);
        if(i != ProcessingItemList.end())
            ProcessingItemList.erase(i);
    }

    ItemsListState.Update();
}

