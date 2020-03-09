#include "stdafx.h"
#pragma hdrstop
#include "Resource.h"
#include "OutputProfile.h"
#include "VideoPreview.h"
#include "ProfilePane.h"
#include "MainFrm.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
//CPGPCombo
CPGPCombo::CPGPCombo(const CString& strName, const COleVariant& varValue, LPCTSTR lpszDescr /*= NULL*/) : 
    CMFCPropertyGridProperty(strName, varValue, lpszDescr) 
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
CProfilePane::CProfilePane() : CBProfileHeight(0)
{
}
CProfilePane::~CProfilePane()
{
}

BEGIN_MESSAGE_MAP(CProfilePane, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_COMMAND(ID_EXPAND_ALL, OnExpandAllProperties)
	ON_UPDATE_COMMAND_UI(ID_EXPAND_ALL, OnUpdateExpandAllProperties)
	ON_COMMAND(ID_SORTPROPERTIES, OnSortProperties)
	ON_UPDATE_COMMAND_UI(ID_SORTPROPERTIES, OnUpdateSortProperties)
	ON_COMMAND(ID_PROPERTIES1, OnProperties1)
	ON_UPDATE_COMMAND_UI(ID_PROPERTIES1, OnUpdateProperties1)
	ON_COMMAND(ID_PROPERTIES2, OnProperties2)
	ON_UPDATE_COMMAND_UI(ID_PROPERTIES2, OnUpdateProperties2)
	ON_WM_SETFOCUS()
	ON_WM_SETTINGCHANGE()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
//CProfilePane message handlers
void CProfilePane::AdjustLayout()
{
	if(GetSafeHwnd () == NULL || (AfxGetMainWnd() != NULL && AfxGetMainWnd()->IsIconic()))
		return;

	CRect rectClient;
	GetClientRect(rectClient);

    //TODO: need TBProfile?
	//int cyTlb = TBProfile.CalcFixedLayout(FALSE, TRUE).cy;
    int cyTlb = 0;
	//CBProfile.SetWindowPos(NULL, rectClient.left, rectClient.top, rectClient.Width(), CBProfileHeight, SWP_NOACTIVATE | SWP_NOZORDER);
	//TBProfile.SetWindowPos(NULL, rectClient.left, rectClient.top + CBProfileHeight, rectClient.Width(), cyTlb, SWP_NOACTIVATE | SWP_NOZORDER);
	PGProfile.SetWindowPos(NULL, rectClient.left, rectClient.top + CBProfileHeight + cyTlb, rectClient.Width(), rectClient.Height() -(CBProfileHeight+cyTlb), SWP_NOACTIVATE | SWP_NOZORDER);
}
int CProfilePane::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if(CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rectDummy;
	rectDummy.SetRectEmpty();

	//Create combo:
	//const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_BORDER | CBS_SORT | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	//if(!CBProfile.Create(dwViewStyle, rectDummy, this, 1))
	//{
	//	TRACE0("Failed to create Properties Combo \n");
	//	return -1;      // fail to create
	//}

 //   //TODO: init profiles
	//CBProfile.AddString(_T("<Current>"));
	//CBProfile.AddString(_T("Profile 1"));
 //   CBProfile.AddString(_T("Profile 2"));
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

	//TBProfile.Create(this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_PROPERTIES);
	//TBProfile.LoadToolBar(IDR_PROPERTIES, 0, 0, TRUE /* Is locked */);
	//TBProfile.CleanUpLockedImages();
	//TBProfile.LoadBitmap(IDB_PROPERTIES_HC, 0, 0, TRUE /* Locked */);

	//TBProfile.SetPaneStyle(TBProfile.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);
	//TBProfile.SetPaneStyle(TBProfile.GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));
	//TBProfile.SetOwner(this);

	////All commands will be routed via this control, not via the parent frame:
	//TBProfile.SetRouteCommandsViaFrame(FALSE);

	AdjustLayout();
	return 0;
}

void CProfilePane::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	AdjustLayout();
}

void CProfilePane::OnExpandAllProperties()
{
	PGProfile.ExpandAll();
}

void CProfilePane::OnUpdateExpandAllProperties(CCmdUI* /* pCmdUI */)
{
}

void CProfilePane::OnSortProperties()
{
	PGProfile.SetAlphabeticMode(!PGProfile.IsAlphabeticMode());
}

void CProfilePane::OnUpdateSortProperties(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(PGProfile.IsAlphabeticMode());
}

void CProfilePane::OnProperties1()
{
	// TODO: Add your command handler code here
}

void CProfilePane::OnUpdateProperties1(CCmdUI* /*pCmdUI*/)
{
	// TODO: Add your command update UI handler code here
}

void CProfilePane::OnProperties2()
{
	// TODO: Add your command handler code here
}

void CProfilePane::OnUpdateProperties2(CCmdUI* /*pCmdUI*/)
{
	// TODO: Add your command update UI handler code here
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

    //unused
    LOGFONT sample_logfont;
	CFont* sample_font = CFont::FromHandle((HFONT)::GetStockObject(DEFAULT_GUI_FONT));
	sample_font->GetLogFont(&sample_logfont);
	lstrcpy(sample_logfont.lfFaceName, _T("Arial"));

    //main
    pgpBackgroundColor = new CMFCPropertyGridColorProperty(_T("Background Color"), RGB(0, 0, 0), NULL, _T("TODO"));
    pgpBackgroundColor->EnableOtherButton(_T("Other..."));
    PGProfile.AddProperty(pgpBackgroundColor);
    pgpWriteHeader = new CMFCPropertyGridProperty(_T("Write Header"), (_variant_t)false, _T("TODO"));
    PGProfile.AddProperty(pgpWriteHeader);
   
    //header
    //TODO: header text
    //TODO: text color also can be defined in font dialog
    CMFCPropertyGridProperty* pgp_header = new CMFCPropertyGridProperty(_T("Header"));
    pgpHeaderFont = new CPGPFont(_T("Font"), sample_logfont, CF_EFFECTS | CF_SCREENFONTS, _T("TODO"));
    pgpHeaderFontColor = new CMFCPropertyGridColorProperty(_T("Font Color"), RGB(0, 0, 0), NULL, _T("TODO"));
    pgpHeaderFontColor->EnableOtherButton(_T("Other..."));
    pgp_header->AddSubItem(pgpHeaderFont);
    pgp_header->AddSubItem(pgpHeaderFontColor);
    PGProfile.AddProperty(pgp_header);

    //frames grid
    CMFCPropertyGridProperty* pgp_frames_grid = new CMFCPropertyGridProperty(_T("Frames Grid"));
    pgpFramesGridColumns = new CMFCPropertyGridProperty(_T("Columns"), (_variant_t) 4l, _T("TODO"));
	pgpFramesGridRows = new CMFCPropertyGridProperty(_T("Rows"), (_variant_t) 4l, _T("TODO"));  
	pgpFramesGridColumns->EnableSpinControl(TRUE, 1, 10);
    pgpFramesGridRows->EnableSpinControl(TRUE, 1, 10);
	pgp_frames_grid->AddSubItem(pgpFramesGridColumns);
    pgp_frames_grid->AddSubItem(pgpFramesGridRows);   
	PGProfile.AddProperty(pgp_frames_grid);
    pgpFramesGridColumns->AllowEdit(FALSE);
    pgpFramesGridRows->AllowEdit(FALSE);
    pgp_frames_grid->AllowEdit(FALSE); 
    //TODO: "write frame for each time interval" option

    //timestamp
    CMFCPropertyGridProperty* pgp_timestamp = new CMFCPropertyGridProperty(_T("Timestamp"));
    pgpTimestampType = new CPGPCombo(_T("Type"), _T(""), _T("TODO"));
    pgpTimestampType->AddItem(_T("Disabled"), TIMESTAMP_TYPE_DISABLED);
    pgpTimestampType->AddItem(_T("Top-Left"), TIMESTAMP_TYPE_TOP_LEFT);
    pgpTimestampType->AddItem(_T("Top-Center"), TIMESTAMP_TYPE_TOP_CENTER);
    pgpTimestampType->AddItem(_T("Top-Right"), TIMESTAMP_TYPE_TOP_RIGHT);
    pgpTimestampType->AddItem(_T("Bottom-Left"), TIMESTAMP_TYPE_BOTTOM_LEFT);
    pgpTimestampType->AddItem(_T("Bottom-Center"), TIMESTAMP_TYPE_BOTTOM_CENTER);
    pgpTimestampType->AddItem(_T("Bottom-Right"), TIMESTAMP_TYPE_BOTTOM_RIGHT);
    pgpTimestampType->AllowEdit(FALSE);

    pgpTimestampFont = new CPGPFont(_T("Font"), sample_logfont, CF_EFFECTS | CF_SCREENFONTS, _T("TODO"));
    pgpTimestampFontColor = new CMFCPropertyGridColorProperty(_T("Font Color"), RGB(0, 0, 0), NULL, _T("TODO"));
    pgpTimestampFontColor->EnableOtherButton(_T("Other..."));
    pgp_timestamp->AddSubItem(pgpTimestampType);
    pgp_timestamp->AddSubItem(pgpTimestampFont);
    pgp_timestamp->AddSubItem(pgpTimestampFontColor);
    PGProfile.AddProperty(pgp_timestamp);

    //output sizes
    CMFCPropertyGridProperty* pgp_output_image_size = new CMFCPropertyGridProperty(_T("Output Image Size"));
    pgpOutputSizeMethod = new CPGPCombo(_T("Size Value Meaning"), _T("Output image width"), _T("TODO"));
    pgpOutputSizeMethod->AddItem(_T("Use Original Frame Witdh"), OUTPUT_IMAGE_WIDTH_BY_ORIGINAL_FRAME_WIDTH);
    pgpOutputSizeMethod->AddItem(_T("Image width"), OUTPUT_IMAGE_WIDTH_AS_IS);
    pgpOutputSizeMethod->AddItem(_T("Frame Witdh"), OUTPUT_IMAGE_WIDTH_BY_FRAME_WIDTH);
    pgpOutputSizeMethod->AddItem(_T("Frame Height"), OUTPUT_IMAGE_WIDTH_BY_FRAME_HEIGHT);
    pgpOutputSizeMethod->AllowEdit(FALSE);
    pgpOutputSize = new CMFCPropertyGridProperty(_T("Size Value"), (_variant_t) 1000, _T("TODO"));
    pgp_output_image_size->AddSubItem(pgpOutputSizeMethod);
    pgp_output_image_size->AddSubItem(pgpOutputSize);
    pgpOutputSize->Enable(FALSE); //TODO:
    PGProfile.AddProperty(pgp_output_image_size);

    //output file
    CMFCPropertyGridProperty* pgp_output_file = new CMFCPropertyGridProperty(_T("Output File"));
    pgpOutputFileName = new CMFCPropertyGridProperty(_T("Name"), _T("%n"), _T("TODO"));
    pgpOutputFileFormat = new CPGPCombo(_T("Format"), _T("JPG"), _T("TODO"));    
    pgpOutputFileFormat->AddItem(_T("BMP"), OUTPUT_FILE_FORMAT_BMP);
    pgpOutputFileFormat->AddItem(_T("JPG"), OUTPUT_FILE_FORMAT_JPG);
    pgpOutputFileFormat->AddItem(_T("PNG"), OUTPUT_FILE_FORMAT_PNG);
    pgp_output_file->AddSubItem(pgpOutputFileName);
    pgp_output_file->AddSubItem(pgpOutputFileFormat);
    PGProfile.AddProperty(pgp_output_file);

    //TODO:
	//static const TCHAR szFilter[] = _T("Icon Files(*.ico)|*.ico|All Files(*.*)|*.*||");
	//pGroup3->AddSubItem(new CMFCPropertyGridFileProperty(_T("Icon"), TRUE, _T(""), _T("ico"), 0, szFilter, _T("Specifies the window icon")));
	//pGroup3->AddSubItem(new CMFCPropertyGridFileProperty(_T("Folder"), _T("c:\\")));
	//PGProfile.AddProperty(pGroup3);
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
    ASSERT(profile);

    LOGFONT lf;

    pgpBackgroundColor->SetColor(profile->BackgroundColor);
    pgpWriteHeader->SetValue(_variant_t(bool(profile->WriteHeader)));

    profile->HeaderFont.Get(lf);
    pgpHeaderFont->SetFont(lf);
    pgpHeaderFontColor->SetColor(profile->HeaderFont.Color);

    pgpFramesGridColumns->SetValue(_variant_t(long(profile->FrameColumns)));
    pgpFramesGridRows->SetValue(_variant_t(long(profile->FrameRows)));

    pgpOutputSizeMethod->SetItem(profile->OutputSizeMethod);
    pgpOutputSize->SetValue(_variant_t(int(profile->OutputImageSize)));

    pgpTimestampType->SetItem(profile->TimestampType);
    profile->TimestampFont.Get(lf);
    pgpTimestampFont->SetFont(lf);
    pgpTimestampFontColor->SetColor(profile->TimestampFont.Color);

    pgpOutputFileName->SetValue(COleVariant(profile->OutputFileName));
    pgpOutputFileFormat->SetItem(profile->OutputFileFormat);
}
void CProfilePane::GetOutputProfile(COutputProfile* profile)
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

    profile->OutputSizeMethod = pgpOutputSizeMethod->GetItem();
    profile->OutputImageSize = pgpOutputSize->GetValue().intVal;

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