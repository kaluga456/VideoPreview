#include "stdafx.h"
#pragma hdrstop
#include "OutputProfile.h"
#include "OutputProfileList.h"

COutputProfiles OutputProfiles;
//////////////////////////////////////////////////////////////////////////
//COutputProfiles
COutputProfiles::COutputProfiles() : SelectedProfile(NULL)
{
}
COutputProfile* COutputProfiles::GetProfile(LPCTSTR output_profile_name)
{
    if(NULL == output_profile_name) return NULL;
    COutputProfilesMap::const_iterator i = Profiles.find(output_profile_name);
    return (i == Profiles.end()) ? NULL : i->second.get();
}
LPCTSTR COutputProfiles::GetSelectedProfileName()
{
    if(NULL == SelectedProfile) return NULL;
    for(COutputProfilesMap::const_iterator i = Profiles.begin(); i != Profiles.end(); ++i)
    {
        if(SelectedProfile == i->second.get())
            return i->first;
    }
    return NULL;
}
COutputProfile* COutputProfiles::GetSelectedProfile()
{
    return SelectedProfile;
}
COutputProfile* COutputProfiles::SelectFirst()
{
    //select first if exists
    COutputProfilesMap::const_iterator i = Profiles.begin();
    SelectedProfile = (i == Profiles.end()) ? NULL : i->second.get();
    return SelectedProfile;
}
void COutputProfiles::SetSelectedProfile(LPCTSTR output_profile_name)
{
    if(NULL == output_profile_name)
    {
        SelectedProfile = NULL;
        return;
    }
    COutputProfilesMap::const_iterator i = Profiles.find(output_profile_name);
    SelectedProfile = (i == Profiles.end()) ? NULL : i->second.get();
}
void COutputProfiles::AddProfile(CWinAppEx& app, LPCTSTR profile_name, POutputProfile profile)
{
    if(NULL == profile) return;
    Profiles[profile_name] = profile;
    WriteProfile(app, profile_name);
    SelectedProfile = profile.get();
}
void COutputProfiles::DeleteSelectedProfile(CWinAppEx& app)
{
    if(NULL == SelectedProfile) return;

    CString selected_profile_name = GetSelectedProfileName();
    CString msg = _T("Do you want to delete profile\r\n\"") + CString(selected_profile_name) + _T("\" ?");
    const int result = ::AfxMessageBox(msg, MB_OKCANCEL | MB_ICONEXCLAMATION | MB_DEFBUTTON1);
    if(result != IDOK) return;

    DeleteProfile(app, selected_profile_name);
    COutputProfilesMap::iterator profile_i = Profiles.find(selected_profile_name);
    if(profile_i != Profiles.end())
        Profiles.erase(profile_i);

    profile_i = Profiles.begin();
    SelectedProfile = (profile_i == Profiles.end()) ? NULL : profile_i->second.get();
}
void COutputProfiles::Fill(CComboBox& combo_box, const COutputProfile* selected_profile /*= NULL*/)
{
    combo_box.ResetContent();
    int sel_index = 0;
    for(COutputProfilesMap::const_iterator profile_i = Profiles.begin(); profile_i != Profiles.end(); ++profile_i)
    {
        POutputProfile profile = profile_i->second;
        const int index = combo_box.AddString(profile_i->first);
        if(SelectedProfile == profile_i->second.get())
            sel_index = index;
    }
    combo_box.SetCurSel(sel_index);
}
void COutputProfiles::Fill(CMFCToolBarComboBoxButton* combo_box, const COutputProfile* selected_profile /*= NULL*/)
{
    combo_box->RemoveAllItems();
    int sel_index = 0; //DEFAULT_OUTPUT_PROFILE_NAME
    for(COutputProfilesMap::const_iterator profile_i = Profiles.begin(); profile_i != Profiles.end(); ++profile_i)
    {
        POutputProfile profile = profile_i->second;
        const int index = combo_box->AddItem(profile_i->first, reinterpret_cast<DWORD_PTR>(profile.get()));
        if(SelectedProfile == profile_i->second.get())
            sel_index = index;
    }
    combo_box->SelectItem(sel_index, FALSE);
}
void COutputProfiles::ReadProfiles(CWinAppEx& app)
{
    HKEY root_reg_key = app.GetAppRegistryKey();
    if(NULL == root_reg_key)
        return;

    CRegKey app_reg;
    if(ERROR_SUCCESS != app_reg.Open(root_reg_key, REG_PROFILES_SECTION))
        return;
    
    DWORD profile_name_size = 0;
    TCHAR profile_name[MAX_OUTPUT_PROFILE_NAME_SIZE + 1];
    profile_name[MAX_OUTPUT_PROFILE_NAME_SIZE] = 0;
    for(int value_index = 0; ; ++value_index)
    {
        profile_name_size = MAX_OUTPUT_PROFILE_NAME_SIZE + 1;
        LONG result = ::RegEnumValue(app_reg.m_hKey, value_index, profile_name, &profile_name_size, NULL, NULL, NULL, NULL);
        if(result != ERROR_SUCCESS) 
        {
            ASSERT(ERROR_NO_MORE_ITEMS == result);
            break;
        }

        if(*profile_name == 0)
        {
            ASSERT(0);
            continue;
        }

        POutputProfile profile(new COutputProfile);
        if(FALSE == app.GetSectionObject(REG_PROFILES_SECTION, profile_name, *(profile.get()))) continue;

        Profiles[profile_name] = profile;
    }
}
void COutputProfiles::WriteProfile(CWinAppEx& app, LPCTSTR profile_name)
{
    COutputProfile* profile = GetProfile(profile_name);
    if(NULL == profile)
        return;
    app.WriteSectionObject(REG_PROFILES_SECTION, profile_name, *profile);
}
void COutputProfiles::DeleteProfile(CWinAppEx& app, LPCTSTR profile_name)
{
    HKEY root_reg_key = app.GetAppRegistryKey();
    if(NULL == root_reg_key) 
        return;

    CRegKey app_reg;
    if(ERROR_SUCCESS != app_reg.Open(root_reg_key, REG_PROFILES_SECTION))
        return;

    app_reg.DeleteValue(profile_name);
}
CString COutputProfiles::GenerateProfileName() const
{
    CString profile_name;
    for(int index = 1;; ++index)
    {
        profile_name.Format(_T("Profile %u"), index);
        COutputProfilesMap::const_iterator profile_i = Profiles.find(profile_name);
        if(profile_i != Profiles.end()) continue;
        return profile_name;
    }
}
//////////////////////////////////////////////////////////////////////////