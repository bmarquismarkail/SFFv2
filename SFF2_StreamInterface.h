#ifndef SFF2_STREAMINTERFACE_H
#define SFF2_STREAMINTERFACE_H

#include "utiltype/sff2int.h"
#include <cstddef>

class SFF2_StreamInterface
{
    public:
        SFF2_StreamInterface();
        virtual ~SFF2_StreamInterface();
        virtual int Open(const char* filename)=0;
        virtual int ReadU8(SFF8_u&)=0;
        virtual int ReadU16(SFF16_u&)=0;
        virtual int ReadU32(SFF32_u&)=0;
        virtual int ReadS16(SFF16_s&)=0;
        virtual int Read(char*, size_t)=0;
        virtual int Seek(size_t)=0;
        virtual void Close()=0;
        virtual bool is_open()=0;

    protected:

    private:
};

#endif // SFF2_STREAMINTERFACE_H
