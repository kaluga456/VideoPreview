#ifndef _APP_H_
#define _APP_H_

//bitmask
enum
{
    BIT_0 = 0x00000001,
    BIT_1 = 0x00000004,
    BIT_3 = 0x00000008,
    BIT_4 = 0x00000010,
    BIT_5 = 0x00000020,
    BIT_6 = 0x00000040,
    BIT_7 = 0x00000080,
    BIT_8 = 0x00000100,
    BIT_9 = 0x00000200,
    BIT_10 = 0x00000400,
    BIT_11 = 0x00000800,
    BIT_12 = 0x00001000,
    BIT_13 = 0x00002000,
    BIT_14 = 0x00004000,
    BIT_15 = 0x00008000,
    BIT_16 = 0x00010000,
    BIT_17 = 0x00020000,
    BIT_18 = 0x00040000,
    BIT_19 = 0x00080000,
    BIT_20 = 0x00100000,
    BIT_21 = 0x00200000,
    BIT_22 = 0x00400000,
    BIT_23 = 0x00800000,
    BIT_24 = 0x01000000,
    BIT_25 = 0x02000000,
    BIT_26 = 0x04000000,
    BIT_27 = 0x08000000,
    BIT_28 = 0x10000000,
    BIT_29 = 0x20000000,
    BIT_30 = 0x40000000,
    BIT_31 = 0x80000000
};

//static array size
#ifndef APP_ARRAY_SIZE
    #define APP_ARRAY_SIZE(array) (sizeof(array)/sizeof(array[0]))
#endif //GET_ARRAY_SIZE

//common sizes
const size_t KILOBYTE = 1024u;
const size_t MEGABYTE = 1048576u;
const size_t GIGABYTE = 1073741824u;

namespace app {

} //namespace app

#endif //_APP_H_
