#include "stdafx.h"
#include "afxwinappex.h"
#include "afxdialogex.h"
#pragma hdrstop
#include "app_thread.h"
#include "About.h"
#include "Resource.h"
#include "ClipboardFiles.h"
#include "VideoFileTypes.h"
#include "OutputProfile.h"
#include "OutputProfileList.h"
#include "ProcessingItem.h"
#include "Settings.h"
#include "ScreenshotGenerator.h"
#include "ProcessingThread.h"
#include "VideoPreview.h"
#include "VideoPreviewDoc.h"
#include "ProfilePane.h"
#include "FileListView.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

int ProcessingState;

// CMainApp
BEGIN_MESSAGE_MAP(CMainApp, CWinAppEx)
END_MESSAGE_MAP()

// CMainApp construction
CMainApp::CMainApp()
{
	// support Restart Manager
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_ALL_ASPECTS;
#ifdef _MANAGED
	// If the application is built using Common Language Runtime support (/clr):
	//     1) This additional setting is needed for Restart Manager support to work properly.
	//     2) In your project, you must add a reference to System.Windows.Forms in order to build.
	System::Windows::Forms::Application::SetUnhandledExceptionMode(System::Windows::Forms::UnhandledExceptionMode::ThrowException);
#endif

	// TODO: replace application ID string below with unique ID string; recommended
	// format for string is CompanyName.ProductName.SubProduct.VersionInformation
	SetAppID(_T("Kaluga456.VideoPreview.VideoPreview.NoVersion"));

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
    m_bSaveState = FALSE;
}

// The one and only CMainApp object
CMainApp theApp;

// CMainApp initialization
BOOL CMainApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	//Set this to include all the common control classes you want to use in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinAppEx::InitInstance();

	EnableTaskbarInteraction(FALSE);

    //TODO: for drag`n`drop
    AfxOleInit();

	// AfxInitRichEdit2() is required to use RichEdit control	
	// AfxInitRichEdit2();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization

    //NOTE: the root registry key is HKEY_CURRENT_USER\Software\Kaluga456\VideoPreview
	SetRegistryKey(_T("Kaluga456"));
    SetRegistryBase(_T(""));
	//LoadStdProfileSettings(4);  // Load standard INI file options (including MRU)
    
    Settings.Read();

	InitContextMenuManager();
	InitShellManager();
	InitKeyboardManager();

	InitTooltipManager();
	CMFCToolTipInfo ttParams;
	ttParams.m_bVislManagerTheme = TRUE;
	theApp.GetTooltipManager()->SetTooltipParams(AFX_TOOLTIP_TYPE_ALL, RUNTIME_CLASS(CMFCToolTipCtrl), &ttParams);

    //use system menu font as common font for all controls
    NONCLIENTMETRICS ncm;
    ncm.cbSize = sizeof(ncm);
    SystemParametersInfo(SPI_GETNONCLIENTMETRICS, NULL, &ncm, NULL);
    AppFont.CreateFontIndirect(&ncm.lfMenuFont);
    CMFCMenuBar::SetMenuFont(&ncm.lfMenuFont,  TRUE);

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views
	CSingleDocTemplate* pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CVideoPreviewDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CFileListView));
	if(!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);

	//TODO: Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Dispatch commands specified on the command line.  Will return FALSE if
	// app was launched with /RegServer, /Register, /Unregserver or /Unregister.
	if(!ProcessShellCommand(cmdInfo))
		return FALSE;

	// The one and only window has been initialized, so show and update it
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();

	return TRUE;
}
int CMainApp::ExitInstance()
{
    Settings.Write();
    
    return CWinAppEx::ExitInstance();
}

// CMainApp customization load/save methods
void CMainApp::PreLoadState()
{
    //TODO:
	//BOOL bNameValid;
	//CString strName;
	//bNameValid = strName.LoadString(IDS_EDIT_MENU);
	//ASSERT(bNameValid);
	//GetContextMenuManager()->AddMenu(strName, IDR_POPUP_EDIT);
	//bNameValid = strName.LoadString(IDS_EXPLORER);
	//ASSERT(bNameValid);
	//GetContextMenuManager()->AddMenu(strName, IDR_POPUP_EXPLORER);
}
