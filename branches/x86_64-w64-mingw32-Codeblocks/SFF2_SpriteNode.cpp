

#include "SFF2_SpriteNode.h"

SFF2_SpriteNode::SFF2_SpriteNode()
{
    GroupNo = 0;
    SprNo = 0;
    Width = 0;
    Height = 0;
    Xaxis = 0;
    Yaxis = 0;
    LnkInd = 0; //If the sprite is linked, then this will be nonzero.

    /*                                                               /
    /   The Format of the Sprite. Needs to be one of four values:    /
    /   0: Raw Pixels                                                /
    /   2: RLE8                                                      /
    /   3: RLE5                                                      /
    /   4: LZ5                                                       /
    /                                                               */
    Fmt = 0;
    Coldepth = 0;
    DataOfs = 0;
    DataLen = 0; //If the sprite is linked, this will remain zero.
    PalInd = 0;

    /*                                                                       /
    /   Sprite Flags. Only Bit 0 is used.                                    /
    /   Bit 0:                                                               /
    /       Set: The data is literal and loaded into memory until needed.    /
    /       Unset: The data is translated, and is converted immediately.     /
    /                                                                       */
    Flags = 0;
}

SFF2_SpriteNode::SFF2_SpriteNode(std::ifstream *in)
{
    ReadFromDisk(in);
}

void SFF2_SpriteNode::ReadFromDisk(std::ifstream *in)
{
    in->read((char*) &GroupNo, sizeof(GroupNo));
    in->read((char*) &SprNo, sizeof(SprNo));
    in->read((char*) &Width, sizeof(Width));
    in->read((char*) &Height, sizeof(Height));
    in->read((char*) &Xaxis, sizeof(Xaxis));
    in->read((char*) &Yaxis, sizeof(Yaxis));
    in->read((char*) &LnkInd, sizeof(LnkInd));
    in->read((char*) &Fmt, sizeof(Fmt));
    in->read((char*) &Coldepth, sizeof(Coldepth));
    in->read((char*) &DataOfs, sizeof(DataOfs));
    in->read((char*) &DataLen, sizeof(DataLen));
    in->read((char*) &PalInd, sizeof(PalInd));
    in->read((char*) &Flags, sizeof(Flags));
}

SFF2_SpriteNode::~SFF2_SpriteNode()
{
    //dtor
}
