#include "stdafx.h"
#pragma hdrstop
#include "resource.h"
#include "VideoPreview.h"
#include "SourceFileTypes.h"
#include "OutputProfile.h"
#include "Settings.h"
#include "DialogSettings.h"

//void CDialogSettings::SetButtonChek(int control_id, BOOL value /*= TRUE*/)
//{
//    CDlgItem<CButton> button(m_hWnd, control_id);
//    button.SetCheck(value ? BST_CHECKED : BST_UNCHECKED);
//}
//BOOL CDialogSettings::GetButtonChek(int control_id)
//{
//    CDlgItem<CButton> button(m_hWnd, control_id);
//    return BST_CHECKED == button.GetCheck();
//}
BEGIN_MESSAGE_MAP(CDialogSettings, CDialogEx)
    ON_EN_CHANGE(IDC_EDIT_SOURCE_FILE_TYPE, &CDialogSettings::OnEnChangeEditSourceFileType) 
    ON_LBN_SELCHANGE(IDC_LIST_SOURCE_FILE_TYPES, &CDialogSettings::OnSourceFileTypesSelChange)
    ON_BN_CLICKED(IDC_BTN_ADD_SOURCE_FILE_TYPE, &CDialogSettings::OnBtnAddSourceFileType)
    ON_BN_CLICKED(IDC_BTN_REMOVE_SOURCE_FILE_TYPE, &CDialogSettings::OnBtnRemoveSourceFileType)
    ON_BN_CLICKED(IDC_BTN_DEFAULT_SOURCE_FILE_TYPE, &CDialogSettings::OnBtnDefaultSourceFileType)
END_MESSAGE_MAP()
BOOL CDialogSettings::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    //init controls
    CDlgItem<CEdit> edit_src_file(m_hWnd, IDC_EDIT_SOURCE_FILE_TYPE);
    edit_src_file.SetLimitText(SOURCE_FILE_TYPE_MAX_SIZE - 1);

    //TOOD:
    //const int use_explorer_context = theApp.GetInt(_T("UseExplorerContext"), FALSE);
    //SetButtonChek(IDC_CBTN_USE_EXPLORER_CONTEXT_MENU, use_explorer_context);

    //set app font for all controls
    HWND hwnd = ::GetTopWindow(this->GetSafeHwnd());
    while(hwnd)
    {
        UINT nID = ::GetDlgCtrlID(hwnd);
        SetWindowFont(hwnd, theApp.GetAppFont()->m_hObject, TRUE);
        hwnd = ::GetNextWindow(hwnd, GW_HWNDNEXT);
    }

    SrcTypes.SourceFileTypeList = ::SourceFileTypes.SourceFileTypeList;
    UpdateSourceFileTypesListBox();

	return TRUE;
}
void CDialogSettings::UpdateSourceFileTypesListBox()
{
    CDlgItem<CListBox> list_box(m_hWnd, IDC_LIST_SOURCE_FILE_TYPES);
    list_box.ResetContent();
    const CSourceFileTypeList& list = SrcTypes.SourceFileTypeList;
    for(CSourceFileTypeList::const_iterator i = list.begin(); i != list.end(); ++i)
    {
        ASSERT(i->IsValid());
        if(false == i->IsValid())
            continue;

        list_box.AddString(i->Get());
    }
}
void CDialogSettings::OnOK()
{
    //TODO:
//    Settings.UseExplorerContextMenu = GetButtonChek(IDC_CBTN_USE_EXPLORER_CONTEXT_MENU);

    ::SourceFileTypes.SourceFileTypeList = SrcTypes.SourceFileTypeList;
    EndDialog(IDOK);
}
void CDialogSettings::OnCancel()
{
    EndDialog(IDCANCEL);
}
void CDialogSettings::OnSourceFileTypesSelChange()
{
    CDlgItem<CListBox> list_box(m_hWnd, IDC_LIST_SOURCE_FILE_TYPES);
    GetDlgItem(IDC_BTN_REMOVE_SOURCE_FILE_TYPE)->EnableWindow(list_box.GetSelCount());
}
void CDialogSettings::OnEnChangeEditSourceFileType()
{
    TCHAR text[SOURCE_FILE_TYPE_MAX_SIZE];
    GetDlgItemText(IDC_EDIT_SOURCE_FILE_TYPE, text, SOURCE_FILE_TYPE_MAX_SIZE);

    CDlgItem<CButton> btn(m_hWnd, IDC_BTN_ADD_SOURCE_FILE_TYPE);
    btn.EnableWindow(text[0] != 0);
}
void CDialogSettings::OnBtnAddSourceFileType()
{
    TCHAR text[SOURCE_FILE_TYPE_MAX_SIZE];
    GetDlgItemText(IDC_EDIT_SOURCE_FILE_TYPE, text, SOURCE_FILE_TYPE_MAX_SIZE);
    if(true == SrcTypes.AddType(text))
        UpdateSourceFileTypesListBox();
    else
        ::AfxMessageBox(_T("Fail to add source file type. Unexpected character in file extension."), MB_OK | MB_ICONERROR);
}
void CDialogSettings::OnBtnRemoveSourceFileType()
{
    {
        CDlgItem<CListBox> list_box(m_hWnd, IDC_LIST_SOURCE_FILE_TYPES);

        const int count = list_box.GetCount();
        for(int i = 0; i < count; ++i)
        {
            if(0 == list_box.GetSel(i)) continue;

            TCHAR text[2 * SOURCE_FILE_TYPE_MAX_SIZE];
            list_box.GetText(i, text);
            SrcTypes.RemoveType(text);
        }
    }

    UpdateSourceFileTypesListBox();

    CDlgItem<CListBox> list_box(m_hWnd, IDC_LIST_SOURCE_FILE_TYPES);
    CDlgItem<CButton> btn(m_hWnd, IDC_BTN_REMOVE_SOURCE_FILE_TYPE);
    btn.EnableWindow(list_box.GetSelCount());
}
void CDialogSettings::OnBtnDefaultSourceFileType()
{
    if(IDCANCEL == ::AfxMessageBox(_T("Restore default source file types?"), MB_OKCANCEL | MB_DEFBUTTON1 | MB_ICONWARNING))
        return;

    SrcTypes.SetString(DEFAULT_SOURCE_FILE_TYPES);
    UpdateSourceFileTypesListBox();
}