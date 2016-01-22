#ifndef SFF2INT_H_INCLUDED
#define SFF2INT_H_INCLUDED

#if __cplusplus >= 201103L
    #include <cstdint>
        using std::int16_t;
        using std::uint8_t;
        using std::uint16_t;
        using std::uint32_t;
#else
    #include <stdint.h>
#endif // __cplusplus

typedef int16_t     SFF16_s;
typedef uint8_t     SFF8_u;
typedef uint16_t    SFF16_u;
typedef uint32_t    SFF32_u;


#endif // SFF2INT_H_INCLUDED
