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

    //init
    bool Read();
    bool Write();
};

extern CSettings Settings;
