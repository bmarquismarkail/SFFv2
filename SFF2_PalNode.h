#ifndef SFF2_PALNODE_H
#define SFF2_PALNODE_H

#include <fstream>

#include "utiltype/sff2int.h"
#include "sff2_streaminterface.hpp"

class SFF2_PalNode
{
    public:
        SFF2_PalNode();
        SFF2_PalNode(SFF2_StreamInterface *in);
        void ReadFromDisk(SFF2_StreamInterface *in);
        ~SFF2_PalNode();
        SFF16_u GetGroupNo()    { return GroupNo; }
        SFF16_u GetPalNo()      { return PalNo; }
        SFF16_u GetNumCols()    { return NumCols; }
        SFF16_u GetLnkInd()     { return LnkInd; }
        SFF32_u GetDataOfs()    { return DataOfs; }
        SFF32_u GetDataLen()    { return DataLen; }
        bool isLinked ()        {return (DataLen) ? false: true;}
    protected:
    private:
        SFF16_u GroupNo;
        SFF16_u PalNo;
        SFF16_u NumCols;
        SFF16_u LnkInd;
        SFF32_u DataOfs;
        SFF32_u DataLen;
};

#endif // SFF2_PALNODE_H
