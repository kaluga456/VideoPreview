#pragma once

//TODO: DoDataExchange
class CDialogSettings : public CDialogEx
{
public:
    CDialogSettings(CWnd* parent) : CDialogEx(IDD_OPTIONS, parent) {}

//	BEGIN_MSG_MAP(CDialogSettings)
//        COMMAND_HANDLER(IDC_BTN_ADD_SOURCE_FILE_TYPE, BN_CLICKED, OnBnClickedBtnAddSourceFileType)
//        COMMAND_HANDLER(IDC_BTN_REMOVE_SOURCE_FILE_TYPE, BN_CLICKED, OnBnClickedBtnRemoveSourceFileType)
//        COMMAND_HANDLER(IDC_CBTN_USE_SOURCE_FILE_LOCATION, BN_CLICKED, OnBnClickedCbtnUseSourceFileLocation)
//        COMMAND_HANDLER(IDC_BTN_DEFAULT_SOURCE_FILE_TYPE, BN_CLICKED, OnBnClickedBtnDefaultSourceFileType)
//        COMMAND_HANDLER(IDC_EDIT_SOURCE_FILE_TYPE, EN_CHANGE, OnEnChangeEditSourceFileType)
//        COMMAND_HANDLER(IDC_LIST_SOURCE_FILE_TYPES, LBN_SELCHANGE, OnLbnSelchangeListSourceFileTypes)
//    END_MSG_MAP()
//
	BOOL OnInitDialog();
    void OnOK();
    void OnCancel();
//    LRESULT OnBnClickedCbtnUseSourceFileLocation(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
//    LRESULT OnBnClickedBtnAddSourceFileType(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
//    LRESULT OnBnClickedBtnRemoveSourceFileType(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
//    LRESULT OnBnClickedBtnDefaultSourceFileType(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
//    LRESULT OnEnChangeEditSourceFileType(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
//    LRESULT OnLbnSelchangeListSourceFileTypes(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

public:
    DECLARE_MESSAGE_MAP()
    afx_msg void OnBtnOutputDirectory();
    afx_msg void OnSourceFileTypesSelChange();
    afx_msg void OnBtnAddSourceFileType();

private:
    CSourceFileTypes SourceFileTypes;
    void SetButtonChek(int control_id, BOOL value = TRUE);
    BOOL GetButtonChek(int control_id);
    void UpdateSourceFileTypesListBox();
};

