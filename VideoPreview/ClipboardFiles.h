#pragma once

//base class
class CDropFiles
{
public:
    CDropFiles() : Dropfiles(nullptr) {}
    virtual ~CDropFiles() {}

    virtual void Deinit() = 0;

    CString GetFileName(size_t index);
    LPCTSTR GetFileName(size_t index, LPTSTR buffer, size_t buffer_size);

protected:
    DROPFILES* Dropfiles;
};

//CClipboardFiles - read files from clipboard
class CClipboardFiles : public CDropFiles
{
public:
    explicit CClipboardFiles(HWND hwnd);
    virtual ~CClipboardFiles();

    void Deinit() override;

private:
    HANDLE Clipboard;
};

//COleObjectFiles - read files from OLE object
class COleObjectFiles : public CDropFiles
{
public:
    explicit COleObjectFiles(COleDataObject* ole_object);
    virtual ~COleObjectFiles();

    void Deinit() override;

private:
    HGLOBAL ObjectData;
};