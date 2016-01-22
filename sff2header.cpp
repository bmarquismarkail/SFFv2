#include "sff2header.h"

#include <cstring>
#include <iostream>
#include <fstream>

SFF2_Header::SFF2_Header()
{
    strncpy(Signature,"ElecbyteSpr",12);
    Verlo3 = Verlo2 = Verlo1 = 0;
    Verhi = 2;
    Compatverlo3 = Compatverlo2 = Compatverlo1 = 0;
    Compatverhi = 2;
    SNde_Offs = 0;
    Num_Sprite =0;
    PalNde_Offs = 0;
    Num_Pal = 0;
    Ldata_Ofs = 0;
    Ldata_Len = 0;
    Tdata_Ofs = 0;
    Tdata_Len = 0;
    Reserved1 = Reserved2 = Reserved3 = Reserved4 = Reserved5 = Reserved6 = 0;
    memset(unused,0,436);

}

SFF2_Header::SFF2_Header( std::ifstream *in)
{
    in->read(Signature, 12);
    if (strcmp(Signature, "ElecbyteSpr"))
    {
        std::cerr << "Incompatible  Format";
        return;
    }
    in->read((char*) &Verlo3,sizeof(Verlo3));
    in->read((char*) &Verlo2,sizeof(Verlo2));
    in->read((char*) &Verlo1,sizeof(Verlo1));
    in->read((char*) &Verhi,sizeof(Verhi));
    in->read((char*) &Reserved1, sizeof(Reserved1));
    in->read((char*) &Reserved2, sizeof(Reserved2));
    in->read((char*) &Compatverlo3,sizeof(Compatverlo3));
    in->read((char*) &Compatverlo2,sizeof(Compatverlo2));
    in->read((char*) &Compatverlo1,sizeof(Compatverlo1));
    in->read((char*) &Compatverhi,sizeof(Compatverhi));
    in->read((char*) &Reserved3, sizeof (Reserved3));
    in->read((char*) &Reserved4, sizeof(Reserved4));
    in->read((char*) &SNde_Offs, sizeof(SNde_Offs));
    in->read((char*) &Num_Sprite, sizeof(Num_Sprite));
    in->read((char*) &PalNde_Offs, sizeof(PalNde_Offs));
    in->read((char*) &Num_Pal, sizeof(Num_Pal));
    in->read((char*) &Ldata_Ofs, sizeof(Ldata_Ofs));
    in->read((char*) &Ldata_Len, sizeof(Ldata_Len));
    in->read((char*) &Tdata_Ofs, sizeof(Tdata_Ofs));
    in->read((char*) &Tdata_Len, sizeof(Tdata_Len));
    in->read((char*) &Reserved5, sizeof(Reserved5));
    in->read((char*) &Reserved6, sizeof(Reserved6));
    in->read(unused, sizeof(unused));
}

SFF2_Header::~SFF2_Header()
{

}
