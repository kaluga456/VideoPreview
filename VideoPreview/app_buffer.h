#ifndef _APP_BUFFER_H_
#define _APP_BUFFER_H_

namespace app {

//heap memory buffer for POD types
template <typename T> class buffer
{
public:
    //ctor/dtor
    buffer() : Data(nullptr) {}
    explicit buffer(const buffer& source) 
    {
        if(source.Data != nullptr)
        {
            _ASSERTE(source.Size > 0);

            //allocate
            Data = new T[source.Size];
            Size = source.Size;

            //copy
            ::memcpy(Data, source.Data, source.Size * sizeof(T));
        }
        else
            Data = nullptr;
    }
    explicit buffer(size_t size)
    {
        if(size > 0)
        {
            Data = new T[size];
            Size = size;
        }
        else
            Data = nullptr;
    }
    buffer(const T* data, size_t size) 
    {
        if(data != nullptr && size > 0)
        {
            //allocate
            Data = new T[size];
            Size = size;

            //copy
            ::memcpy(Data, data, size * sizeof(T));
        }
        else
            Data = nullptr;
    }
    ~buffer() noexcept {if(Data != nullptr) delete [] Data;}

    //access
    T* data() noexcept {return Data;}
    operator T*()  noexcept {return Data;}
    size_t size() const noexcept {return (nullptr == Data) ? 0 : Size;}

    //init
    void reset() noexcept
    {
        if(Data != nullptr)
        {
            delete [] Data;
            Data = nullptr;
        }
    }
    void reset(size_t size)
    {
        if(Data != nullptr)
            delete [] Data;

        if(size > 0)
        {
            Data = new T[size];
            Size = size;
        }
        else
            Data = nullptr;
    }
    void reset(const T* data, size_t sz)
    {
        if(Data != nullptr)
            delete [] Data;

        if(data != nullptr && sz > 0)
        {
            //allocate
            Data = new T[sz];
            Size = sz;

            //copy
            ::memcpy(Data, data, size() * sizeof(T));
        }
        else
            Data = nullptr;
    }
    void reset(const buffer& source)
    {
        if(Data != nullptr)
            delete [] Data;

        if(source.Data != nullptr)
        {
            _ASSERTE(source.Size > 0);

            //allocate
            Data = new T[source.Size];
            Size = source.Size;

            //copy
            ::memcpy(Data, source.Data, source.Size * sizeof(T));
        }
        else
            Data = nullptr;
    }

    //operators
    buffer<T>& operator=(const buffer& source)
    {
        reset(source);
        return *this;
    }

private:
    T* Data;
    size_t Size;
};

//heap memory buffer for null-terminated string
template <typename T> class strbuf
{
public:
    //ctor/dtor
    strbuf() : Data(nullptr) {}
    explicit strbuf(const strbuf& source)
    {
        if(source.Data != nullptr)
        {
            //allocate
            Data = new T[source.Size + 1];
            Size = source.Size;

            //copy
            ::memcpy(Data, source.Data, (source.Size + 1) * sizeof(T));
        }
        else
            Data = nullptr;
    }
    strbuf(const T* data, size_t size) 
    {
        if(data != nullptr)
        {
            //allocate
            Data = new T[size + 1];
            Size = size;

            //copy
            ::memcpy(Data, data, size * sizeof(T));

            //terminate
            Data[size] = static_cast<T>(0);
        }
        else
            Data = nullptr;
    }
    ~strbuf() noexcept {if(Data != nullptr) delete [] Data;}

    //access
    const T* data() noexcept {return (nullptr == Data) ? &NullT : Data;}
    operator const T*()  noexcept {return data();}
    size_t size() const noexcept {return (nullptr == Data) ? 0 : Size;}

    //init
    void reset() noexcept
    {
        if(Data != nullptr) 
        {
            delete [] Data;
            Data = nullptr;
        }
    }
    void reset(const T* data, size_t sz)
    {
        if(Data != nullptr) 
            delete [] Data;

        if(data != nullptr)
        {
            Data = new T[sz + 1];
            Size = sz;
            ::memcpy(Data, data, size() * sizeof(T));
            Data[sz] = static_cast<T>(0);
        }
        else
            Data = nullptr;
    }
    void reset(const strbuf& source)
    {
        if(Data != nullptr) 
            delete [] Data;

        if(source.Data != nullptr)
        {
            Data = new T[source.Size + 1];
            Size = source.Size;
            ::memcpy(Data, source.Data, (source.Size + 1)* sizeof(T));
        }
        else
            Data = nullptr;
    }

    //operators
    strbuf<T>& operator=(const strbuf& source)
    {
        reset(source);
        return *this;
    }

private:
    static const T NullT = static_cast<T>(0); //empty string
    T* Data;
    size_t Size;
};

typedef buffer<unsigned char> byte_buffer;
typedef strbuf<char> astrbuf;
typedef strbuf<wchar_t> wstrbuf;
#ifndef _UNICODE
    typedef strbuf<char> tstrbuf;
#else
    typedef strbuf<wchar_t> tstrbuf;
#endif //_UNICODE

} //namespace app

#endif //_APP_BUFFER_H_
