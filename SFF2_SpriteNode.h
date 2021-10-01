#ifndef SFF2_SPRITENODE_H
#define SFF2_SPRITENODE_H

#include <fstream>

#include "utiltype/sff2int.h"
#include "sff2_streaminterface.hpp"

class SFF2_SpriteNode
{
    public:
        SFF2_SpriteNode();
        SFF2_SpriteNode(SFF2_StreamInterface*in);
        void ReadFromDisk(SFF2_StreamInterface *in);
        ~SFF2_SpriteNode();
        SFF16_u GetGroupNo()        { return GroupNo; }
        SFF16_u GetSprNo()          { return SprNo; }
        SFF16_u GetWidth()          { return Width; }
        SFF16_u GetHeight()         { return Height; }
        SFF16_s GetXaxis()          { return Xaxis; }
        SFF16_s GetYaxis()          { return Yaxis; }
        SFF16_u GetLnkInd()         { return LnkInd; }
        SFF8_u  GetFmt()            { return Fmt; }
        SFF8_u  GetColdepth()       { return Coldepth; }
        SFF32_u GetDataOfs()        { return DataOfs; }
        SFF32_u GetDataLen()        { return DataLen; }
        SFF16_u GetPalInd()         { return PalInd; }
        SFF16_u GetFlags()          { return Flags; }
        bool isTdata()              {return Flags & 0x00000001;}
        bool isLinked ()            {return (DataLen) ? false: true;}
    protected:
    private:
        SFF16_u GroupNo;
        SFF16_u SprNo;
        SFF16_u Width;
        SFF16_u Height;
        SFF16_s Xaxis;
        SFF16_s Yaxis;
        SFF16_u LnkInd;
        SFF8_u Fmt;
        SFF8_u Coldepth;
        SFF32_u DataOfs;
        SFF32_u DataLen;
        SFF16_u PalInd;
        SFF16_u Flags;
};

#endif // SFF2_SPRITENODE_H
