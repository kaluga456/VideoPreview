#pragma once

//processing item state
enum
{
    //processing now 0...100%
    PIS_MIN_PROCESSING = 0,
    PIS_MAX_PROCESSING = 100,

    PIS_READY,                  //ready to process
    PIS_DONE,                   //processed successfully
    PIS_FAILED,                 //processed with error
    PIS_STOPPED                 //TODO: stopped by user
};

//processing item - one per each source file
class CProcessingItem
{
public:
    //ctor/dtor
    explicit CProcessingItem(LPCTSTR source_file_name);
    CProcessingItem(int state, LPCTSTR source_file_name, LPCTSTR result_string = _T("")); //for loading from setting file
    ~CProcessingItem();

    bool IsActive() const {return PIS_MIN_PROCESSING <= State && State <= PIS_MAX_PROCESSING;}
    void Reset(bool delete_output_file);

    //data
    int State;
    CString SourceFileName;
    CString ResultString; //for PIS_READY - empty, for PIS_DONE - output file name, for PIS_FAILED - error description 
};
typedef boost::shared_ptr<CProcessingItem> PProcessingItem;
typedef std::map<CProcessingItem*, PProcessingItem> CProcessingItemList;
