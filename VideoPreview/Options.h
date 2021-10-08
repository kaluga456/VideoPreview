#pragma once

//COptions - main settings
class COptions
{
public:
    //profile pane
    int ProfilePaneWidth;

    //file list view
    int ColumnWidth1;
    int ColumnWidth2;
    int ColumnWidth3;
    int SortedColumn;
    int SortOrder;

    //source files
    int UseExplorerContextMenu;

    //processing items
    int SaveFileListOnExit;

    //output
    CString OutputDirectory;
    int UseSourceFileLocation;
    int OverwriteOutputFiles;

    //action on error
    enum
    {
        ACTION_ON_ERROR_SKIP,
        ACTION_ON_ERROR_STOP,
        ACTION_ON_ERROR_PROMT
    };
    int ActionOnError;

    //ctor/dtor
    COptions() {}

    //init, if file_name not NULL, then it is import/export settings call
    bool Read();
    bool Write();
};

extern COptions Options;
