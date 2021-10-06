#include "stdafx.h"
#pragma hdrstop
#include "app_thread.h"
#include "Resource.h"
#include "Options.h"
#include "SourceFileTypes.h"
#include "OutputProfile.h"
#include "ProcessingItem.h"
#include "ScreenshotGenerator.h"
#include "ProcessingThread.h"
#include "VideoPreview.h"
#include "DialogAbout.h"
#include "DialogSettings.h"
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
    CString result = reinterpret_cast<LPCTSTR>(value);
    ::free(reinterpret_cast<void*>(value));
    return result;
}

// CMainFrame
IMPLEMENT_DYNCREATE(CMainFrame, CFrameWndEx)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWndEx)
	ON_WM_CREATE()
    ON_WM_DESTROY()
    ON_WM_CONTEXTMENU() //TEST:
    ON_REGISTERED_MESSAGE(AFX_WM_RESETTOOLBAR, OnResetToolbar)
    ON_MESSAGE(WM_PROCESSING_THREAD, OnProcessingThread)

    //commands    
    ON_COMMAND(ID_FILE_ADDFILES, &CMainFrame::OnAddFiles)
    ON_COMMAND(ID_FILE_ADDFOLDER, &CMainFrame::OnAddFolder)
    ON_COMMAND(ID_FILE_REMOVEFAILED, &CMainFrame::OnRemoveFailed)
    ON_COMMAND(ID_FILE_REMOVE_COMPLETED, &CMainFrame::OnRemoveCompleted)
    ON_COMMAND(ID_FILE_REMOVE_ALL, &CMainFrame::OnRemoveAll)
    ON_COMMAND(ID_FILE_STARTPROCESSING, &CMainFrame::OnStartProcessing)
    ON_COMMAND(ID_FILE_STOPPROCESSING, &CMainFrame::OnStopProcessing)       
    ON_COMMAND(ID_FILE_OPTIONS, &CMainFrame::OnOptions)
    ON_COMMAND(ID_PROFILE_SAVE, &CMainFrame::OnProfileSave)
    ON_COMMAND(ID_PROFILE_DELETE, &CMainFrame::OnProfileDelete)
    ON_COMMAND(ID_PROFILE_PREVIEW, &CMainFrame::OnProfilePreview)
    ON_COMMAND(ID_APP_ABOUT, &CMainFrame::OnAppAbout)
    ON_COMMAND(ID_EDIT_TEST, &CMainFrame::OnEditTest) //TEST

    //file list context menu
    ON_COMMAND(ID_POPUP_OPEN_VIDEO, &CMainFrame::OnContextOpenVideo)
    ON_COMMAND(ID_POPUP_OPEN_PREVIEW, &CMainFrame::OnContextOpenPreview)
    ON_COMMAND(ID_POPUP_PROCESS_ITEM, &CMainFrame::OnContextProcessItem)
    ON_COMMAND(ID_POPUP_RESET_ITEM, &CMainFrame::OnContextResetItem)
    ON_COMMAND(ID_POPUP_REMOVE_ITEM, &CMainFrame::OnContextRemoveItem)

    //other
    ON_CBN_SELCHANGE(ID_PROFILE_COMBO, OnProfileCombo)

    //update UI
    ON_UPDATE_COMMAND_UI(ID_FILE_REMOVEFAILED, OnUpdateUI)
    ON_UPDATE_COMMAND_UI(ID_FILE_REMOVE_COMPLETED, OnUpdateUI)
    ON_UPDATE_COMMAND_UI(ID_FILE_REMOVE_ALL, OnUpdateUI)
    ON_UPDATE_COMMAND_UI(ID_FILE_STARTPROCESSING, OnUpdateUI)
    ON_UPDATE_COMMAND_UI(ID_FILE_STOPPROCESSING, OnUpdateUI)
    ON_UPDATE_COMMAND_UI(ID_PROFILE_SAVE, OnUpdateUI)
    ON_UPDATE_COMMAND_UI(ID_PROFILE_DELETE, OnUpdateUI)
    ON_UPDATE_COMMAND_UI(ID_PROFILE_COMBO, OnUpdateUI)

    ON_UPDATE_COMMAND_UI(ID_POPUP_OPEN_VIDEO, OnUpdateContextMenu)
    ON_UPDATE_COMMAND_UI(ID_POPUP_OPEN_PREVIEW, OnUpdateContextMenu)
    ON_UPDATE_COMMAND_UI(ID_POPUP_PROCESS_ITEM, OnUpdateContextMenu)
    ON_UPDATE_COMMAND_UI(ID_POPUP_RESET_ITEM, OnUpdateContextMenu)
    ON_UPDATE_COMMAND_UI(ID_POPUP_REMOVE_ITEM, OnUpdateContextMenu)
END_MESSAGE_MAP()

void CMainFrame::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

//TODO: need status bar?
static UINT SBIndicators[] =
{
	ID_SEPARATOR           // status line indicator
};

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

 //   //TODO: init profiles
	//CBProfile.AddString(_T("<Current>"));
	//CBProfile.AddString(_T("Profile 1"));
 //   CBProfile.AddString(_T("Profile 2"));
	//CBProfile.SetCurSel(0);
    ////////////////////////////////////////////////////////

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

    //TODO: init profile list
    CBProfile->AddSortedItem(_T("<Default>"), reinterpret_cast<DWORD_PTR>(&DefaultProfile));
    for(COutputProfiles::const_iterator profile_i = OutputProfiles.begin(); profile_i != OutputProfiles.end(); ++profile_i)
    {
        POutputProfile profile = profile_i->second;
        CBProfile->AddSortedItem(profile_i->first, reinterpret_cast<DWORD_PTR>(profile.get()));
    }

    //TODO:
	if(FALSE == CBProfile->SelectItem(Options.SelectedProfile))
        CBProfile->SelectItem(0);

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

    LPCTSTR current_profile = CBProfile->GetItem();
    Options.SelectedProfile = current_profile ? current_profile : _T("");

    CFrameWndEx::OnDestroy();
}
BOOL CMainFrame::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
    LPNMHDR nmhdr = reinterpret_cast<LPNMHDR>(lParam);

    CView* flv = GetActiveView(); //file list view
    if(flv && nmhdr->hwndFrom == flv->m_hWnd)
    {
        //if(LVN_COLUMNCLICK == nmhdr->code) 
        //{
        //    LPNMLISTVIEW nmlv = reinterpret_cast<LPNMLISTVIEW>(lParam);
        //    //OnColumnClick(nmlv->iSubItem);
        //    return TRUE;
        //}
        if(NM_DBLCLK == nmhdr->code)
        {
            OnContextOpenVideo();
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
void CMainFrame::OnStartProcessing()
{
    ProcessNextItem();
}
void CMainFrame::OnStopProcessing()
{
    //TODO: confirm
    SetProcessingState(false);
}
void CMainFrame::OnRemoveFailed()
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
void CMainFrame::OnRemoveCompleted()
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
void CMainFrame::OnRemoveAll()
{
    const int result = AfxMessageBox(_T("Remove all files from list?"), MB_OKCANCEL | MB_ICONEXCLAMATION | MB_DEFBUTTON1);
    if(result != IDOK) return;
    RemoveAllItems();
}
void CMainFrame::OnOptions()
{
    CDialogSettings dialog(this);
    dialog.DoModal();
}
void CMainFrame::OnProfileSave()
{
    //TODO:
}
void CMainFrame::OnProfileDelete()
{
    const COutputProfile* current_profile = GetCurrentProfile();

    //default profile
    if(current_profile == &DefaultProfile) 
        return;

    CString selected_profile_name = CBProfile->GetItem();
    CString msg = _T("Do you want to delete profile\r\n\"") + CString(selected_profile_name) + _T("\" ?");
    const int result = AfxMessageBox(msg, MB_OKCANCEL | MB_ICONEXCLAMATION | MB_DEFBUTTON1);
    if(result != IDOK) return;

    CBProfile->DeleteItem(selected_profile_name);
    theApp.DeleteProfile(selected_profile_name);
    COutputProfiles::iterator profile_i = OutputProfiles.find(selected_profile_name);
    if(profile_i != OutputProfiles.end()) 
        OutputProfiles.erase(profile_i);   
}
void CMainFrame::OnProfilePreview()
{
    //TODO:
    int i = 0;
}
void CMainFrame::OnProfileCombo()
{
    const COutputProfile* current_profile = GetCurrentProfile();
    //TODO: disable delete profile command
    ProfilePane.SetOutputProfile(current_profile);
}
void CMainFrame::OnEditTest()
{
    //TEST:
    //COutputProfile profile;
    //profile.SetDefault();
    //ProfilePane.SetOutputProfile(&profile);
    //UpdateDialogControls(this, FALSE);
    //MessageBox(L"OnEditTest", L"DEBUG", MB_OK | MB_ICONINFORMATION);
    
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
    case ID_FILE_REMOVE_COMPLETED:
    {
        pCmdUI->Enable(ItemsListState.HasDone());
        break;
    }
    case ID_FILE_REMOVEFAILED:
    {
        pCmdUI->Enable(ItemsListState.HasFailed());
        break;
    }
    case ID_FILE_REMOVE_ALL:
    {
        CListCtrl& list_ctrl = GetFileListView()->GetListCtrl();
        const int items_count = list_ctrl.GetItemCount();
        pCmdUI->Enable(items_count);
        break;
    }
    case ID_PROFILE_DELETE:
    {
        const COutputProfile* current_profile = GetCurrentProfile();
        ASSERT(current_profile);
        pCmdUI->Enable(current_profile != &DefaultProfile);
        break;
    }
    case ID_FILE_STARTPROCESSING:
    {
        pCmdUI->Enable(false == IsProcessing);
        break;
    }
    case ID_FILE_STOPPROCESSING:
    {
        pCmdUI->Enable(IsProcessing);
        break;
    }
    case ID_PROFILE_COMBO:
        pCmdUI->Enable(false == IsProcessing);
        break;
    }
}
void CMainFrame::OnUpdateContextMenu(CCmdUI* pCmdUI)
{
    switch(pCmdUI->m_nID)
    {
    case ID_POPUP_OPEN_VIDEO:
    {
        CProcessingItem* pi = GetFileListView()->GetFocusedItem();
        pCmdUI->Enable(pi != NULL);
        break;
    }
    case ID_POPUP_OPEN_PREVIEW:
    {
        CProcessingItem* pi = GetFileListView()->GetFocusedItem();
        pCmdUI->Enable(pi != NULL && PIS_DONE == pi->State);
        break;
    }
    case ID_POPUP_PROCESS_ITEM:
    case ID_POPUP_RESET_ITEM:
    case ID_POPUP_REMOVE_ITEM:
    {
        const int sel_items_count = GetFileListView()->GetListCtrl().GetSelectedCount();
        pCmdUI->Enable(sel_items_count);
        break;
    }
    }
}
const COutputProfile* CMainFrame::GetCurrentProfile()
{
    const COutputProfile* result = reinterpret_cast<const COutputProfile*>(CBProfile->GetItemData());
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
void CMainFrame::OnContextOpenVideo()
{
    CProcessingItem* pi = GetFileListView()->GetFocusedItem();
    if(NULL == pi) return;
    ::ShellExecute(NULL, _T("open"), pi->SourceFileName, NULL, NULL, SW_SHOWNORMAL);
}
void CMainFrame::OnContextOpenPreview()
{
    //TODO:
    CProcessingItem* pi = GetFileListView()->GetFocusedItem();
    ASSERT(PIS_DONE == pi->State);
    if(NULL == pi) return;
    ::ShellExecute(NULL, _T("open"), pi->ResultString, NULL, NULL, SW_SHOWNORMAL);
}
void CMainFrame::OnContextProcessItem()
{
}
void CMainFrame::OnContextResetItem()
{
    int index = -1;
    CProcessingItem* pi = GetFileListView()->GetFocusedItem(&index);
    if(NULL == pi) return;
    pi->Reset(false);
    GetFileListView()->UpdateItem(pi, index);
}
void CMainFrame::OnContextRemoveItem()
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