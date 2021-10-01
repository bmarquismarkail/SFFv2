#ifndef SFF2_STREAMINTERFACE_H
#define SFF2_STREAMINTERFACE_H

#include "utiltype/sff2int.h"
#include <cstddef>

class SFF2_StreamInterface
{
    public:
        SFF2_StreamInterface(const char* filename);
        virtual ~SFF2_StreamInterface();
        virtual int ReadU8(SFF8_u&);
        virtual int ReadU16(SFF16_u&);
        virtual int ReadU32(SFF32_u&);
        virtual int ReadS16(SFF16_s&);
        virtual int Read(char*, size_t);
        virtual int Seek(size_t);
        virtual void Close();
        virtual bool is_open();

    protected:

    private:
};

#endif // SFF2_STREAMINTERFACE_H
