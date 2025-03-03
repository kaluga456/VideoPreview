#ifndef _APP_BUFFER_H_
#define _APP_BUFFER_H_

namespace app {

//heap memory buffer for POD types
template <typename T> class buffer
{
public:
    //ctor/dtor
    buffer() : Data(NULL) {}
    explicit buffer(const buffer& source) 
    {
        if(source.Data != NULL)
        {
            _ASSERTE(source.Size > 0);

            //allocate
            Data = new T[source.Size];
            Size = source.Size;

            //copy
            ::memcpy(Data, source.Data, source.Size * sizeof(T));
        }
        else
            Data = NULL;
    }
    explicit buffer(size_t size)
    {
        if(size > 0)
        {
            Data = new T[size];
            Size = size;
        }
        else
            Data = NULL;
    }
    buffer(const T* data, size_t size) 
    {
        if(data != NULL && size > 0)
        {
            //allocate
            Data = new T[size];
            Size = size;

            //copy
            ::memcpy(Data, data, size * sizeof(T));
        }
        else
            Data = NULL;
    }
    ~buffer() noexcept {if(Data != NULL) delete [] Data;}

    //access
    T* data() noexcept {return Data;}
    operator T*()  noexcept {return Data;}
    size_t size() const noexcept {return (NULL == Data) ? 0 : Size;}

    //init
    void reset() noexcept
    {
        if(Data != NULL) 
        {
            delete [] Data;
            Data = NULL;
        }
    }
    void reset(size_t size)
    {
        if(Data != NULL) 
            delete [] Data;

        if(size > 0)
        {
            Data = new T[size];
            Size = size;
        }
        else
            Data = NULL;
    }
    void reset(const T* data, size_t size)
    {
        if(Data != NULL) 
            delete [] Data;

        if(data != NULL && size > 0)
        {
            //allocate
            Data = new T[size];
            Size = size;

            //copy
            ::memcpy(Data, data, size() * sizeof(T));
        }
        else
            Data = NULL;
    }
    void reset(const buffer& source)
    {
        if(Data != NULL) 
            delete [] Data;

        if(source.Data != NULL)
        {
            _ASSERTE(source.Size > 0);

            //allocate
            Data = new T[source.Size];
            Size = source.Size;

            //copy
            ::memcpy(Data, source.Data, source.Size * sizeof(T));
        }
        else
            Data = NULL;
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
    strbuf() : Data(NULL) {}
    explicit strbuf(const strbuf& source)
    {
        if(source.Data != NULL)
        {
            //allocate
            Data = new T[source.Size + 1];
            Size = source.Size;

            //copy
            ::memcpy(Data, source.Data, (source.Size + 1) * sizeof(T));
        }
        else
            Data = NULL;
    }
    strbuf(const T* data, size_t size) 
    {
        if(data != NULL)
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
            Data = NULL;
    }
    ~strbuf() noexcept {if(Data != NULL) delete [] Data;}

    //access
    const T* data() noexcept {return (NULL == Data) ? &NullT : Data;}
    operator const T*()  noexcept {return data();}
    size_t size() const noexcept {return (NULL == Data) ? 0 : Size;}

    //init
    void reset() noexcept
    {
        if(Data != NULL) 
        {
            delete [] Data;
            Data = NULL;
        }
    }
    void reset(const T* data, size_t size)
    {
        if(Data != NULL) 
            delete [] Data;

        if(data != NULL)
        {
            Data = new T[size + 1];
            Size = size;
            ::memcpy(Data, data, size() * sizeof(T));
            Data[size] = static_cast<T>(0);
        }
        else
            Data = NULL;
    }
    void reset(const strbuf& source)
    {
        if(Data != NULL) 
            delete [] Data;

        if(source.Data != NULL)
        {
            Data = new T[source.Size + 1];
            Size = source.Size;
            ::memcpy(Data, source.Data, (source.Size + 1)* sizeof(T));
        }
        else
            Data = NULL;
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
