#pragma once

//processing item state
enum
{
    //processing now 0...100%
    PIS_MIN_PROCESSING = 0,
    PIS_MAX_PROCESSING = 100,

    PIS_WAIT,                  //ready to process
    PIS_DONE,                  //processed successfully
    PIS_FAILED                 //processed with error
};

//processing item - one for each video file
class CProcessingItem
{
public:
    explicit CProcessingItem(LPCTSTR source_file_name);
    CProcessingItem(int state, LPCTSTR source_file_name, LPCTSTR result_string = _T("")); //for loading from setting file

    bool IsActive() const {return PIS_MIN_PROCESSING <= State && State <= PIS_MAX_PROCESSING;}
    void Reset();

    //data
    INT_PTR State;
    int Selected; //used for Process Selected command
    CString SourceFileName;
    CString ResultString; //for PIS_WAIT - empty, for PIS_DONE - output file name, for PIS_FAILED - error description
};

typedef std::shared_ptr<CProcessingItem> PProcessingItem;
typedef std::map<CProcessingItem*, PProcessingItem> CProcessingItemList;

class CFileList
{
public:
    //item types in list
    enum
    {
        PILS_HAS_READY = 0x00000001,
        PILS_HAS_DONE = 0x00000002,
        PILS_HAS_FAILED = 0x00000004
    };
    void SetBit(int bit, bool value = true);
    BOOL HasReady() const {return Types & PILS_HAS_READY;}
    BOOL HasDone() const {return Types & PILS_HAS_DONE;}
    BOOL HasFailed() const {return Types & PILS_HAS_FAILED;}
    void SetReady(bool value) {SetBit(PILS_HAS_READY, value);}
    void SetDone(bool value) {SetBit(PILS_HAS_DONE, value);}
    void SetFailed(bool value) {SetBit(PILS_HAS_FAILED, value);}
    void SetTypes(int Value) {Types = Value;}
    void UpdateTypes();

    CProcessingItemList Items;
    void AddFile(LPCTSTR file_name);
    bool AddFiles(CDropFiles* DropFiles);

    bool RemoveItems(int Type);

    CFileList() : Types(0) {}

private:
    int Types;
};

extern CFileList FileList;