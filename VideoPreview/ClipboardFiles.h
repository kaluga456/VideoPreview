#pragma once

//base class
class CDropFiles
{
public:
    CDropFiles() : Dropfiles(nullptr) {}
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
    ~CClipboardFiles() override;

    void Deinit() override;

private:
    HANDLE Clipboard;
};

//COleObjectFiles - read files from OLE object
class COleObjectFiles : public CDropFiles
{
public:
    explicit COleObjectFiles(COleDataObject* ole_object);
    ~COleObjectFiles() override;

    void Deinit() override;

private:
    HGLOBAL ObjectData;
};