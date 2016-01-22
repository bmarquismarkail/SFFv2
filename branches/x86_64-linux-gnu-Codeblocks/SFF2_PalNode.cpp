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

SFF2_PalNode::SFF2_PalNode(std::ifstream *in)
{
    ReadFromDisk(in);
}

void SFF2_PalNode::ReadFromDisk(std::ifstream *in)
{
    in->read((char*) &GroupNo, sizeof(GroupNo));
    in->read((char*) &PalNo, sizeof(PalNo));
    in->read((char*) &NumCols, sizeof(NumCols));
    in->read((char*) &LnkInd, sizeof(LnkInd));
    in->read((char*) &DataOfs, sizeof(DataOfs));
    in->read((char*) &DataLen, sizeof(DataLen));
}
SFF2_PalNode::~SFF2_PalNode()
{
    //dtor
}
