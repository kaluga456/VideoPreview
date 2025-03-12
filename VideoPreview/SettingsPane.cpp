#include "stdafx.h"
#pragma hdrstop
#include "Resource.h"
#include "Settings.h"
#include "OutputProfile.h"
#include "OutputProfileList.h"
#include "ScreenshotGenerator.h"
#include "ProcessingThread.h"
#include "VideoPreview.h"
#include "SettingsPane.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//profile properties
//NOTE: keep enum and SETTING_DESCR indexes synced
//profile property ids
enum
{
    IDP_BACKGROUND_COLOR,

    IDP_WRITE_HEADER,

    //TODO:
    //IDP_HEADER_TEXT,

    IDP_HEADER_FONT,
    IDP_HEADER_FONT_COLOR,

    IDP_FRAME_COLUMNS,
    IDP_FRAME_ROWS,

    //TODO:
    //IDP_USE_TIME_INTERVAL,
    //IDP_FRAME_TIME_INTERVAL,

    IDP_OUTPUT_SIZE_METHOD,
    IDP_OUTPUT_IMAGE_SIZE,

    //TODO:
    //IDP_BORDER_PADDING,
    //IDP_FRAME_PADDING,

    IDP_TIMESTAMP_TYPE,
    IDP_TIMESTAMP_FONT,
    IDP_TIMESTAMP_FONT_COLOR,

    //TODO:
    //IDP_OUTPUT_FILE_NAME,

    IDP_OUTPUT_FILE_FORMAT,

    //common settings
    ID_SETTINGS_OVERWRITE_OUTPUT_FILES,
    ID_SETTINGS_SAVE_FILELIST_ON_EXIT,

    SETTING_COUNT
};

//TODO:
//profile property descriptions
LPCTSTR SETTING_DESCR[] =
{
    _T("Output image background color"), //IDP_BACKGROUND_COLOR,
    _T("Write summury information about video file.\nSuch as file name, resolution and duration."), //IDP_WRITE_HEADER,

    //TODO:
    //_T("TODO"), //IDP_HEADER_TEXT,

    _T("Output image header font"), //IDP_HEADER_FONT,
    _T("Output image header font color"), //IDP_HEADER_FONT_COLOR,

    _T("Output image columns count"), //IDP_FRAME_COLUMNS,
    _T("Output image rows count"), //IDP_FRAME_ROWS,

    //TODO:
    //_T("TODO"), //USE_TIME_INTERVAL,
    //_T("TODO"), //FRAME_TIME_INTERVAL,

    _T("Output image size calculation method"), //IDP_OUTPUT_SIZE_METHOD,

    _T("Defines one of the following:\n")
    _T("- output image width\n")
    _T("- video frame witdh\n")
    _T("- video frame height\n"), //IDP_OUTPUT_IMAGE_SIZE,

    //TODO:
    //_T("TODO"), //IDP_BORDER_PADDING,
    //_T("TODO"), //IDP_FRAME_PADDING,

    _T("Timestamp position in frame"), //IDP_TIMESTAMP_TYPE,
    _T("Timestamp font"), //IDP_TIMESTAMP_FONT,
    _T("Timestamp font color"), //IDP_TIMESTAMP_FONT_COLOR,

    //TODO:
    //_T("TODO"), //IDP_OUTPUT_FILE_NAME,

    _T("Ouput image format"), //IDP_OUTPUT_FILE_FORMAT,

    //OPTIONS
    _T("Overwrite output images"), //ID_SETTINGS_OVERWRITE_OUTPUT_FILES
    _T("Save file list on exit\nRestore file list on startup"), //ID_SETTINGS_SAVE_FILELIST_ON_EXIT
};
static_assert(sizeof(SETTING_DESCR) / sizeof(LPCTSTR) == SETTING_COUNT, "Invalid SETTING_DESCR size");
/////////////////////////////////////////////////////////////////////////////
//CPGPNumberEdit
CPGPNumberEdit::CPGPNumberEdit(const CString& strName, const COleVariant& varValue, LPCTSTR lpszDescr /*= NULL*/, DWORD_PTR dwData /*= 0*/) :
    CMFCPropertyGridProperty(strName, varValue, lpszDescr, dwData, NULL, NULL, _T("0123456789")) 
{
}
void CPGPNumberEdit::SetInt(int value)
{
    //TODO: assert types workarround
    CString val;
    val.Format(_T("%u"), value);

    COleVariant v = val;
    SetValue(v);    

    _variant_t v1 = GetValue();
}
int CPGPNumberEdit::GetInt()
{
    const COleVariant& v = GetValue();
    LONG lval = 0;
    VarI4FromStr(v.bstrVal, NULL, LOCALE_NOUSEROVERRIDE, &lval);
    return lval;
}
/////////////////////////////////////////////////////////////////////////////
//CPGPCombo
CPGPCombo::CPGPCombo(const CString& strName, const COleVariant& varValue, LPCTSTR lpszDescr /*= NULL*/, DWORD_PTR dwData /*= 0*/) : 
    CMFCPropertyGridProperty(strName, varValue, lpszDescr, dwData) 
{
    AllowEdit(FALSE);
}
void CPGPCombo::AddItem(CString key, INT_PTR value)
{
    ASSERT(false == key.IsEmpty());
    Items[key] = reinterpret_cast<void*>(value);
    AddOption(key);
}
INT_PTR CPGPCombo::GetItem() const
{
    CString key = GetValue();
    void* value;
    return Items.Lookup(key, value) ? reinterpret_cast<INT_PTR>(value) : -1;
}
void CPGPCombo::SetItem(INT_PTR value)
{
    CString key;
    void* val = NULL;
    for(POSITION pos = Items.GetStartPosition(); pos;)
    {
        Items.GetNextAssoc(pos, key, val);
        if(value != reinterpret_cast<INT_PTR>(val)) continue;
        SetValue(key);
        return;
    }

    ASSERT(0);
}
/////////////////////////////////////////////////////////////////////////////
//CPGPFont
CPGPFont::CPGPFont(const CString& strName, LOGFONT& lf, DWORD dwFontDialogFlags /*= CF_EFFECTS | CF_SCREENFONTS*/, 
		LPCTSTR lpszDescr /*= NULL*/, DWORD_PTR dwData /*= 0*/, COLORREF color /*= (COLORREF)-1*/) : 
        CMFCPropertyGridFontProperty(strName, lf, dwFontDialogFlags, lpszDescr, dwData, color)
{
}
void CPGPFont::SetFont(const LOGFONT& logfont)
{
    ::memcpy_s(&m_lf, sizeof(m_lf), &logfont, sizeof(LOGFONT));

    //TODO:
    Redraw();
}
/////////////////////////////////////////////////////////////////////////////
//CSettingsPane
BEGIN_MESSAGE_MAP(CSettingsPane, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	ON_WM_SETTINGCHANGE()

    ON_CBN_SELCHANGE(ID_PROFILE_COMBO, OnProfileComboChanged)
    ON_REGISTERED_MESSAGE(AFX_WM_PROPERTY_CHANGED, OnPropertyChanged)    

    ON_UPDATE_COMMAND_UI(ID_PROFILE_COMBO, OnUpdateUI)
    ON_UPDATE_COMMAND_UI(ID_CMD_PROFILE_ADD, OnUpdateUI)
    ON_UPDATE_COMMAND_UI(ID_CMD_PROFILE_PREVIEW, OnUpdateUI)
    ON_UPDATE_COMMAND_UI(ID_CMD_PROFILE_SAVE, OnUpdateUI)
    ON_UPDATE_COMMAND_UI(ID_CMD_PROFILE_DELETE, OnUpdateUI)
END_MESSAGE_MAP()
CSettingsPane::CSettingsPane() : ProfileChanged(false)
{
}
CSettingsPane::~CSettingsPane()
{
}
int CSettingsPane::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if(CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

    //PGProfile
	CRect rectDummy;
	rectDummy.SetRectEmpty();
	if(!PGProfile.Create(WS_VISIBLE | WS_CHILD, rectDummy, this, 2))
	{
		TRACE0("Failed to create Properties Grid\n");
		return -1;      // fail to create
	}
    PGProfile.SetFont(theApp.GetAppFont(), FALSE);
    PGProfile.MarkModifiedProperties(FALSE, TRUE);
	InitPropList();

    //ToolBar
	ToolBar.Create(this, AFX_DEFAULT_TOOLBAR_STYLE, ID_TOOLBAR_SETTINGS);
	ToolBar.LoadToolBar(ID_TOOLBAR_SETTINGS, 0, 0, TRUE /* Is locked */);
	ToolBar.SetPaneStyle(ToolBar.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);
	ToolBar.SetPaneStyle(ToolBar.GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));
	ToolBar.SetOwner(this);
	ToolBar.SetRouteCommandsViaFrame(FALSE); //All commands will be routed via this control, not via the parent frame:
    CBProfiles = static_cast<CMFCToolBarComboBoxButton*>(ToolBar.GetButton(ToolBar.CommandToIndex(ID_PROFILE_COMBO)));
    ASSERT(CBProfiles);

	AdjustLayout();
    UpdateProfileCombo();

	return 0;
}
void CSettingsPane::InitPropList()
{
    PGProfile.SetBoolLabels(_T("Yes"), _T("No"));
	PGProfile.EnableHeaderCtrl(FALSE);
	PGProfile.EnableDescriptionArea();
	PGProfile.SetVSDotNetLook();
	PGProfile.MarkModifiedProperties();
    PGProfile.SetGroupNameFullWidth(TRUE, FALSE);

    //sample font
    LOGFONT sample_logfont;
	CFont* sample_font = CFont::FromHandle((HFONT)::GetStockObject(DEFAULT_GUI_FONT));
	sample_font->GetLogFont(&sample_logfont);
	lstrcpy(sample_logfont.lfFaceName, _T("Arial"));

    //main
    CMFCPropertyGridProperty* pgp_profile_root = new CMFCPropertyGridProperty(_T("Profile Settings"));
    pgpBackgroundColor = new CMFCPropertyGridColorProperty(_T("Background Color"), RGB(0, 0, 0), NULL, SETTING_DESCR[IDP_BACKGROUND_COLOR], IDP_BACKGROUND_COLOR);
    pgpBackgroundColor->EnableOtherButton(_T("Other..."));
    pgp_profile_root->AddSubItem(pgpBackgroundColor);
    
   
    //header
    //TODO: header text
    //TODO: text color also can be defined in font dialog
    CMFCPropertyGridProperty* pgp_header = new CMFCPropertyGridProperty(_T("Header"));
    pgpWriteHeader = new CMFCPropertyGridProperty(_T("Write Header"), (_variant_t)false, SETTING_DESCR[IDP_WRITE_HEADER], IDP_WRITE_HEADER);
    pgp_header->AddSubItem(pgpWriteHeader);
    pgpHeaderFont = new CPGPFont(_T("Font"), sample_logfont, CF_SCREENFONTS, SETTING_DESCR[IDP_HEADER_FONT], IDP_HEADER_FONT);
    pgpHeaderFontColor = new CMFCPropertyGridColorProperty(_T("Font Color"), RGB(0, 0, 0), NULL, SETTING_DESCR[IDP_HEADER_FONT_COLOR], IDP_HEADER_FONT_COLOR);
    pgpHeaderFontColor->EnableOtherButton(_T("Other..."));
    pgp_header->AddSubItem(pgpHeaderFont);
    pgp_header->AddSubItem(pgpHeaderFontColor);
    pgp_profile_root->AddSubItem(pgp_header);

    //TODO:
    //IDP_HEADER_TEXT

    //frames grid
    CMFCPropertyGridProperty* pgp_frames_grid = new CMFCPropertyGridProperty(_T("Frames Grid"));
    pgpFramesGridColumns = new CMFCPropertyGridProperty(_T("Columns"), (_variant_t) 4l, SETTING_DESCR[IDP_FRAME_COLUMNS], IDP_FRAME_COLUMNS);
	pgpFramesGridRows = new CMFCPropertyGridProperty(_T("Rows"), (_variant_t) 4l, SETTING_DESCR[IDP_FRAME_ROWS], IDP_FRAME_ROWS);  
	pgpFramesGridColumns->EnableSpinControl(TRUE, 1, 10);
    pgpFramesGridRows->EnableSpinControl(TRUE, 1, 10);
	pgp_frames_grid->AddSubItem(pgpFramesGridColumns);
    pgp_frames_grid->AddSubItem(pgpFramesGridRows); 
    pgp_profile_root->AddSubItem(pgp_frames_grid);
    pgpFramesGridColumns->AllowEdit(FALSE);
    pgpFramesGridRows->AllowEdit(FALSE);
    pgp_frames_grid->AllowEdit(FALSE); 
    //TODO: "write frame for each time interval" option
    //IDP_USE_TIME_INTERVAL
    //IDP_FRAME_TIME_INTERVAL

    //output sizes
    CMFCPropertyGridProperty* pgp_output_image_size = new CMFCPropertyGridProperty(_T("Output Image Size"));
    pgpOutputSizeMethod = new CPGPCombo(_T("Size Meaning"), _T("Output image width"), SETTING_DESCR[IDP_OUTPUT_SIZE_METHOD], IDP_OUTPUT_SIZE_METHOD);
    pgpOutputSizeMethod->AddItem(_T("Image width"), OUTPUT_IMAGE_WIDTH_AS_IS);
    pgpOutputSizeMethod->AddItem(_T("Frame Witdh"), OUTPUT_IMAGE_WIDTH_BY_FRAME_WIDTH);
    pgpOutputSizeMethod->AddItem(_T("Frame Height"), OUTPUT_IMAGE_WIDTH_BY_FRAME_HEIGHT);
    pgpOutputSizeMethod->AddItem(_T("Use Original Frame"), OUTPUT_IMAGE_WIDTH_BY_ORIGINAL_FRAME);
    pgpOutputSizeMethod->AllowEdit(FALSE);
    pgpOutputSize = new CPGPNumberEdit(_T("Size Value"), (_variant_t)_T("1000"), SETTING_DESCR[IDP_OUTPUT_IMAGE_SIZE], IDP_OUTPUT_IMAGE_SIZE);
    pgp_output_image_size->AddSubItem(pgpOutputSizeMethod);
    pgp_output_image_size->AddSubItem(pgpOutputSize);
    //pgpOutputSize->Enable(FALSE); //TODO:
    pgp_profile_root->AddSubItem(pgp_output_image_size);
    //TODO:
    //IDP_BORDER_PADDING
    //IDP_FRAME_PADDING

    //timestamp
    CMFCPropertyGridProperty* pgp_timestamp = new CMFCPropertyGridProperty(_T("Timestamp"));
    pgpTimestampType = new CPGPCombo(_T("Type"), _T(""), SETTING_DESCR[IDP_TIMESTAMP_TYPE], IDP_TIMESTAMP_TYPE);
    pgpTimestampType->AddItem(_T("Disabled"), TIMESTAMP_TYPE_DISABLED);
    pgpTimestampType->AddItem(_T("Top-Left"), TIMESTAMP_TYPE_TOP_LEFT);
    pgpTimestampType->AddItem(_T("Top-Center"), TIMESTAMP_TYPE_TOP_CENTER);
    pgpTimestampType->AddItem(_T("Top-Right"), TIMESTAMP_TYPE_TOP_RIGHT);
    pgpTimestampType->AddItem(_T("Bottom-Left"), TIMESTAMP_TYPE_BOTTOM_LEFT);
    pgpTimestampType->AddItem(_T("Bottom-Center"), TIMESTAMP_TYPE_BOTTOM_CENTER);
    pgpTimestampType->AddItem(_T("Bottom-Right"), TIMESTAMP_TYPE_BOTTOM_RIGHT);
    pgpTimestampType->AllowEdit(FALSE);
    pgpTimestampFont = new CPGPFont(_T("Font"), sample_logfont, CF_SCREENFONTS, SETTING_DESCR[IDP_TIMESTAMP_FONT], IDP_TIMESTAMP_FONT);
    pgpTimestampFontColor = new CMFCPropertyGridColorProperty(_T("Font Color"), RGB(0, 0, 0), NULL, SETTING_DESCR[IDP_TIMESTAMP_FONT_COLOR], IDP_TIMESTAMP_FONT_COLOR);
    pgpTimestampFontColor->EnableOtherButton(_T("Other..."));
    pgp_timestamp->AddSubItem(pgpTimestampType);
    pgp_timestamp->AddSubItem(pgpTimestampFont);
    pgp_timestamp->AddSubItem(pgpTimestampFontColor);
    pgp_profile_root->AddSubItem(pgp_timestamp);

    //output file
    CMFCPropertyGridProperty* pgp_output_file = new CMFCPropertyGridProperty(_T("Output File"));
    //pgpOutputFileName = new CMFCPropertyGridProperty(_T("Name"), _T("%n"), SETTING_DESCR[IDP_OUTPUT_FILE_NAME], IDP_OUTPUT_FILE_NAME);
    pgpOutputFileFormat = new CPGPCombo(_T("Format"), _T("JPG"), SETTING_DESCR[IDP_OUTPUT_FILE_FORMAT], IDP_OUTPUT_FILE_FORMAT);    
    pgpOutputFileFormat->AddItem(_T("BMP"), OUTPUT_FILE_FORMAT_BMP);
    pgpOutputFileFormat->AddItem(_T("JPG"), OUTPUT_FILE_FORMAT_JPG);
    pgpOutputFileFormat->AddItem(_T("PNG"), OUTPUT_FILE_FORMAT_PNG);
    //pgp_output_file->AddSubItem(pgpOutputFileName);
    pgp_output_file->AddSubItem(pgpOutputFileFormat);
    pgp_profile_root->AddSubItem(pgp_output_file);

    PGProfile.AddProperty(pgp_profile_root);

    //TODO:
    CMFCPropertyGridProperty* pgp_settings_root = new CMFCPropertyGridProperty(_T("Common Settings"));
    pgpOverwriteFiles = new CMFCPropertyGridProperty(_T("Overwrite Output Files"), (_variant_t)false, SETTING_DESCR[ID_SETTINGS_OVERWRITE_OUTPUT_FILES], ID_SETTINGS_OVERWRITE_OUTPUT_FILES);
    pgpSaveFileListOnExit = new CMFCPropertyGridProperty(_T("Save File List On Exit"), (_variant_t)false, SETTING_DESCR[ID_SETTINGS_SAVE_FILELIST_ON_EXIT], ID_SETTINGS_SAVE_FILELIST_ON_EXIT);
    pgp_settings_root->AddSubItem(pgpOverwriteFiles);
    pgp_settings_root->AddSubItem(pgpSaveFileListOnExit);

    PGProfile.AddProperty(pgp_settings_root);

    //TODO:
	//static const TCHAR szFilter[] = _T("Icon Files(*.ico)|*.ico|All Files(*.*)|*.*||");
	//pGroup3->AddSubItem(new CMFCPropertyGridFileProperty(_T("Icon"), TRUE, _T(""), _T("ico"), 0, szFilter, _T("Specifies the window icon")));
	//pGroup3->AddSubItem(new CMFCPropertyGridFileProperty(_T("Folder"), _T("c:\\")));
	//PGProfile.AddProperty(pGroup3);
}
void CSettingsPane::SetSettings()
{
    pgpOverwriteFiles->SetValue(_variant_t(bool(Settings.OverwriteOutputFiles != 0)));
    pgpSaveFileListOnExit->SetValue(_variant_t(bool(Settings.SaveFileListOnExit != 0)));
}
bool CSettingsPane::OnSettingsChanged(const DWORD_PTR property_id)
{
    switch(property_id)
    {
    case ID_SETTINGS_OVERWRITE_OUTPUT_FILES:
        Settings.OverwriteOutputFiles = pgpOverwriteFiles->GetValue().boolVal;
        return true;
    case ID_SETTINGS_SAVE_FILELIST_ON_EXIT:
        Settings.SaveFileListOnExit = pgpSaveFileListOnExit->GetValue().boolVal;
        return true;
    }
    return false;
}
LRESULT CSettingsPane::OnPropertyChanged(WPARAM wp, LPARAM lp)
{
    CMFCPropertyGridProperty* property = reinterpret_cast<CMFCPropertyGridProperty*>(lp);
    const DWORD_PTR property_id = property->GetData();

    //common settings
    if(OnSettingsChanged(property_id))
        return 0;

    //profile settings
    switch(property_id)
    {
    case IDP_BACKGROUND_COLOR: break;
    case IDP_WRITE_HEADER:
    {
        const BOOL enable = pgpWriteHeader->GetValue().boolVal;
        pgpHeaderFont->Enable(enable);
        pgpHeaderFontColor->Enable(enable);
        break;
    }
    case IDP_HEADER_FONT: break;
    case IDP_HEADER_FONT_COLOR: break;

    case IDP_FRAME_COLUMNS: break;
    case IDP_FRAME_ROWS: break;

    case IDP_OUTPUT_SIZE_METHOD: 
    {
        const BOOL enable = (OUTPUT_IMAGE_WIDTH_BY_ORIGINAL_FRAME != pgpOutputSizeMethod->GetItem());
        pgpOutputSize->Enable(enable);
        break;
    }
    case IDP_OUTPUT_IMAGE_SIZE: break;
    case IDP_TIMESTAMP_TYPE: 
    {
        const BOOL enable = (TIMESTAMP_TYPE_DISABLED != pgpTimestampType->GetItem());
        pgpTimestampFont->Enable(enable);
        pgpTimestampFontColor->Enable(enable);
        break;
    }
    case IDP_TIMESTAMP_FONT: break;
    case IDP_TIMESTAMP_FONT_COLOR: break;
    case IDP_OUTPUT_FILE_FORMAT: break;
    default:
        ASSERT(0);
    }

    ProfileChanged = true;
    return 0;
}
void CSettingsPane::AdjustLayout()
{
	if(GetSafeHwnd() == NULL || (AfxGetMainWnd() != NULL && AfxGetMainWnd()->IsIconic()))
		return;

	CRect rectClient;
	GetClientRect(rectClient);

    //toolbar and grid
	const int cyTlb = ToolBar.CalcFixedLayout(FALSE, TRUE).cy;
	ToolBar.SetWindowPos(NULL, rectClient.left, rectClient.top, rectClient.Width(), cyTlb, SWP_NOACTIVATE | SWP_NOZORDER);
	PGProfile.SetWindowPos(NULL, rectClient.left, rectClient.top + cyTlb, rectClient.Width(), rectClient.Height() - cyTlb, SWP_NOACTIVATE | SWP_NOZORDER);

    //profiles combo
    CRect cb_rect = CBProfiles->Rect();
    cb_rect.right = rectClient.right;
    CBProfiles->SetRect(cb_rect);
    ToolBar.Invalidate(FALSE);
}
void CSettingsPane::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	AdjustLayout();
}
void CSettingsPane::OnUpdateUI(CCmdUI* pCmdUI)
{
    switch(pCmdUI->m_nID)
    {
    case ID_PROFILE_COMBO:
    case ID_CMD_PROFILE_ADD: 
        pCmdUI->Enable(PTS_NONE == ::ProcessingState);
        break;
    case ID_CMD_PROFILE_PREVIEW:
    case ID_CMD_PROFILE_SAVE:
    case ID_CMD_PROFILE_DELETE:
        pCmdUI->Enable(PTS_NONE == ::ProcessingState && false == OutputProfiles.IsEmpty() && OutputProfiles.GetSelectedProfile());
        break;
    }
}
COutputProfile* CSettingsPane::GetComboProfile()
{
    LPCTSTR profile_name = CBProfiles->GetItem();
    return OutputProfiles.GetProfile(profile_name);
}
void CSettingsPane::OnProfileComboChanged()
{
    COutputProfile* old_profile = OutputProfiles.GetSelectedProfile();
    COutputProfile* new_profile = GetComboProfile();
    if(new_profile == old_profile) return;

    PromtSaveCurrentProfile();

    OutputProfiles.SetSelectedProfile(new_profile);
    SetOutputProfile(new_profile);
    ResetProfileChanged();
}
void CSettingsPane::UpdateProfileCombo()
{
    OutputProfiles.Fill(CBProfiles);
    ToolBar.Invalidate();
}
void CSettingsPane::PromtSaveCurrentProfile()
{
    COutputProfile* old_profile = OutputProfiles.GetSelectedProfile();
    CString old_profile_name = OutputProfiles.GetSelectedProfileName();
    if(IsProfileChanged() && old_profile_name && false == old_profile_name.IsEmpty())
    {
        CString msg;
        msg.Format(_T("Do you want to save profile\n\"%s\" ?"), old_profile_name);
        if(IDOK == ::AfxMessageBox(msg, MB_OKCANCEL | MB_ICONEXCLAMATION | MB_DEFBUTTON1)) 
        {
            GetOutputProfile(old_profile);
            OutputProfiles.WriteProfile(theApp, old_profile_name);
        }
    }
}
void CSettingsPane::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);
	PGProfile.SetFocus();
}
void CSettingsPane::SetOutputProfile(const COutputProfile* profile)
{
    ASSERT(profile);

    LOGFONT lf;

    pgpBackgroundColor->SetColor(profile->BackgroundColor);
    pgpWriteHeader->SetValue(_variant_t(bool(profile->WriteHeader != 0)));

    profile->HeaderFont.Get(lf);
    pgpHeaderFont->SetFont(lf);
    pgpHeaderFontColor->SetColor(profile->HeaderFont.Color);
    BOOL enable = pgpWriteHeader->GetValue().boolVal;
    pgpHeaderFont->Enable(enable);
    pgpHeaderFontColor->Enable(enable);

    pgpFramesGridColumns->SetValue(_variant_t(long(profile->FrameColumns)));
    pgpFramesGridRows->SetValue(_variant_t(long(profile->FrameRows)));

    pgpOutputSizeMethod->SetItem(profile->OutputSizeMethod);

    pgpOutputSize->SetInt(profile->OutputImageSize);
    pgpOutputSize->Enable(profile->OutputSizeMethod != OUTPUT_IMAGE_WIDTH_BY_ORIGINAL_FRAME);

    pgpTimestampType->SetItem(profile->TimestampType);
    profile->TimestampFont.Get(lf);
    pgpTimestampFont->SetFont(lf);
    pgpTimestampFontColor->SetColor(profile->TimestampFont.Color);
    enable = (profile->TimestampType != TIMESTAMP_TYPE_DISABLED);
    pgpTimestampFont->Enable(enable);
    pgpTimestampFontColor->Enable(enable);

    //pgpOutputFileName->SetValue(COleVariant(profile->OutputFileName));
    pgpOutputFileFormat->SetItem(profile->OutputFileFormat);

    ProfileChanged = false;
    EnableWindow(TRUE);
}
void CSettingsPane::GetOutputProfile(COutputProfile* profile)
{
    //TODO: value checks
    //TODO: try catch ?
    ASSERT(profile);
    profile->BackgroundColor = pgpBackgroundColor->GetColor();

    profile->WriteHeader = pgpWriteHeader->GetValue().boolVal;
    LPLOGFONT header_logfont = pgpHeaderFont->GetLogFont();
    COLORREF header_font_clor = pgpHeaderFontColor->GetColor();
    profile->HeaderFont.Set(*header_logfont, header_font_clor);
    
    profile->FrameColumns = pgpFramesGridColumns->GetValue().intVal;
    profile->FrameRows = pgpFramesGridRows->GetValue().intVal;

    profile->OutputSizeMethod = static_cast<int>(pgpOutputSizeMethod->GetItem());
    profile->OutputImageSize = pgpOutputSize->GetInt();
    
    profile->TimestampType = static_cast<int>(pgpTimestampType->GetItem());
    LPLOGFONT timestamp_logfont = pgpTimestampFont->GetLogFont();
    COLORREF timestamp_font_clor = pgpTimestampFontColor->GetColor();
    profile->TimestampFont.Set(*timestamp_logfont, timestamp_font_clor);

    //const COleVariant& output_file_name = pgpOutputFileName->GetValue();
    //profile->OutputFileName = output_file_name;

    int output_format = static_cast<int>(pgpOutputFileFormat->GetItem());
    ASSERT(0 <= output_format && output_format <= OUTPUT_FILE_FORMAT_COUNT);
    if(output_format < 0 && OUTPUT_FILE_FORMAT_COUNT < output_format) output_format = OUTPUT_FILE_FORMAT_JPG;
    profile->OutputFileFormat = output_format;
}
