#ifndef SFF2HEADER_H_INCLUDED
#define SFF2HEADER_H_INCLUDED

#include <fstream>

#include "utiltype/sff2int.h"
#include "SFF2_StreamInterface.h"

class SFF2_Header
{
    char Signature[12];
    SFF8_u Verlo3;
    SFF8_u Verlo2;
    SFF8_u Verlo1;
    SFF8_u Verhi;
    SFF32_u Reserved1;
    SFF32_u Reserved2;
    SFF8_u Compatverlo3;
    SFF8_u Compatverlo2;
    SFF8_u Compatverlo1;
    SFF8_u Compatverhi;
    SFF32_u Reserved3;
    SFF32_u Reserved4;
    SFF32_u SNde_Offs;
    SFF32_u Num_Sprite;
    SFF32_u PalNde_Offs;
    SFF32_u Num_Pal;
    SFF32_u Ldata_Ofs;
    SFF32_u Ldata_Len;
    SFF32_u Tdata_Ofs;
    SFF32_u Tdata_Len;
    SFF32_u Reserved5;
    SFF32_u Reserved6;
    char unused[436];
public:
    SFF2_Header();
    SFF2_Header(SFF2_StreamInterface*);
    ~SFF2_Header();
    char *getSignature()        {return Signature;};
    //bool setSignature();
    float getVersion()          { return (float) Verhi + Verlo1 * 0.1 + Verlo2 * 0.01 + Verlo3 * 0.001;}
    float getCompatVersion()    { return (float) Compatverhi + Compatverlo1 * 0.1 + Compatverlo2 * 0.01 + Compatverlo3 * 0.001;}
    SFF32_u getSprOfs()         { return SNde_Offs;}
    SFF32_u getNumSpr()         { return Num_Sprite;}
    SFF32_u getPalOfs()         { return PalNde_Offs;}
    SFF32_u getNumPal()         { return Num_Pal;}
    SFF32_u getLDataOfs()       { return Ldata_Ofs;}
    SFF32_u getLDataLen()       { return Ldata_Len;}
    SFF32_u getTDataOfs()       { return Tdata_Ofs;}
    SFF32_u getTDataLen()       { return Tdata_Len;}
};

#endif // SFF2HEADER_H_INCLUDED
