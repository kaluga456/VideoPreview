#include "stdafx.h"
#pragma hdrstop
#include "Resource.h"
#include "SourceFileTypes.h"
#include "OutputProfile.h"
#include "VideoPreview.h"
#include "DialogAbout.h"
#include "DialogSettings.h"
#include "ProfilePane.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWndEx)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWndEx)
	ON_WM_CREATE()
    ON_WM_DESTROY()
	ON_REGISTERED_MESSAGE(AFX_WM_CREATETOOLBAR, &CMainFrame::OnToolbarCreateNew)

    //commands    
    ON_COMMAND(ID_FILE_ADDFILES, &CMainFrame::OnAddFiles)
    ON_COMMAND(ID_FILE_ADDFOLDER, &CMainFrame::OnAddFolder)
    ON_COMMAND(ID_FILE_REMOVECOMPLETED, &CMainFrame::OnRemoveCompleted)
    ON_COMMAND(ID_FILE_STARTPROCESSING, &CMainFrame::OnStartProcessing)
    ON_COMMAND(ID_FILE_STOPPROCESSING, &CMainFrame::OnStopProcessing)
    ON_COMMAND(ID_FILE_REMOVEALL, &CMainFrame::OnRemopeAll)       
    ON_COMMAND(ID_FILE_OPTIONS, &CMainFrame::OnOptions)
    ON_COMMAND(ID_APP_ABOUT, &CMainFrame::OnAppAbout) //TEST:
    ON_COMMAND(ID_EDIT_TEST, &CMainFrame::OnEditTest) //TEST

    //update UI
 

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
CMainFrame::CMainFrame()
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

	if(!ToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!ToolBar.LoadToolBar(theApp.m_bHiColorIcons ? IDR_MAINFRAME_256 : IDR_MAINFRAME))
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
	//ToolBar.EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, strCustomize);

	if(!StatusBar.Create(this))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}
	StatusBar.SetIndicators(SBIndicators, sizeof(SBIndicators)/sizeof(UINT));

    DockPane(&MainMenu);
    DockPane(&ToolBar);

    //TODO:
    //HKCU\Software\Local AppWizard-Generated Applications\VideoPreview\Workspace\WindowPlacement\MainWindowRect
    CDockState state;
    state.Clear();
    SetDockState(state);
    GetDockingManager()->DisableRestoreDockState(TRUE);
    GetDockingManager()->EnableDocking(CBRS_NOALIGN);

    //TODO:
	// Outlook bar is created and docking on the left side should be allowed.
	//EnableDocking(CBRS_ALIGN_LEFT);

	// Load menu item image (not placed on any standard toolbars):
	CMFCToolBar::AddToolBarForImageCollection(IDR_MENU_IMAGES, theApp.m_bHiColorIcons ? IDB_MENU_IMAGES_24 : 0);

	//create profile window
	CString strPropertiesWnd;
	bNameValid = strPropertiesWnd.LoadString(IDS_PROPERTIES_WND);
	ASSERT(bNameValid);
    if(!ProfilePane.Create(strPropertiesWnd, this, CRect(0, 0, 300, 200), FALSE, ID_VIEW_PROPERTIESWND, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |  CBRS_LEFT, AFX_CBRS_REGULAR_TABS, AFX_CBRS_RESIZE)) 
	{
		TRACE0("Failed to create Profile window\n");
		return FALSE; // failed to create
	}
    DockPane(&ProfilePane);

    //TODO:
    ProfilePane.SetOutputProfile(&DefaultProfile);

    return 0;
}

void CMainFrame::OnDestroy()
{
    //TEST:
    ProfilePane.GetOutputProfile(&DefaultProfile);

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


// CMainFrame message handlers
LRESULT CMainFrame::OnToolbarCreateNew(WPARAM wp,LPARAM lp)
{
	LRESULT lres = CFrameWndEx::OnToolbarCreateNew(wp,lp);
	if (lres == 0)
	{
		return 0;
	}

	CMFCToolBar* pUserToolbar = (CMFCToolBar*)lres;
	ASSERT_VALID(pUserToolbar);

	BOOL bNameValid;
	CString strCustomize;
	bNameValid = strCustomize.LoadString(IDS_TOOLBAR_CUSTOMIZE);
	ASSERT(bNameValid);

	//pUserToolbar->EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, strCustomize);
	return lres;
}
void CMainFrame::OnViewPropertiesWindow()
{
	// Show or activate the pane, depending on current state.  The
	// pane can only be closed via the [x] button on the pane frame.
	ProfilePane.ShowPane(TRUE, FALSE, TRUE);
	ProfilePane.SetFocus();
}
void CMainFrame::OnAddFiles()
{
    MessageBox(L"OnAddFiles", L"DEBUG", MB_OK | MB_ICONINFORMATION);
}
void CMainFrame::OnAddFolder()
{
}
void CMainFrame::OnStartProcessing()
{
    // TODO: Add your command handler code here
}
void CMainFrame::OnStopProcessing()
{
    // TODO: Add your command handler code here
}
void CMainFrame::OnRemoveCompleted()
{
    // TODO: Add your command handler code here
}
void CMainFrame::OnRemopeAll()
{
    // TODO: Add your command handler code here
}
void CMainFrame::OnOptions()
{
    CDialogSettings dialog(this);
    dialog.DoModal();
}
void CMainFrame::OnEditTest()
{
    COutputProfile profile;
    profile.SetDefault();
    ProfilePane.SetOutputProfile(&profile);
    //MessageBox(L"OnEditTest", L"DEBUG", MB_OK | MB_ICONINFORMATION);
}