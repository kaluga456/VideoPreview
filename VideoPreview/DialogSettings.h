#pragma once

class CDialogSettings : public CDialogEx
{
    DECLARE_MESSAGE_MAP()
public:
    CDialogSettings(CWnd* parent) : CDialogEx(IDD_SETTINGS, parent) {}

	BOOL OnInitDialog();
    void OnOK();
    void OnCancel();

    afx_msg void OnEnChangeEditSourceFileType();
    afx_msg void OnSourceFileTypesSelChange();
    afx_msg void OnBtnAddSourceFileType();
    afx_msg void OnBtnRemoveSourceFileType();
    afx_msg void OnBtnDefaultSourceFileType();

private:
    CSourceFileTypes SrcTypes;
    void SetButtonChek(int control_id, BOOL value = TRUE);
    BOOL GetButtonChek(int control_id);
    void UpdateSourceFileTypesListBox();
};

