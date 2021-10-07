#include "stdafx.h"
#pragma hdrstop
#include "resource.h"
#include "VideoPreview.h"
#include "SourceFileTypes.h"
#include "OutputProfile.h"
#include "Options.h"
#include "DialogSettings.h"

void CDialogSettings::SetButtonChek(int control_id, BOOL value /*= TRUE*/)
{
    CDlgItem<CButton> button(m_hWnd, control_id);
    button.SetCheck(value ? BST_CHECKED : BST_UNCHECKED);
}
BOOL CDialogSettings::GetButtonChek(int control_id)
{
    CDlgItem<CButton> button(m_hWnd, control_id);
    return BST_CHECKED == button.GetCheck();
}
void CDialogSettings::UpdateSourceFileTypesListBox()
{
    CDlgItem<CListBox> list_box(m_hWnd, IDC_LIST_SOURCE_FILE_TYPES);
    list_box.ResetContent();
    const CSourceFileTypeList& list = SourceFileTypes.SourceFileTypeList;
    for(CSourceFileTypeList::const_iterator i = list.begin(); i != list.end(); ++i)
    {
        ASSERT(i->IsValid());
        if(false == i->IsValid())
            continue;

        list_box.AddString(i->Get());
    }
}

BOOL CDialogSettings::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    //TODO: all reg names and structure to COptions
    //read options
    const int use_src_file_location = theApp.GetInt(_T("UseSrcFileLocation"), TRUE);
    const int overwrite_output = theApp.GetInt(_T("OverwriteOutputFiles"), FALSE);
    const int action_on_error = theApp.GetInt(_T("ActionOnError"), COptions::ACTION_ON_ERROR_SKIP);
    const int use_explorer_context = theApp.GetInt(_T("UseExplorerContext"), FALSE);
    const int save_videofile_list = theApp.GetInt(_T("SaveVideoFileList"), TRUE);
    CString output_loc = theApp.GetString(_T("OutputDir"));
    CString src_file_types = theApp.GetString(_T("SourceFileTypes"), DEFAULT_SOURCE_FILE_TYPES);

    //init controls
    SetButtonChek(IDC_CBTN_USE_SOURCE_FILE_LOCATION, use_src_file_location);
    GetDlgItem(IDC_BTN_OUTPUT_DIRECTORY)->EnableWindow(BST_CHECKED == use_src_file_location);
    CWnd* edit_output_loc = GetDlgItem(IDC_EDIT_OUTPUT_LOCATION);
    edit_output_loc->EnableWindow(BST_CHECKED == use_src_file_location);
    edit_output_loc->SetWindowText(output_loc);

    CDlgItem<CEdit> edit_src_file(m_hWnd, IDC_EDIT_SOURCE_FILE_TYPE);
    edit_src_file.SetLimitText(SOURCE_FILE_TYPE_MAX_SIZE - 1);

    SetButtonChek(IDC_CBTN_OVERWRITE_OUTPUT_FILES, overwrite_output);

    if(COptions::ACTION_ON_ERROR_SKIP == action_on_error)
        SetButtonChek(IDC_RBTN_ACTION_ON_ERROR_SKIP);
    else if(COptions::ACTION_ON_ERROR_STOP == action_on_error)
        SetButtonChek(IDC_RBTN_ACTION_ON_ERROR_STOP);
    else
        SetButtonChek(IDC_RBTN_ACTION_ON_ERROR_PROMT);

    SourceFileTypes.SetString(src_file_types);
    UpdateSourceFileTypesListBox();

    SetButtonChek(IDC_CBTN_USE_EXPLORER_CONTEXT_MENU, use_explorer_context);
    SetButtonChek(IDC_CBTN_SAVE_FILE_LIST, save_videofile_list);

	//CenterWindow(GetParent());
	return TRUE;
}
void CDialogSettings::OnOK()
{
    //TODO:
    CString output_dir;
    GetDlgItemText(IDC_EDIT_OUTPUT_LOCATION, output_dir);
    theApp.WriteString(_T("OutputDir"), output_dir);
//    Options.OutputDirectory = output_directory;
//    Options.UseSourceFileLocation = GetButtonChek(IDC_CBTN_USE_SOURCE_FILE_LOCATION);
//    Options.OverwriteOutputFiles = GetButtonChek(IDC_CBTN_OVERWRITE_OUTPUT_FILES);
//
//    if(GetButtonChek(IDC_RBTN_ACTION_ON_ERROR_SKIP))
//        Options.ActionOnError = COptions::ACTION_ON_ERROR_SKIP;
//    else if(GetButtonChek(IDC_RBTN_ACTION_ON_ERROR_STOP))
//        Options.ActionOnError = COptions::ACTION_ON_ERROR_STOP;
//    else
//        Options.ActionOnError = COptions::ACTION_ON_ERROR_PROMT;
//
//    SourceFileTypes = SourceFileTypes;
//    Options.UseExplorerContextMenu = GetButtonChek(IDC_CBTN_USE_EXPLORER_CONTEXT_MENU);
//    Options.SaveFileListOnExit = GetButtonChek(IDC_CBTN_SAVE_FILE_LIST);
    EndDialog(IDOK);
}
void CDialogSettings::OnCancel()
{
    //TODO:
    EndDialog(IDCANCEL);
}
//LRESULT CDialogSettings::OnBnClickedCbtnUseSourceFileLocation(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//{
//    const BOOL enable = FALSE == GetButtonChek(IDC_CBTN_USE_SOURCE_FILE_LOCATION);
//    CEdit(GetDlgItem(IDC_EDIT_OUTPUT_LOCATION)).EnableWindow(enable);
//    CButton(GetDlgItem(IDC_BTN_OUTPUT_DIRECTORY)).EnableWindow(enable);
//    return 0;
//}
//LRESULT CDialogSettings::OnBnClickedBtnOutputDirectory(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//{
//    //TODO: how to set initial path?
//    TCHAR path[MAX_PATH];
//    BROWSEINFO bi;
//    memset(&bi, 0, sizeof(bi));
//    bi.hwndOwner = m_hWnd;
//    bi.lpszTitle = _T("Select directory for output files");
//    bi.pszDisplayName = path;
//    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_USENEWUI | BIF_SHAREABLE;
//
//    LPCITEMIDLIST pidl = SHBrowseForFolder(&bi);
//    if(NULL == pidl) 
//        return 0; //canceled
//
//    if(FALSE == SHGetPathFromIDList(pidl, path))
//	    return 0; //fail
//
//    SetDlgItemText(IDC_EDIT_OUTPUT_LOCATION, path);
//    return 0;
//}
//LRESULT CDialogSettings::OnBnClickedBtnAddSourceFileType(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//{

//}
//LRESULT CDialogSettings::OnBnClickedBtnRemoveSourceFileType(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//{
//    CListBox list_box(GetDlgItem(IDC_LIST_SOURCE_FILE_TYPES));
//
//    int items[1000];
//    const int count = list_box.GetSelItems(1000, items);
//    if(0 == count) return 0;
//    for(int i = 0; i < count; ++i)
//    {
//        TCHAR text[2 * SOURCE_FILE_TYPE_MAX_SIZE];
//        list_box.GetText(items[i], text);
//        SourceFileTypes.RemoveType(text);
//    }
//    UpdateSourceFileTypesListBox();
//    CButton(GetDlgItem(IDC_BTN_REMOVE_SOURCE_FILE_TYPE)).EnableWindow(list_box.GetSelCount());
//    return 0;
//}
//LRESULT CDialogSettings::OnBnClickedBtnDefaultSourceFileType(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//{
//    if(IDCANCEL == MessageBox(_T("Set default source file types?"), _T("Warning"), MB_OKCANCEL | MB_DEFBUTTON2 | MB_ICONWARNING))
//        return 0;
//    SourceFileTypes.SetString(DEFAULT_SOURCE_FILE_TYPES);
//    UpdateSourceFileTypesListBox();
//    return 0;
//}
//LRESULT CDialogSettings::OnEnChangeEditSourceFileType(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//{
//    TCHAR text[SOURCE_FILE_TYPE_MAX_SIZE];
//    GetDlgItemText(IDC_EDIT_SOURCE_FILE_TYPE, text, SOURCE_FILE_TYPE_MAX_SIZE);
//    CButton(GetDlgItem(IDC_BTN_ADD_SOURCE_FILE_TYPE)).EnableWindow(text[0] != 0);
//    return 0;
//}
BEGIN_MESSAGE_MAP(CDialogSettings, CDialogEx)
    ON_BN_CLICKED(IDC_BTN_OUTPUT_DIRECTORY, &CDialogSettings::OnBtnOutputDirectory)
    ON_LBN_SELCHANGE(IDC_LIST_SOURCE_FILE_TYPES, &CDialogSettings::OnSourceFileTypesSelChange)
    ON_BN_CLICKED(IDC_BTN_ADD_SOURCE_FILE_TYPE, &CDialogSettings::OnBtnAddSourceFileType)
END_MESSAGE_MAP()


void CDialogSettings::OnBtnOutputDirectory()
{
    //TODO: how to set initial path?
    TCHAR path[MAX_PATH];
    BROWSEINFO bi;
    memset(&bi, 0, sizeof(bi));
    bi.hwndOwner = m_hWnd;
    bi.lpszTitle = _T("Select directory for output files");
    bi.pszDisplayName = path;
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_USENEWUI | BIF_SHAREABLE;

    LPCITEMIDLIST pidl = SHBrowseForFolder(&bi);
    if(NULL == pidl) 
        return; //canceled

    if(FALSE == SHGetPathFromIDList(pidl, path))
	    return; //fail

    SetDlgItemText(IDC_EDIT_OUTPUT_LOCATION, path);
    return;
}


void CDialogSettings::OnSourceFileTypesSelChange()
{
    CDlgItem<CListBox> list_box(m_hWnd, IDC_LIST_SOURCE_FILE_TYPES);
    GetDlgItem(IDC_BTN_REMOVE_SOURCE_FILE_TYPE)->EnableWindow(list_box.GetSelCount());
}


void CDialogSettings::OnBtnAddSourceFileType()
{
    //TODO:
//    TCHAR text[SOURCE_FILE_TYPE_MAX_SIZE];
//    GetDlgItemText(IDC_EDIT_SOURCE_FILE_TYPE, text, SOURCE_FILE_TYPE_MAX_SIZE);
//    if(true == SourceFileTypes.AddType(text))
//        UpdateSourceFileTypesListBox();
//    else
//        MessageBox(_T("Fail to add source file type. Unexpected character in file extension."), _T("Error"), MB_OK | MB_ICONERROR);
//    return 0;
}
