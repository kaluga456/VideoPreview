#pragma once

//base class
class CDropFiles
{
public:
    CDropFiles() : Dropfiles(NULL) {}
    virtual ~CDropFiles() {}

    virtual void Deinit() = 0;

    CString GetFileName(UINT index);
    LPCTSTR GetFileName(UINT index, LPTSTR buffer, UINT buffer_size);

protected:
    DROPFILES* Dropfiles;
};

//CClipboardFiles - read files from clipboard
class CClipboardFiles : public CDropFiles
{
public:
    explicit CClipboardFiles(HWND hwnd);
    virtual ~CClipboardFiles();

    virtual void Deinit();

private:
    HANDLE Clipboard;
};

//COleObjectFiles - read files from OLE object
class COleObjectFiles : public CDropFiles
{
public:
    explicit COleObjectFiles(COleDataObject* ole_object);
    virtual ~COleObjectFiles();

    virtual void Deinit();

private:
    HGLOBAL ObjectData;
};