#include "stdafx.h"
#pragma hdrstop
#include "app_thread.h"
#include "Resource.h"
#include "OutputProfile.h"
#include "OutputProfileList.h"
#include "ScreenshotGenerator.h"
#include "ProcessingThread.h"
#include "VideoPreview.h"
#include "ProfilePane.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//profile property ids
enum
{
    ID_PROP_BACKGROUND_COLOR,

    ID_PROP_WRITE_HEADER,
    ID_PROP_HEADER_TEXT,
    ID_PROP_HEADER_FONT,
    ID_PROP_HEADER_FONT_COLOR,

    ID_PROP_FRAME_COLUMNS,
    ID_PROP_FRAME_ROWS,

    //TODO:
    ID_PROP_USE_TIME_INTERVAL,
    ID_PROP_FRAME_TIME_INTERVAL,

    ID_PROP_OUTPUT_SIZE_METHOD,
    ID_PROP_OUTPUT_IMAGE_SIZE,

    //TODO:
    ID_PROP_BORDER_PADDING,
    ID_PROP_FRAME_PADDING,

    ID_PROP_TIMESTAMP_TYPE,
    ID_PROP_TIMESTAMP_FONT,
    ID_PROP_TIMESTAMP_FONT_COLOR,

    ID_PROP_OUTPUT_FILE_NAME,
    ID_PROP_OUTPUT_FILE_FORMAT,

    PROFILE_PROP_COUNT
};

//TODO:
//profile property descriptions
LPCTSTR PROPERTY_DESCR[] =
{
    _T("TODO"), //ID_PROP_BACKGROUND_COLOR,
    _T("TODO"), //ID_PROP_WRITE_HEADER,
    _T("TODO"), //ID_PROP_HEADER_TEXT,

    _T("TODO"), //ID_PROP_HEADER_FONT,
    _T("TODO"), //ID_PROP_HEADER_FONT_COLOR,

    _T("TODO"), //ID_PROP_FRAME_COLUMNS,
    _T("TODO"), //ID_PROP_FRAME_ROWS,

    //TODO:
    _T("TODO"), //USE_TIME_INTERVAL,
    _T("TODO"), //FRAME_TIME_INTERVAL,

    _T("TODO"), //ID_PROP_OUTPUT_SIZE_METHOD,
    _T("TODO"), //ID_PROP_OUTPUT_IMAGE_SIZE,

    //TODO:
    _T("TODO"), //ID_PROP_BORDER_PADDING,
    _T("TODO"), //ID_PROP_FRAME_PADDING,

    _T("TODO"), //ID_PROP_TIMESTAMP_TYPE,
    _T("TODO"), //ID_PROP_TIMESTAMP_FONT,
    _T("TODO"), //ID_PROP_TIMESTAMP_FONT_COLOR,

    _T("TODO"), //ID_PROP_OUTPUT_FILE_NAME,
    _T("TODO"), //ID_PROP_OUTPUT_FILE_FORMAT,
};
static_assert(sizeof(PROPERTY_DESCR) / sizeof(LPCTSTR) == PROFILE_PROP_COUNT, "Invalid PROPERTY_DESCR size");
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
void CPGPCombo::AddItem(CString key, int value)
{
    ASSERT(false == key.IsEmpty());
    Items[key] = reinterpret_cast<void*>(value);
    AddOption(key);
}
int CPGPCombo::GetItem() const
{
    CString key = GetValue();
    void* value;
    return Items.Lookup(key, value) ? reinterpret_cast<int>(value) : -1;
}
void CPGPCombo::SetItem(int value)
{
    CString key;
    void* val = NULL;
    for(POSITION pos = Items.GetStartPosition(); pos;)
    {
        Items.GetNextAssoc(pos, key, val);
        if(value != reinterpret_cast<int>(val)) continue;        
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
//CProfilePane
BEGIN_MESSAGE_MAP(CProfilePane, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	ON_WM_SETTINGCHANGE()

    ON_CBN_SELCHANGE(ID_PROFILE_COMBO, OnProfileComboChanged)

    ON_UPDATE_COMMAND_UI(ID_PROFILE_COMBO, OnUpdateUI)
    ON_UPDATE_COMMAND_UI(ID_CMD_PROFILE_ADD, OnUpdateUI)
    ON_UPDATE_COMMAND_UI(ID_CMD_PROFILE_PREVIEW, OnUpdateUI)
    ON_UPDATE_COMMAND_UI(ID_CMD_PROFILE_SAVE, OnUpdateUI)
    ON_UPDATE_COMMAND_UI(ID_CMD_PROFILE_DELETE, OnUpdateUI)

    ON_REGISTERED_MESSAGE(AFX_WM_PROPERTY_CHANGED, OnProfilePropertyChanged)    
END_MESSAGE_MAP()
CProfilePane::CProfilePane()
{
}
CProfilePane::~CProfilePane()
{
}
void CProfilePane::AdjustLayout()
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
int CProfilePane::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if(CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rectDummy;
	rectDummy.SetRectEmpty();

	//Create combo:
	//const DWORD dwviewstyle = WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_BORDER | CBS_SORT | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	//if(!CBProfile.Create(dwviewstyle, rectDummy, this, 1))
	//{
	//	TRACE0("failed to create properties combo \n");
	//	return -1;      // fail to create
	//}

    //TODO: init profiles
	//CBProfile.AddString(_T("<Current>"));
	//CBProfile.AddString(_T("Profile 1"));
    //CBProfile.AddString(_T("Profile 2"));
	//CBProfile.SetCurSel(0);

	//CRect rectCombo;
	//CBProfile.GetClientRect(&rectCombo);
	//CBProfileHeight = rectCombo.Height();

	if(!PGProfile.Create(WS_VISIBLE | WS_CHILD, rectDummy, this, 2))
	{
		TRACE0("Failed to create Properties Grid\n");
		return -1;      // fail to create
	}

	InitPropList();

	ToolBar.Create(this, AFX_DEFAULT_TOOLBAR_STYLE, ID_TOOLBAR_SETTINGS);
	ToolBar.LoadToolBar(ID_TOOLBAR_SETTINGS, 0, 0, TRUE /* Is locked */);
	ToolBar.SetPaneStyle(ToolBar.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);
	ToolBar.SetPaneStyle(ToolBar.GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));
	ToolBar.SetOwner(this);

	//All commands will be routed via this control, not via the parent frame:
	ToolBar.SetRouteCommandsViaFrame(FALSE);
    CBProfiles = GetProfileCombo();
    ASSERT(CBProfiles);

	AdjustLayout();

    UpdateProfileCombo();

	return 0;
}
void CProfilePane::InitPropList()
{
	SetPropListFont();
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
    pgpBackgroundColor = new CMFCPropertyGridColorProperty(_T("Background Color"), RGB(0, 0, 0), NULL, _T("TODO"), ID_PROP_BACKGROUND_COLOR);
    pgpBackgroundColor->EnableOtherButton(_T("Other..."));
    pgp_profile_root->AddSubItem(pgpBackgroundColor);
    
   
    //header
    //TODO: header text
    //TODO: text color also can be defined in font dialog
    CMFCPropertyGridProperty* pgp_header = new CMFCPropertyGridProperty(_T("Header"));
    pgpWriteHeader = new CMFCPropertyGridProperty(_T("Write Header"), (_variant_t)false, _T("TODO"), ID_PROP_WRITE_HEADER);
    pgp_header->AddSubItem(pgpWriteHeader);
    pgpHeaderFont = new CPGPFont(_T("Font"), sample_logfont, CF_EFFECTS | CF_SCREENFONTS, _T("TODO"), ID_PROP_HEADER_FONT);
    pgpHeaderFontColor = new CMFCPropertyGridColorProperty(_T("Font Color"), RGB(0, 0, 0), NULL, _T("TODO"), ID_PROP_HEADER_FONT_COLOR);
    pgpHeaderFontColor->EnableOtherButton(_T("Other..."));
    pgp_header->AddSubItem(pgpHeaderFont);
    pgp_header->AddSubItem(pgpHeaderFontColor);
    pgp_profile_root->AddSubItem(pgp_header);

    //TODO:
    //ID_PROP_HEADER_TEXT

    //frames grid
    CMFCPropertyGridProperty* pgp_frames_grid = new CMFCPropertyGridProperty(_T("Frames Grid"));
    pgpFramesGridColumns = new CMFCPropertyGridProperty(_T("Columns"), (_variant_t) 4l, _T("TODO"), ID_PROP_FRAME_COLUMNS);
	pgpFramesGridRows = new CMFCPropertyGridProperty(_T("Rows"), (_variant_t) 4l, _T("TODO"), ID_PROP_FRAME_ROWS);  
	pgpFramesGridColumns->EnableSpinControl(TRUE, 1, 10);
    pgpFramesGridRows->EnableSpinControl(TRUE, 1, 10);
	pgp_frames_grid->AddSubItem(pgpFramesGridColumns);
    pgp_frames_grid->AddSubItem(pgpFramesGridRows); 
    pgp_profile_root->AddSubItem(pgp_frames_grid);
    pgpFramesGridColumns->AllowEdit(FALSE);
    pgpFramesGridRows->AllowEdit(FALSE);
    pgp_frames_grid->AllowEdit(FALSE); 
    //TODO: "write frame for each time interval" option
    //ID_PROP_USE_TIME_INTERVAL
    //ID_PROP_FRAME_TIME_INTERVAL

    //output sizes
    CMFCPropertyGridProperty* pgp_output_image_size = new CMFCPropertyGridProperty(_T("Output Image Size"));
    pgpOutputSizeMethod = new CPGPCombo(_T("Size Value Meaning"), _T("Output image width"), _T("TODO"), ID_PROP_OUTPUT_SIZE_METHOD);
    pgpOutputSizeMethod->AddItem(_T("Use Original Frame Witdh"), OUTPUT_IMAGE_WIDTH_BY_ORIGINAL_FRAME_WIDTH);
    pgpOutputSizeMethod->AddItem(_T("Image width"), OUTPUT_IMAGE_WIDTH_AS_IS);
    pgpOutputSizeMethod->AddItem(_T("Frame Witdh"), OUTPUT_IMAGE_WIDTH_BY_FRAME_WIDTH);
    pgpOutputSizeMethod->AddItem(_T("Frame Height"), OUTPUT_IMAGE_WIDTH_BY_FRAME_HEIGHT);
    pgpOutputSizeMethod->AllowEdit(FALSE);
    pgpOutputSize = new CPGPNumberEdit(_T("Size Value"), (_variant_t)_T("1000"), _T("TODO"), ID_PROP_OUTPUT_IMAGE_SIZE);
    pgp_output_image_size->AddSubItem(pgpOutputSizeMethod);
    pgp_output_image_size->AddSubItem(pgpOutputSize);
    //pgpOutputSize->Enable(FALSE); //TODO:
    pgp_profile_root->AddSubItem(pgp_output_image_size);
    //TODO:
    //ID_PROP_BORDER_PADDING
    //ID_PROP_FRAME_PADDING

    //timestamp
    CMFCPropertyGridProperty* pgp_timestamp = new CMFCPropertyGridProperty(_T("Timestamp"));
    pgpTimestampType = new CPGPCombo(_T("Type"), _T(""), _T("TODO"), ID_PROP_TIMESTAMP_TYPE);
    pgpTimestampType->AddItem(_T("Disabled"), TIMESTAMP_TYPE_DISABLED);
    pgpTimestampType->AddItem(_T("Top-Left"), TIMESTAMP_TYPE_TOP_LEFT);
    pgpTimestampType->AddItem(_T("Top-Center"), TIMESTAMP_TYPE_TOP_CENTER);
    pgpTimestampType->AddItem(_T("Top-Right"), TIMESTAMP_TYPE_TOP_RIGHT);
    pgpTimestampType->AddItem(_T("Bottom-Left"), TIMESTAMP_TYPE_BOTTOM_LEFT);
    pgpTimestampType->AddItem(_T("Bottom-Center"), TIMESTAMP_TYPE_BOTTOM_CENTER);
    pgpTimestampType->AddItem(_T("Bottom-Right"), TIMESTAMP_TYPE_BOTTOM_RIGHT);
    pgpTimestampType->AllowEdit(FALSE);
    pgpTimestampFont = new CPGPFont(_T("Font"), sample_logfont, CF_EFFECTS | CF_SCREENFONTS, _T("TODO"), ID_PROP_TIMESTAMP_FONT);
    pgpTimestampFontColor = new CMFCPropertyGridColorProperty(_T("Font Color"), RGB(0, 0, 0), NULL, _T("TODO"), ID_PROP_TIMESTAMP_FONT_COLOR);
    pgpTimestampFontColor->EnableOtherButton(_T("Other..."));
    pgp_timestamp->AddSubItem(pgpTimestampType);
    pgp_timestamp->AddSubItem(pgpTimestampFont);
    pgp_timestamp->AddSubItem(pgpTimestampFontColor);
    pgp_profile_root->AddSubItem(pgp_timestamp);

    //output file
    CMFCPropertyGridProperty* pgp_output_file = new CMFCPropertyGridProperty(_T("Output File"));
    pgpOutputFileName = new CMFCPropertyGridProperty(_T("Name"), _T("%n"), PROPERTY_DESCR[ID_PROP_OUTPUT_FILE_NAME], ID_PROP_OUTPUT_FILE_NAME);
    pgpOutputFileFormat = new CPGPCombo(_T("Format"), _T("JPG"), PROPERTY_DESCR[ID_PROP_OUTPUT_FILE_FORMAT], ID_PROP_OUTPUT_FILE_FORMAT);    
    pgpOutputFileFormat->AddItem(_T("BMP"), OUTPUT_FILE_FORMAT_BMP);
    pgpOutputFileFormat->AddItem(_T("JPG"), OUTPUT_FILE_FORMAT_JPG);
    pgpOutputFileFormat->AddItem(_T("PNG"), OUTPUT_FILE_FORMAT_PNG);
    pgp_output_file->AddSubItem(pgpOutputFileName);
    pgp_output_file->AddSubItem(pgpOutputFileFormat);
    pgp_profile_root->AddSubItem(pgp_output_file);

    PGProfile.AddProperty(pgp_profile_root);

    //TODO:
    //CMFCPropertyGridProperty* pgp_settings_root = new CMFCPropertyGridProperty(_T("Common Settings"));


    //TODO:
	//static const TCHAR szFilter[] = _T("Icon Files(*.ico)|*.ico|All Files(*.*)|*.*||");
	//pGroup3->AddSubItem(new CMFCPropertyGridFileProperty(_T("Icon"), TRUE, _T(""), _T("ico"), 0, szFilter, _T("Specifies the window icon")));
	//pGroup3->AddSubItem(new CMFCPropertyGridFileProperty(_T("Folder"), _T("c:\\")));
	//PGProfile.AddProperty(pGroup3);
}
void CProfilePane::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	AdjustLayout();
}
void CProfilePane::OnUpdateUI(CCmdUI* pCmdUI)
{
    switch(pCmdUI->m_nID)
    {
    case ID_PROFILE_COMBO:
    case ID_CMD_PROFILE_ADD: 
        pCmdUI->Enable(false == ::IsProcessing);
        break;
    case ID_CMD_PROFILE_PREVIEW:
    case ID_CMD_PROFILE_SAVE:
    case ID_CMD_PROFILE_DELETE:
        pCmdUI->Enable(false == ::IsProcessing && false == OutputProfiles.IsEmpty() && OutputProfiles.GetSelectedProfile());
        break;
    }
}
CMFCToolBarComboBoxButton* CProfilePane::GetProfileCombo()
{
    static int profile_combo_index = -1;
    if(profile_combo_index < 0) profile_combo_index = ToolBar.CommandToIndex(ID_PROFILE_COMBO);
    return static_cast<CMFCToolBarComboBoxButton*>(ToolBar.GetButton(profile_combo_index));
}
COutputProfile* CProfilePane::GetComboProfile()
{
    LPCTSTR profile_name = CBProfiles->GetItem();
    return OutputProfiles.GetProfile(profile_name);
}
void CProfilePane::OnProfileComboChanged()
{
    COutputProfile* old_profile = OutputProfiles.GetSelectedProfile();
    COutputProfile* new_profile = GetComboProfile();
    if(new_profile == old_profile) return;

    PromtSaveCurrentProfile();

    OutputProfiles.SetSelectedProfile(new_profile);
    SetOutputProfile(new_profile);
    ResetProfileChanged();
}
void CProfilePane::UpdateProfileCombo()
{
    OutputProfiles.Fill(CBProfiles);
    ToolBar.Invalidate();
}
void CProfilePane::PromtSaveCurrentProfile()
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
void CProfilePane::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);
	PGProfile.SetFocus();
}
void CProfilePane::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
	CDockablePane::OnSettingChange(uFlags, lpszSection);
	SetPropListFont();
}
void CProfilePane::SetPropListFont()
{
	::DeleteObject(m_fntPropList.Detach());

	LOGFONT lf;
	afxGlobalData.fontRegular.GetLogFont(&lf);

	NONCLIENTMETRICS info;
	info.cbSize = sizeof(info);

	afxGlobalData.GetNonClientMetrics(info);

	lf.lfHeight = info.lfMenuFont.lfHeight;
	lf.lfWeight = info.lfMenuFont.lfWeight;
	lf.lfItalic = info.lfMenuFont.lfItalic;

	m_fntPropList.CreateFontIndirect(&lf);

	//PGProfile.SetFont(&m_fntPropList);
	//CBProfile.SetFont(&m_fntPropList);
}
void CProfilePane::SetVSDotNetLook(BOOL bSet)
{
	PGProfile.SetVSDotNetLook(bSet);
	PGProfile.SetGroupNameFullWidth(bSet);
}
void CProfilePane::SetOutputProfile(const COutputProfile* profile)
{
    //TEST:
    TRACE1("GetOutputProfile: %s\r\n", profile->OutputFileName);

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
    pgpOutputSize->Enable(profile->OutputSizeMethod != OUTPUT_IMAGE_WIDTH_BY_ORIGINAL_FRAME_WIDTH);

    pgpTimestampType->SetItem(profile->TimestampType);
    profile->TimestampFont.Get(lf);
    pgpTimestampFont->SetFont(lf);
    pgpTimestampFontColor->SetColor(profile->TimestampFont.Color);
    enable = (profile->TimestampType != TIMESTAMP_TYPE_DISABLED);
    pgpTimestampFont->Enable(enable);
    pgpTimestampFontColor->Enable(enable);

    pgpOutputFileName->SetValue(COleVariant(profile->OutputFileName));
    pgpOutputFileFormat->SetItem(profile->OutputFileFormat);

    ProfileChanged = false;
    EnableWindow(TRUE);
}
void CProfilePane::GetOutputProfile(COutputProfile* profile)
{
    //TEST:
    TRACE1("GetOutputProfile: %s\r\n", profile->OutputFileName);

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

    profile->OutputSizeMethod = pgpOutputSizeMethod->GetItem();
    profile->OutputImageSize = pgpOutputSize->GetInt();
    
    profile->TimestampType = pgpTimestampType->GetItem();
    LPLOGFONT timestamp_logfont = pgpTimestampFont->GetLogFont();
    COLORREF timestamp_font_clor = pgpTimestampFontColor->GetColor();
    profile->TimestampFont.Set(*timestamp_logfont, timestamp_font_clor);

    const COleVariant& output_file_name = pgpOutputFileName->GetValue();
    profile->OutputFileName = output_file_name;

    int output_format = pgpOutputFileFormat->GetItem();
    ASSERT(0 <= output_format && output_format <= OUTPUT_FILE_FORMAT_COUNT);
    if(output_format < 0 && OUTPUT_FILE_FORMAT_COUNT < output_format) output_format = OUTPUT_FILE_FORMAT_JPG;
    profile->OutputFileFormat = output_format;
}
LRESULT CProfilePane::OnProfilePropertyChanged(WPARAM wp, LPARAM lp)
{
    //TODO:
    CMFCPropertyGridProperty* property = reinterpret_cast<CMFCPropertyGridProperty*>(lp);
    const DWORD_PTR property_id = property->GetData();
    switch(property_id)
    {
    case ID_PROP_BACKGROUND_COLOR: break;
    case ID_PROP_WRITE_HEADER:
    {
        const BOOL enable = pgpWriteHeader->GetValue().boolVal;
        pgpHeaderFont->Enable(enable);
        pgpHeaderFontColor->Enable(enable);
        break;
    }
    case ID_PROP_HEADER_TEXT: break;
    case ID_PROP_HEADER_FONT: break;
    case ID_PROP_HEADER_FONT_COLOR: break;

    case ID_PROP_FRAME_COLUMNS: break;
    case ID_PROP_FRAME_ROWS: break;

    //TODO:
    case ID_PROP_USE_TIME_INTERVAL: break;
    case ID_PROP_FRAME_TIME_INTERVAL: break;

    case ID_PROP_OUTPUT_SIZE_METHOD: 
    {
        const BOOL enable = (OUTPUT_IMAGE_WIDTH_BY_ORIGINAL_FRAME_WIDTH != pgpOutputSizeMethod->GetItem());
        pgpOutputSize->Enable(enable);
        break;
    }
    case ID_PROP_OUTPUT_IMAGE_SIZE: break;

    //TODO:
    case ID_PROP_BORDER_PADDING: break;
    case ID_PROP_FRAME_PADDING: break;

    case ID_PROP_TIMESTAMP_TYPE: 
    {
        const BOOL enable = (TIMESTAMP_TYPE_DISABLED != pgpTimestampType->GetItem());
        pgpTimestampFont->Enable(enable);
        pgpTimestampFontColor->Enable(enable);
        break;
    }
    case ID_PROP_TIMESTAMP_FONT: break;
    case ID_PROP_TIMESTAMP_FONT_COLOR: break;

    case ID_PROP_OUTPUT_FILE_NAME: break;
    case ID_PROP_OUTPUT_FILE_FORMAT: break;

    default:
        ASSERT(0);
    }

    ProfileChanged = true;
    return 0;
}