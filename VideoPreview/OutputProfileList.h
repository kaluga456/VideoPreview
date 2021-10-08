#pragma once

const int MAX_OUTPUT_PROFILE_NAME_SIZE = 64;
LPCTSTR const REG_PROFILES_SECTION = _T("Profiles");
typedef std::map<CString, POutputProfile> COutputProfilesMap;

//output profiles
class COutputProfiles
{
public:
    COutputProfiles();

    COutputProfile* GetProfile(LPCTSTR output_profile_name);
    bool IsEmpty() const {return Profiles.empty();}

    //selected profile
    COutputProfile* GetSelectedProfile();
    LPCTSTR GetSelectedProfileName();
    COutputProfile* SelectFirst(); //select first if exists
    void SetSelectedProfile(LPCTSTR output_profile_name);
    void SetSelectedProfile(COutputProfile* output_profile_name) {SelectedProfile = output_profile_name;}   

    //
    void AddProfile(CWinAppEx& app, LPCTSTR profile_name, POutputProfile profile);
    void DeleteSelectedProfile(CWinAppEx& app);

    //GUI
    void Fill(CComboBox& combo_box, const COutputProfile* selected_profile = NULL);
    void Fill(CMFCToolBarComboBoxButton* combo_box, const COutputProfile* selected_profile = NULL);

    //serialization
    void ReadProfiles(CWinAppEx& app);
    void WriteProfile(CWinAppEx& app, LPCTSTR profile_name);
    void DeleteProfile(CWinAppEx& app, LPCTSTR profile_name);
    
    CString GenerateProfileName() const;

private:
    COutputProfilesMap Profiles;
    COutputProfile* SelectedProfile; //current profile
};

extern COutputProfiles OutputProfiles;