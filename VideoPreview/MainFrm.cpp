#include "stdafx.h"
#pragma hdrstop
#include "app_thread.h"
#include "Resource.h"
#include "Options.h"
#include "SourceFileTypes.h"
#include "OutputProfile.h"
#include "ProcessingItem.h"
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

CProcessingItemList ProcessingItemList;

// CMainFrame
IMPLEMENT_DYNCREATE(CMainFrame, CFrameWndEx)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWndEx)
	ON_WM_CREATE()
    ON_WM_DESTROY()
    ON_REGISTERED_MESSAGE(AFX_WM_RESETTOOLBAR, OnResetToolbar)
    ON_MESSAGE(WM_PROCESSING_THREAD, OnProcessingThread)

    //commands    
    ON_COMMAND(ID_FILE_ADDFILES, &CMainFrame::OnAddFiles)
    ON_COMMAND(ID_FILE_ADDFOLDER, &CMainFrame::OnAddFolder)
    ON_COMMAND(ID_FILE_REMOVECOMPLETED, &CMainFrame::OnRemoveCompleted)
    ON_COMMAND(ID_FILE_REMOVEALL, &CMainFrame::OnRemoveAll)
    ON_COMMAND(ID_FILE_STARTPROCESSING, &CMainFrame::OnStartProcessing)
    ON_COMMAND(ID_FILE_STOPPROCESSING, &CMainFrame::OnStopProcessing)       
    ON_COMMAND(ID_FILE_OPTIONS, &CMainFrame::OnOptions)
    ON_COMMAND(ID_PROFILE_SAVE, &CMainFrame::OnProfileSave)
    ON_COMMAND(ID_PROFILE_DELETE, &CMainFrame::OnProfileDelete)
    ON_COMMAND(ID_PROFILE_PREVIEW, &CMainFrame::OnProfilePreview)
    ON_COMMAND(ID_APP_ABOUT, &CMainFrame::OnAppAbout)
    ON_COMMAND(ID_EDIT_TEST, &CMainFrame::OnEditTest) //TEST

    //other
    ON_CBN_SELCHANGE(ID_PROFILE_COMBO, OnProfileCombo)

    //TODO: update UI
    ON_UPDATE_COMMAND_UI(ID_FILE_REMOVECOMPLETED, OnUpdateUI)
    ON_UPDATE_COMMAND_UI(ID_FILE_REMOVEALL, OnUpdateUI)
    ON_UPDATE_COMMAND_UI(ID_FILE_STARTPROCESSING, OnUpdateUI)
    ON_UPDATE_COMMAND_UI(ID_FILE_STOPPROCESSING, OnUpdateUI)
    ON_UPDATE_COMMAND_UI(ID_PROFILE_SAVE, OnUpdateUI)
    ON_UPDATE_COMMAND_UI(ID_PROFILE_DELETE, OnUpdateUI)
    ON_UPDATE_COMMAND_UI(ID_PROFILE_COMBO, OnUpdateUI)

END_MESSAGE_MAP()

void CMainFrame::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

//TODO:
static UINT SBIndicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
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
	if (CFrameWndEx::OnCreate(lpCreateStruct) == -1)
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
     //TODO:
    int a = 0;
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
    //TODO:
    MessageBox(L"OnAddFiles", L"DEBUG", MB_OK | MB_ICONINFORMATION);
}
void CMainFrame::OnAddFolder()
{
    //TODO:
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
        file_list_view->UpdateItem(CurrentItem);
        return 0;
    case PTM_DONE:       //LPARAM - result text, including output file name (LPTSTR)
        //TODO:
        CurrentItem->State = PIS_DONE;
        file_list_view->UpdateItem(CurrentItem);
        CurrentItem.reset();
        IsProcessing = false;
        break;
    case PTM_FAILED:      //LPARAM - error description (LPTSTR)
        //TODO:
        CurrentItem->State = PIS_FAILED;
        file_list_view->UpdateItem(CurrentItem);
        CurrentItem.reset();
        IsProcessing = false;
        break;
    default:
        ASSERT(FALSE);
        return 0;
    }

    //TODO:
    //PProcessingItem pi = file_list_view->GetUnprocessedItem();
    //if(NULL == pi)
    //{
    //    //TODO: update UI
    //    IsProcessing = false;
    //    return 0;
    //}
    //const COutputProfile* current_profile = GetCurrentProfile();
    //if(NULL == current_profile)
    //    return 0;
    //const DWORD result = ProcessingThread.Start(m_hWnd, current_profile, pi->SourceFileName);
    //if(ERROR_SUCCESS != result)
    //{
    //    ; //TODO: message
    //    //TODO: update UI
    //    IsProcessing = false;
    //    return 0;
    //}

    return 0;
}
void CMainFrame::OnStartProcessing()
{
    ASSERT(false == IsProcessing);
    if(IsProcessing) return;

    CFileListView* file_list_view = GetFileListView();
    if(NULL == file_list_view)
        return;

    const COutputProfile* current_profile = GetCurrentProfile();
    if(NULL == current_profile)
        return;

    PProcessingItem pi = file_list_view->GetUnprocessedItem();
    if(NULL == pi)
        return;

    const DWORD result = ProcessingThread.Start(m_hWnd, current_profile, pi->SourceFileName);
    if(ERROR_SUCCESS != result)
    {
        ; //TODO: message
        return;
    }

    CurrentItem = pi;
    IsProcessing = true;
}
void CMainFrame::OnStopProcessing()
{
    ASSERT(IsProcessing);
    if(false == IsProcessing) return;

    //TODO: confirm
    ProcessingThread.Stop();
}
void CMainFrame::OnRemoveCompleted()
{
    // TODO: Add your command handler code here
    const int result = AfxMessageBox(_T("Remove completed files from list?"), MB_OKCANCEL | MB_ICONEXCLAMATION | MB_DEFBUTTON1);
    if(result != IDOK) return;


}
void CMainFrame::OnRemoveAll()
{
    const int result = AfxMessageBox(_T("Remove all files from list?"), MB_OKCANCEL | MB_ICONEXCLAMATION | MB_DEFBUTTON1);
    if(result != IDOK) return;

    CFileListView* file_list_view = GetFileListView();
    file_list_view->GetListCtrl().DeleteAllItems();
    ProcessingItemList.clear();
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

    //CFileListView* file_list_view = static_cast<CFileListView*>(GetActiveView());
    //file_list_view->AddItem(PProcessingItem(new CProcessingItem(_T("d:\\projects\\VideoPreview\\videos\\Frankie the pug walking on his front legs!.mp4"))));

    //TEST:
    AddItem(PProcessingItem(new CProcessingItem(_T("d:\\projects\\VideoPreview\\videos\\Three Cute Golden Retrievers Hug Over Tennis Ball.mp4"))));
    AddItem(PProcessingItem(new CProcessingItem(_T("d:\\projects\\VideoPreview\\videos\\Chicken_Techno_by_Oli_Chang.mp4"))));
    AddItem(PProcessingItem(new CProcessingItem(_T("d:\\projects\\VideoPreview\\videos\\Frankie the pug walking on his front legs!.mp4"))));
}
void CMainFrame::OnUpdateUI(CCmdUI* pCmdUI)
{
    //CWnd::UpdateDialogControls() ?

    //TODO:
    CFileListView* file_list_view = GetFileListView();
    ASSERT(file_list_view);
    switch(pCmdUI->m_nID)
    {
    case ID_REMOVE_COMPLETED:
    {
        CListCtrl& list_ctrl = file_list_view->GetListCtrl();
        const int items_count = list_ctrl.GetItemCount();
        break;
    }
    case ID_PROFILE_DELETE:
    {
        const COutputProfile* current_profile = GetCurrentProfile();
        ASSERT(current_profile);
        pCmdUI->Enable(current_profile != &DefaultProfile);
        break;
    }
    case ID_PROFILE_COMBO:
        pCmdUI->Enable();
        break;
    }
}
void CMainFrame::OnUpdateProfileCombo(CCmdUI* pCmdUI)
{
    pCmdUI->Enable();
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
    //TODO: duplicates
    ProcessingItemList[item.get()] = item;

    CFileListView* flv = GetFileListView();
    flv->AddItem(item);
}
void CMainFrame::RemoveItem(PProcessingItem item)
{
    //TODO: confirm
    CFileListView* flv = GetFileListView();
    flv->RemoveItem(item);
}