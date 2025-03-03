#pragma once

//CSettings - main settings
struct CSettings
{
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

    //init, if file_name not NULL, then it is import/export settings call
    bool Read();
    bool Write();
};

extern CSettings Settings;
