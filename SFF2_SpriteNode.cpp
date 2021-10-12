

#include "SFF2_SpriteNode.h"

SFF2_SpriteNode::SFF2_SpriteNode()
    :   GroupNo(0), SprNo(0), Width(0),
        Height(0), Xaxis(0), Yaxis(0),
        LnkInd(0), Fmt(0), Coldepth(0),
        DataOfs(0), DataLen(0), PalInd(0),
        Flags(0)
{
}

SFF2_SpriteNode::SFF2_SpriteNode(SFF2_StreamInterface *in) : SFF2_SpriteNode()
{
    ReadFromDisk(in);
}

void SFF2_SpriteNode::ReadFromDisk(SFF2_StreamInterface *in)
{
    in->ReadU16(GroupNo);
    in->ReadU16(SprNo);
    in->ReadU16(Width);
    in->ReadU16(Height);
    in->ReadS16(Xaxis);
    in->ReadS16(Yaxis);
    in->ReadU16(LnkInd);
    in->ReadU8(Fmt);
    in->ReadU8(Coldepth);
    in->ReadU32(DataOfs);
    in->ReadU32(DataLen);
    in->ReadU16(PalInd);
    in->ReadU16(Flags);
}

SFF2_SpriteNode::~SFF2_SpriteNode()
{
    //dtor
}
