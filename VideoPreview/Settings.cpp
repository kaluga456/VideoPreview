#include "stdafx.h"
#pragma hdrstop
#include "SourceFileTypes.h"
#include "Settings.h"
#include "OutputProfile.h"
#include "OutputProfileList.h"
#include "ProcessingItem.h"
#include "VideoPreview.h"

extern CProcessingItemList ProcessingItemList;
CSettings Settings;

//deafults
const int DEFAULT_LAYOUT_X = CW_USEDEFAULT;
const int DEFAULT_LAYOUT_Y = CW_USEDEFAULT;
const int DEFAULT_LAYOUT_WIDTH = CW_USEDEFAULT;
const int DEFAULT_LAYOUT_HEIGHT = CW_USEDEFAULT;
const int DEFAULT_LAYOUT_WINDOWS_TATE = SW_SHOWDEFAULT;
const int DEFAULT_USE_EXPLORER_CONTEXT_MENU = FALSE;
const int DEFAULT_USE_SOURCE_FILE_LOCATION = TRUE;
const int DEFAULT_OVERWRITE_OUTPUT_FILES = FALSE;
const int DEFAULT_ACTION_ON_ERROR = CSettings::ACTION_ON_ERROR_SKIP;
const int DEFAULT_SAVE_FILE_LIST_ON_EXIT = true;
const int DEFAULT_FILE_LIST_COLUMN_WIDTH = 100;
const int DEFAULT_FILE_LIST_SORTED_COLUMN = 0;
const int DEFAULT_FILE_LIST_SORT_ORDER = 0; //TODO: check
const int DEFAULT_PROFILE_PANE_DESC_ROWS = 4; //TODO: check
const int DEFAULT_PROFILE_PANE_WIDTH = 200; //TODO: check

//CProcessingItemListSerial - CProcessingItemList serializator
class CProcessingItemListSerial : public CObject
{
     DECLARE_SERIAL(CProcessingItemListSerial)

private:
    CProcessingItemList* ProcessingItemList;
    CProcessingItemListSerial() : ProcessingItemList(NULL) {}

public:  
    explicit CProcessingItemListSerial(CProcessingItemList* item_list) : ProcessingItemList(item_list) {}

    void Serialize(CArchive& archive);
};
IMPLEMENT_SERIAL(CProcessingItemListSerial, CObject, VERSIONABLE_SCHEMA | 1)

void CProcessingItemListSerial::Serialize(CArchive& archive)
{
    CObject::Serialize(archive);

    ASSERT(ProcessingItemList);
    if(NULL == ProcessingItemList) return;

    if(archive.IsStoring())
    {
        const int count = ProcessingItemList->size();
        archive << count;
        for(CProcessingItemList::const_iterator i = ProcessingItemList->begin(); i != ProcessingItemList->end(); ++i)
        {
            CProcessingItem* pi = i->second.get();
            archive << pi->State;
            archive << pi->SourceFileName;
            archive << pi->ResultString;
        }
    }
    else
    {
        ProcessingItemList->clear();

        int count = 0;
        archive >> count;

        int state = 0;
        CString src_file_name;
        CString result_string;
        for(int i = 0; i < count; ++i)
        {
            archive >> state;
            archive >> src_file_name;
            archive >> result_string;

            PProcessingItem pi(new CProcessingItem(state, src_file_name, result_string));
            (*ProcessingItemList)[pi.get()] = pi;
        }
    }
}
bool CSettings::Read()
{
    //main
    OutputDirectory = theApp.GetString(_T("OutputPath")); //TODO: defaults to "My Documents"
    UseSourceFileLocation = theApp.GetInt(_T("UseSourceLocation"), DEFAULT_USE_SOURCE_FILE_LOCATION);
    OverwriteOutputFiles = theApp.GetInt(_T("OverwriteFiles"), DEFAULT_OVERWRITE_OUTPUT_FILES);

    CString s = theApp.GetString(_T("SourceFileTypes"), DEFAULT_SOURCE_FILE_TYPES);
    SourceFileTypes.SetString(s);

    ActionOnError = theApp.GetInt(_T("ActionOnError"), DEFAULT_ACTION_ON_ERROR);
    SaveFileListOnExit = theApp.GetInt(_T("SaveSourceFileList"), DEFAULT_SAVE_FILE_LIST_ON_EXIT);
    UseExplorerContextMenu = theApp.GetInt(_T("ExplorerContextMenu"), DEFAULT_USE_EXPLORER_CONTEXT_MENU);

    //layout
    ColumnWidth1 = theApp.GetSectionInt(_T("Layout"), _T("ColumnWidth1"), DEFAULT_FILE_LIST_COLUMN_WIDTH);
    ColumnWidth2 = theApp.GetSectionInt(_T("Layout"), _T("ColumnWidth2"), DEFAULT_FILE_LIST_COLUMN_WIDTH);
    ColumnWidth3 = theApp.GetSectionInt(_T("Layout"), _T("ColumnWidth3"), DEFAULT_FILE_LIST_COLUMN_WIDTH);
    SortedColumn = theApp.GetSectionInt(_T("Layout"), _T("SortedColumn"), DEFAULT_FILE_LIST_SORTED_COLUMN);
    SortOrder = theApp.GetSectionInt(_T("Layout"), _T("SortOrder"), DEFAULT_FILE_LIST_SORT_ORDER);
    ProfilePaneWidth = theApp.GetSectionInt(_T("Layout"), _T("ProfilePaneWidth"), DEFAULT_PROFILE_PANE_WIDTH);

    //output profiles
    OutputProfiles.ReadProfiles(theApp);
    CString selected_profile = theApp.GetString(_T("SelectedProfile"));
    OutputProfiles.SetSelectedProfile(selected_profile);
  
    //processing items
    if(SaveFileListOnExit)
    {
        CProcessingItemListSerial pils(&ProcessingItemList);
        theApp.GetObject(_T("Items"), pils);
    }

    return true;
}
bool CSettings::Write()
{
    //main
    theApp.WriteString(_T("OutputPath"), OutputDirectory);
    theApp.WriteInt(_T("UseSourceLocation"), UseSourceFileLocation);
    theApp.WriteInt(_T("OverwriteFiles"), OverwriteOutputFiles);
    theApp.WriteString(_T("SourceFileTypes"), SourceFileTypes.GetString());
    theApp.WriteInt(_T("ActionOnError"), ActionOnError);
    theApp.WriteInt(_T("SaveSourceFileList"), SaveFileListOnExit);
    theApp.WriteInt(_T("ExplorerContextMenu"), UseExplorerContextMenu);

    //layout
    theApp.WriteSectionInt(_T("Layout"), _T("ColumnWidth1"), ColumnWidth1);
    theApp.WriteSectionInt(_T("Layout"), _T("ColumnWidth2"), ColumnWidth2);
    theApp.WriteSectionInt(_T("Layout"), _T("ColumnWidth3"), ColumnWidth3);
    theApp.WriteSectionInt(_T("Layout"), _T("SortedColumn"), SortedColumn);
    theApp.WriteSectionInt(_T("Layout"), _T("SortOrder"), SortOrder);
    theApp.WriteSectionInt(_T("Layout"), _T("ProfilePaneWidth"), ProfilePaneWidth);

    //output profiles
    LPCTSTR profile_name = OutputProfiles.GetSelectedProfileName();
    theApp.WriteString(_T("SelectedProfile"), profile_name ? profile_name : _T(""));

    //processing items
    if(SaveFileListOnExit)
    {
        CProcessingItemListSerial pils(&ProcessingItemList);
        theApp.WriteObject(_T("Items"), pils);
    }

    return true;
} 
