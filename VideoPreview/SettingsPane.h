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

class CPGPNumberEdit : public CMFCPropertyGridProperty
{
public:
    CPGPNumberEdit(const CString& strName, const COleVariant& varValue, LPCTSTR lpszDescr = NULL, DWORD_PTR dwData = 0);
    void SetInt(int value);
    int GetInt();
};

class CPGPCombo : public CMFCPropertyGridProperty
{
public:
    CPGPCombo(const CString& strName, const COleVariant& varValue, LPCTSTR lpszDescr = NULL, DWORD_PTR dwData = 0);

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
class CSettingsPane : public CDockablePane
{
public:
    CMFCToolBarComboBoxButton* CBProfiles;
    CProfileToolBar ToolBar;
    CMFCPropertyGridCtrl PGProfile;

	CSettingsPane();
    virtual ~CSettingsPane();

	void AdjustLayout();

    void SetOutputProfile(const COutputProfile* profile);
    void GetOutputProfile(COutputProfile* profile);

    bool IsProfileChanged() const {return ProfileChanged;}
    void ResetProfileChanged() {ProfileChanged = false;}
    void PromtSaveCurrentProfile();

    void UpdateProfileCombo();
    void SetSettings();

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnUpdateUI(CCmdUI* pCmdUI);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
    afx_msg void OnProfileComboChanged();
    afx_msg LRESULT OnPropertyChanged(WPARAM wp, LPARAM lp);
	DECLARE_MESSAGE_MAP()

	CFont m_fntPropList;
    bool ProfileChanged;

    COutputProfile* GetComboProfile();

    void InitPropList();

    //output profile
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
    CPGPNumberEdit* pgpOutputSize;
    CMFCPropertyGridProperty* pgpOutputFileName;
    CPGPCombo* pgpOutputFileFormat;

    //common settings
    CMFCPropertyGridProperty* pgpOverwriteFiles;
    CMFCPropertyGridProperty* pgpSaveFileListOnExit;

    bool OnSettingsChanged(const DWORD_PTR property_id);
};

