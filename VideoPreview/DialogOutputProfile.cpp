#include "stdafx.h"
#pragma hdrstop
#include "resource.h"
#include "OutputProfile.h"
#include "OutputProfileList.h"
#include "DialogOutputProfile.h"
#include "VideoPreview.h"

BEGIN_MESSAGE_MAP(CDialogOutputProfile, CDialogEx)
END_MESSAGE_MAP()

IMPLEMENT_DYNAMIC(CDialogOutputProfile, CDialogEx)

CDialogOutputProfile::CDialogOutputProfile(CWnd* parent, bool new_profile /*= false*/) : 
    CDialogEx(IDD_OUTPUT_PROFILE, parent), 
    IsNewProfile(new_profile)
{
}
CDialogOutputProfile::~CDialogOutputProfile()
{  
}
BOOL CDialogOutputProfile::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    SetWindowText(IsNewProfile ? _T("Add New Output Profile") : _T("Save Output Profile As"));

    IsCopyFrom = IsNewProfile;
    if(IsNewProfile)
    {
        //init IDC_EDIT_OUTPUT_PROFILE_NAME
        //TODO: generate new profile name
        CDlgItem<CEdit> profile_edit(m_hWnd, IDC_EDIT_OUTPUT_PROFILE_NAME);
        profile_edit.SetLimitText(MAX_OUTPUT_PROFILE_NAME_SIZE);
        ProfileName = OutputProfiles.GenerateProfileName();

        //init IDC_CBTN_COPY_FROM_OUTPUT_PROFILE
        CheckDlgButton(IDC_CBTN_COPY_FROM_OUTPUT_PROFILE, BST_CHECKED);

        //init IDC_COMBO_OUTPUT_PROFILES
        CDlgItem<CComboBox> profiles_combo(m_hWnd, IDC_COMBO_OUTPUT_PROFILES);
        OutputProfiles.Fill(profiles_combo);

        ::EnableWindow(::GetDlgItem(m_hWnd, IDC_CBTN_COPY_FROM_OUTPUT_PROFILE), false == OutputProfiles.IsEmpty());
        ::EnableWindow(::GetDlgItem(m_hWnd, IDC_COMBO_OUTPUT_PROFILES), false == OutputProfiles.IsEmpty());   
    }
    else
    {
        //init IDC_EDIT_OUTPUT_PROFILE_NAME
        ProfileName = OutputProfiles.GetSelectedProfileName();

        //TODO: generate new profile name
        if(ProfileName.IsEmpty()) ProfileName = OutputProfiles.GenerateProfileName();

        ::EnableWindow(::GetDlgItem(m_hWnd, IDC_CBTN_COPY_FROM_OUTPUT_PROFILE), FALSE);
        ::EnableWindow(::GetDlgItem(m_hWnd, IDC_COMBO_OUTPUT_PROFILES), FALSE);
    }

    UpdateData(FALSE);

    //NOTE: The application can return 0 only if it has explicitly set the input focus to one of the controls in the dialog box.
    return TRUE;
}
void CDialogOutputProfile::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

    DDX_Text(pDX, IDC_EDIT_OUTPUT_PROFILE_NAME, ProfileName);
    DDX_Check(pDX, IDC_CBTN_COPY_FROM_OUTPUT_PROFILE, IsCopyFrom);
}
void CDialogOutputProfile::OnOK()
{
    UpdateData(TRUE);

    //check profile name
    ProfileName.Trim();
    if(ProfileName.IsEmpty())
    {
        AfxMessageBox(_T("Profile Name Is Empty"), MB_OK | MB_ICONWARNING);
        return;
    }
 
    COutputProfile* profile_to_save = OutputProfiles.GetProfile(ProfileName);

    //for new profile check whether it`s name already used
    if(IsNewProfile && profile_to_save)
    {
        CString msg;
        msg.Format(_T("Profile \"%s\" already exists. Do you want to overwrite it?"), ProfileName);
        if(IDOK != ::AfxMessageBox(msg, MB_OKCANCEL | MB_ICONWARNING | MB_DEFBUTTON1))
            return;
    }

    if(IsCopyFrom)
    {
        CDlgItem<CComboBox> profiles_combo(m_hWnd, IDC_COMBO_OUTPUT_PROFILES);
        const int index = profiles_combo.GetCurSel();
        if(index >= 0)
            profiles_combo.GetLBText(index, CopyFrom);
    }

    EndDialog(IDOK);
}
