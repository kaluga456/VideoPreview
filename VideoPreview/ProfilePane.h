#pragma once

class CProfileToolBar : public CMFCToolBar
{
public:
	virtual void OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL bDisableIfNoHndler)
	{
		CMFCToolBar::OnUpdateCmdUI((CFrameWnd*) GetOwner(), bDisableIfNoHndler);
	}
	virtual BOOL AllowShowOnList() const { return FALSE; }
};

class CPGPCombo : public CMFCPropertyGridProperty
{
public:
    CPGPCombo(const CString& strName, const COleVariant& varValue, LPCTSTR lpszDescr = NULL);

    void AddItem(CString key, int value);
    int GetItem() const;
    void SetItem(int value);

private:
    CMapStringToPtr Items;
};
class CPGPFont : public CMFCPropertyGridFontProperty
{
public:
	CPGPFont(const CString& strName, LOGFONT& lf, DWORD dwFontDialogFlags = CF_EFFECTS | CF_SCREENFONTS, 
		LPCTSTR lpszDescr = NULL, DWORD_PTR dwData = 0, COLORREF color = (COLORREF)-1);

    void SetFont(const LOGFONT& logfont);
};
class CProfilePane : public CDockablePane
{
public:
	CProfilePane();
    virtual ~CProfilePane();

	void AdjustLayout();
	void SetVSDotNetLook(BOOL bSet);

    void SetOutputProfile(const COutputProfile* profile);
    void GetOutputProfile(COutputProfile* profile);

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnExpandAllProperties();
	afx_msg void OnUpdateExpandAllProperties(CCmdUI* pCmdUI);
	afx_msg void OnSortProperties();
	afx_msg void OnUpdateSortProperties(CCmdUI* pCmdUI);
	afx_msg void OnProperties1();
	afx_msg void OnUpdateProperties1(CCmdUI* pCmdUI);
	afx_msg void OnProperties2();
	afx_msg void OnUpdateProperties2(CCmdUI* pCmdUI);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);

	DECLARE_MESSAGE_MAP()

	int CBProfileHeight;
	CFont m_fntPropList;
	//CComboBox CBProfile; //TODO: need this?
	//CProfileToolBar TBProfile; //TODO: need this?
	CMFCPropertyGridCtrl PGProfile;

    //property controls
    CMFCPropertyGridColorProperty* pgpBackgroundColor;
    CMFCPropertyGridProperty* pgpWriteHeader;
    CPGPFont* pgpHeaderFont;
    CMFCPropertyGridColorProperty* pgpHeaderFontColor;
    CMFCPropertyGridProperty* pgpFramesGridColumns;
    CMFCPropertyGridProperty* pgpFramesGridRows;   
    CPGPCombo* pgpTimestampType;
    CPGPFont* pgpTimestampFont;
    CMFCPropertyGridColorProperty* pgpTimestampFontColor;
    CPGPCombo* pgpOutputSizeMethod;
    CMFCPropertyGridProperty* pgpOutputSize;
    CMFCPropertyGridProperty* pgpOutputFileName;
    CPGPCombo* pgpOutputFileFormat;

    void SetPropListFont();
    void InitPropList();
};

