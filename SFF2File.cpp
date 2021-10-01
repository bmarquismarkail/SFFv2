#include "SFF2File.h"

SFF2File::SFF2File():
    Head(NULL),
    SprMem(NULL),
    PalMem(NULL)
{
}

SFF2File::SFF2File(const char *Filename):
    Head(NULL),
    SprMem(NULL),
    PalMem(NULL)
{
    SFF2_StreamInterface *in = new SFF2_StreamInterface(Filename);
    if(!in->is_open())
        exit (-1);
    Load_Sprite(in);
    in->Close();
    delete in;
}

SFF2File::SFF2File(SFF2_StreamInterface *in):
    Head(NULL),
    PalMem(NULL)
{
    Load_Sprite(in);
}


SFF2File::~SFF2File()
{
    delete[] PalMem;
    delete[] SprMem;
    Sprite.clear();
    Pal.clear();
    delete Head;
}

void SFF2File::Load_Sprite(SFF2_StreamInterface *in)
{
    Head = new SFF2_Header(in);
    Sprite.insert(Sprite.begin(), Head->getNumSpr(), SFF2_SpriteNode());
    for( SFF32_u s = 0; s < Head->getNumSpr(); s++)
        {
            in->Seek(Head->getSprOfs()+(28*s));
            Sprite[s].ReadFromDisk(in);
        }

    //Pal = new SFF2_PalNode[Head->getNumPal()];
    Pal.insert(Pal.begin(), Head->getNumPal(), SFF2_PalNode() );
    for( SFF32_u p = 0; p < Head->getNumPal(); p++)
        {
            in->Seek(Head->getPalOfs()+(16*p));
            Pal[p].ReadFromDisk(in);
        }
    PalMem = new SFF2MemBlock[Head->getNumPal()]();
    SprMem = new SFF2MemBlock[Head->getNumSpr()]();

    for( SFF32_u pm = 0; pm < Head->getNumPal(); pm++)
        {
            PalMem[pm].ReadDataFromDisk(in,(Head->getLDataOfs() + Pal[pm].GetDataOfs()),Pal[pm].GetDataLen());
        }
    for( SFF32_u sm = 0; sm < Head->getNumSpr(); sm++)
        {
            if (Sprite[sm].isTdata())
                {
                    SprMem[sm].TranslateDataFromDisk(in,(Head->getTDataOfs() + Sprite[sm].GetDataOfs()), Sprite[sm].GetDataLen(), (Sprite[sm].GetWidth() * Sprite[sm].GetHeight()), Sprite[sm].GetFmt());
                }
            else
                {
                    SprMem[sm].ReadDataFromDisk(in,(Head->getLDataOfs() + Sprite[sm].GetDataOfs()),Sprite[sm].GetDataLen());
                }
        }

}

SFF2_SpriteNode* SFF2File::GetSpriteNode(SFF32_u index)
{
    return &Sprite[index];
}

SFF32_u SFF2File::GetSpriteIndex(SFF16_u groupNo, SFF16_u SprNo)
{
    for(SFF32_u index=0; index < Head->getNumSpr(); index++)
        {
            if( Sprite[index].GetGroupNo() == groupNo && Sprite[index].GetSprNo() == SprNo)
                return index;
        }
    throw;
}

SFF2_PalNode*SFF2File::GetPalNode(SFF32_u index)
{
    return &Pal[index];
}

SFF2MemBlock* SFF2File::GetPalData(SFF32_u index)
{
    return (Pal[index].isLinked()) ? GetPalData(Pal[index].GetLnkInd()) : &PalMem[index];
}

SFF2MemBlock* SFF2File::GetSprData(SFF32_u index)
{
    return (Sprite[index].isLinked()) ? GetSprData(Sprite[index].GetLnkInd()) : &SprMem[index];
}
