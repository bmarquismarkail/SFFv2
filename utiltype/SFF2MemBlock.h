#ifndef SSF2MEMBLOCK_H
#define SSF2MEMBLOCK_H

#include <fstream>

#include "sff2int.h"
#include "../sff2_streaminterface.hpp"

class SFF2MemBlock
{
    public:
        SFF2MemBlock();
        SFF2MemBlock(SFF2_StreamInterface  *in, SFF32_u Ofs, SFF32_u Len);
        ~SFF2MemBlock();
        SFF32_u GetOffset() {return Offset;}
        void SetOffset(SFF32_u Ofs) {Offset = Ofs;}
        SFF8_u* GetData()   {return Data;}
        SFF32_u GetLength() {return Length;};
        void ReadDataFromDisk(SFF2_StreamInterface *in, SFF32_u Ofs, SFF32_u Len);
        void TranslateDataFromDisk(SFF2_StreamInterface *in, SFF32_u Ofs, SFF32_u SrcLen, SFF32_u DstLen, SFF16_u Fmt);
        void TranslateData(SFF32_u SrcLen, SFF32_u DstLen, SFF16_u Fmt);
    protected:
    private:
        SFF32_u Offset;
        SFF8_u* Data;
        SFF32_u Length;
        SFF32_u DecompLen;
};

#endif // SSF2MEMBLOCK_H
