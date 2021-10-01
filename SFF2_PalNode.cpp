#include "SFF2_PalNode.h"

SFF2_PalNode::SFF2_PalNode()
{
    GroupNo = 0;
    PalNo = 0;
    NumCols = 0;
    LnkInd = 0;     //This the sprite is linked, then this will be nonzero.
    DataOfs = 0;
    DataLen = 0;    //The sprite data length. if the sprite is linked, this will be zero.
}

SFF2_PalNode::SFF2_PalNode(SFF2_StreamInterface*in)
{
    ReadFromDisk(in);
}

void SFF2_PalNode::ReadFromDisk(SFF2_StreamInterface *in)
{
    in->ReadU16(GroupNo);
    in->ReadU16(PalNo);
    in->ReadU16(NumCols);
    in->ReadU16(LnkInd);
    in->ReadU32(DataOfs);
    in->ReadU32(DataLen);
}
SFF2_PalNode::~SFF2_PalNode()
{
    //dtor
}
