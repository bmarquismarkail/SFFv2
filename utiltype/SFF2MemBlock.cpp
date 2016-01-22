
#include "SFF2MemBlock.h"

#include <iostream>


#include "..\encoding/LZ5.h"
#include "..\encoding/RLE5.h"
#include "..\encoding/RLE8.h"


SFF2MemBlock::SFF2MemBlock()
{
    Offset      = 0;
    Data        = NULL;
    Length      = 0;
    DecompLen   = 0;
    //ctor
}

SFF2MemBlock::SFF2MemBlock(std::ifstream *in, SFF32_u Ofs, SFF32_u Len)
{
    ReadDataFromDisk(in,Ofs,Len);
}

SFF2MemBlock::~SFF2MemBlock()
{
    if (Data)
    {
        delete [] Data;
        Data = NULL;
    }
}

void SFF2MemBlock::ReadDataFromDisk(std::ifstream *in, SFF32_u Ofs, SFF32_u Len)
{
    Offset = Ofs;
    Length = Len;
    in->seekg(Offset);
    Data = new SFF8_u[Len];
    in->read((char*)Data, Len);
}

void SFF2MemBlock::TranslateDataFromDisk(std::ifstream *in, SFF32_u Ofs, SFF32_u SrcLen, SFF32_u DstLen, SFF16_u Fmt)
{
    if (!SrcLen)
        return;
    ReadDataFromDisk(in, Ofs, SrcLen);
    //By default, sprmaker2 automatically makes uncompressed data ldata. This is just a safeguard for other tools, such as my own.
    if (!Fmt)
        return;

    SFF8_u* Decompressed = new SFF8_u[DstLen];

    switch (Fmt) {
        case 1: //Invalid format as of version 2.00
            std::cerr << "Invalid!";
            break;
        case 2: // RLE 8
            RLE8_Decode(Decompressed, Data, SrcLen);
            Length = DstLen;
            break;
        case 3: //RLE5
            RLE5_Decode(Decompressed, Data, SrcLen);
            Length = DstLen;
            break;
        case 4: //LZ5
            LZ5_Decode(Decompressed, Data, SrcLen);
            Length = DstLen;
            break;
    }
    if (Data) delete[] Data;
    Data = Decompressed;
}

void SFF2MemBlock::TranslateData(SFF32_u SrcLen, SFF32_u DstLen, SFF16_u Fmt)
{
    /* This function needs to return some type of value for error checking for the following reasons:
        -No sprite data pointed in Data, which will lead to undefined results
    */

    //other than reading the data, Translate is virtually the same function.
    if (!Fmt)
        return;

    SFF8_u* Decompressed = new SFF8_u[DstLen];

    switch (Fmt) {
        case 1: //Invalid format as of version 2.00
            std::cerr << "Invalid!";
            break;
        case 2: // RLE 8
            RLE8_Decode(Decompressed, Data, SrcLen);
            Length = DstLen;
            break;
        case 3: //RLE5
            RLE5_Decode(Decompressed, Data, SrcLen);
            Length = DstLen;
            break;
        case 4: //LZ5
            LZ5_Decode(Decompressed, Data, SrcLen);
            Length = DstLen;
            break;
    }
    if (Data) delete[] Data;
    Data = Decompressed;
}
