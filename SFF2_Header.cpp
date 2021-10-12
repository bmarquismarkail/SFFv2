#include "SFF2_Header.h"

#include <cstring>
#include <iostream>
#include <fstream>

SFF2_Header::SFF2_Header()
    :   Signature("ElecbyteSpr"),
        Verlo3(0), Verlo2(0), Verlo1(0),
        Verhi(2),
        Reserved1(0), Reserved2(0),
        Compatverlo3(0), Compatverlo2(0), Compatverlo1(0),
        Compatverhi(2),
        Reserved3(0), Reserved4(0),
        SNde_Offs(0),
        Num_Sprite(0),
        PalNde_Offs(0),
        Num_Pal(0),
        Ldata_Ofs(0),
        Ldata_Len(0),
        Tdata_Ofs(0),
        Tdata_Len(0),
        Reserved5(0), Reserved6(0),
        unused()
{
    //memset(unused,0,436);

}

SFF2_Header::SFF2_Header( SFF2_StreamInterface *in) : SFF2_Header()
{
    in->Read(Signature, 12);
    if (strcmp(Signature, "ElecbyteSpr"))
    {
        std::cerr << "Incompatible  Format";
        return;
    }
    in->ReadU8(Verlo3);
    in->ReadU8(Verlo2);
    in->ReadU8(Verlo1);
    in->ReadU8(Verhi);
    in->ReadU32(Reserved1);
    in->ReadU32(Reserved2);
    in->ReadU8(Compatverlo3);
    in->ReadU8(Compatverlo2);
    in->ReadU8(Compatverlo1);
    in->ReadU8(Compatverhi);
    in->ReadU32(Reserved3);
    in->ReadU32(Reserved4);
    in->ReadU32(SNde_Offs);
    in->ReadU32(Num_Sprite);
    in->ReadU32(PalNde_Offs);
    in->ReadU32(Num_Pal);
    in->ReadU32(Ldata_Ofs);
    in->ReadU32(Ldata_Len);
    in->ReadU32(Tdata_Ofs);
    in->ReadU32(Tdata_Len);
    in->ReadU32(Reserved5);
    in->ReadU32(Reserved6);
    in->Read(unused, sizeof(unused));
}

SFF2_Header::~SFF2_Header()
{

}
