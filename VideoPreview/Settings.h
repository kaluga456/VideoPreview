#pragma once

//CSettings - main settings
class CSettings
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

    //DEPRECATE:
    //action on error
    enum
    {
        ACTION_ON_ERROR_SKIP,
        ACTION_ON_ERROR_STOP,
        ACTION_ON_ERROR_PROMT
    };
    int ActionOnError;

    //ctor/dtor
    CSettings() {}

    //init, if file_name not NULL, then it is import/export settings call
    bool Read();
    bool Write();
};

extern CSettings Settings;
