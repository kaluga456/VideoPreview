#ifndef _APP_GDI_H_
#define _APP_GDI_H_

namespace app
{

//GDI+ error strings (according to MSDN)
const LPCWSTR gdi_error_strings[] =
{
    L"Ok",                           //Ok = 0,
    L"Generic error",                //GenericError = 1,
    L"Invalid parameter",            //InvalidParameter = 2,
    L"Out of memory",                //OutOfMemory = 3,
    L"Object busy",                  //ObjectBusy = 4,
    L"Insufficient buffer",          //InsufficientBuffer = 5,
    L"Not implemented",              //NotImplemented = 6,
    L"Win32 error",                  //Win32Error = 7,
    L"Wrong state",                  //WrongState = 8,
    L"Aborted",                      //Aborted = 9,
    L"File not found",               //FileNotFound = 10,
    L"Value overflow",               //ValueOverflow = 11,
    L"Access denied",                //AccessDenied = 12,
    L"Unknown image format",         //UnknownImageFormat = 13,
    L"Font family not found",        //FontFamilyNotFound = 14,
    L"Font style not found",         //FontStyleNotFound = 15,
    L"Not true type font",           //NotTrueTypeFont = 16,
    L"Unsupported gdiplus version",  //UnsupportedGdiplusVersion = 17,
    L"Gdiplus not initialized",      //GdiplusNotInitialized = 18,
    L"Property not found",           //PropertyNotFound = 19,
    L"Property not supported",       //PropertyNotSupported = 20,
    L"Profile not found"             //ProfileNotFound = 21
};
const size_t GDI_ERRORS_COUNT = sizeof(gdi_error_strings) / sizeof(LPWSTR);

//get error string from gdi+ status code
constexpr LPCWSTR gdi_get_error_string(Gdiplus::Status status)
{
    const size_t value = static_cast<size_t>(status);
    return (value >= GDI_ERRORS_COUNT) ? L"Undefined error" : gdi_error_strings[value];
}

//GDI exception
class gdi_exception : public exception
{
public:
    //ctor/dtor
    explicit gdi_exception(Gdiplus::Status status) noexcept : Status{ status } {}
    virtual ~gdi_exception() noexcept {}

    //access
    virtual const TCHAR* string() const noexcept {return gdi_get_error_string(Status);}

private:
    Gdiplus::Status Status;
};

//throws gdi exception in case of gdi status code failed
inline void verify_gdi(Gdiplus::Status status)
{
    if(status != Gdiplus::Ok)
        throw gdi_exception(status);
}

//GDI initializer
class gdi
{
public:
    //ctor/dtor
    explicit gdi(Gdiplus::GdiplusStartupOutput* output = nullptr) 
    {
        Gdiplus::GdiplusStartupInput input; //used by default here
        verify_gdi(Gdiplus::GdiplusStartup(&Token, &input, output));
    }
    ~gdi()
    {
        Gdiplus::GdiplusShutdown(Token);
    }

private:
    ULONG_PTR Token;

};

//encoder list
class gdi_encoders
{
public:
    //ctor/dtor
    gdi_encoders() : Count(0), Data(nullptr) {}
    ~gdi_encoders() {::free(Data);}

    //operation
    Gdiplus::Status initialize()
    {
        //free previous data
        free();

        //get encoders count and required space for them
        Gdiplus::Status status = Gdiplus::GetImageEncodersSize(&Count, &DataSize);
        if(status != Gdiplus::Ok)
            return status;

        //allocate memory
        Data = static_cast<Gdiplus::ImageCodecInfo*>(::malloc(DataSize));

        //get encoders data
        status = Gdiplus::GetImageEncoders(Count, DataSize, Data);
        if(status != Gdiplus::Ok)
            free();
        return status;
    }
    void free()
    {
        Count = 0;
        ::free(Data);
        Data = nullptr;
    }

    //access
    UINT count() const {return Count;}
    const Gdiplus::ImageCodecInfo* encoder(UINT encoder_index) const
    {
        return (encoder_index < Count) ? (Data + encoder_index) : nullptr;
    }

    //get encoder CLSID from mime string, expected mime strings are
    //image/bmp
    //image/jpeg
    //image/gif
    //image/tiff
    //image/png
    bool encoder_clsid(const TCHAR* mime_string, CLSID& clsid)
    {
        if(nullptr == mime_string)
            return false;

        for(UINT encoder_index = 0; encoder_index < Count; ++encoder_index)
        {
            const Gdiplus::ImageCodecInfo* encoder = Data + encoder_index;
            if(0 == ::wcscmp(encoder->MimeType, mime_string))
            {
                clsid = encoder->Clsid;
                return true;
            }
        }
        return false;
    }

private:
    UINT  Count;        // number of image encoders
    UINT  DataSize;     // size, in bytes, of the image encoder array
    Gdiplus::ImageCodecInfo* Data; //encoders array
};

}
#endif //_APP_GDI_H_
