#pragma once

class CClipboardFiles
{
public:
    explicit CClipboardFiles(HWND hwnd);
    ~CClipboardFiles();

    void Deinit();

    CString GetFileName(UINT index);
    LPCTSTR GetFileName(UINT index, LPTSTR buffer, UINT buffer_size);

private:
    HANDLE Clipboard;
    DROPFILES* Dropfiles;
};