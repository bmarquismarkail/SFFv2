

#include "SFF2File.h"

SFF2File::SFF2File()
{
    PalMem = NULL;
    SprMem = NULL;
    Sprite = NULL;
    Pal = NULL;
}

SFF2File::SFF2File(const char *Filename)
{
    std::ifstream *in = new std::ifstream(Filename , std::ios::binary);
    if(!in->is_open())
        exit (-1);
    Load_Sprite(in);
    in->close();
    delete in;
}

SFF2File::SFF2File(std::ifstream *in)
{
    Load_Sprite(in);
}


SFF2File::~SFF2File()
{
    delete[] PalMem;
    delete[] SprMem;
    delete[] Sprite;
    delete[] Pal;
    delete Head;
}

void SFF2File::Load_Sprite(std::ifstream *in)
{
    Head = new SFF2_Header(in);
    Sprite = new SFF2_SpriteNode[Head->getNumSpr()];
    for( SFF32_u s = 0; s < Head->getNumSpr(); s++)
    {
        in->seekg(Head->getSprOfs()+(28*s));
        Sprite[s].ReadFromDisk(in);
    }

    Pal = new SFF2_PalNode[Head->getNumPal()];
    for( SFF32_u p = 0; p < Head->getNumPal(); p++)
    {
        in->seekg(Head->getPalOfs()+(16*p));
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
            SprMem[sm].TranslateDataFromDisk(in,(Head->getTDataOfs() + Sprite[sm].GetDataOfs()), Sprite[sm].GetDataLen() , (Sprite[sm].GetWidth() * Sprite[sm].GetHeight()), Sprite[sm].GetFmt());
        }
        else
        {
            SprMem[sm].ReadDataFromDisk(in,(Head->getLDataOfs() + Sprite[sm].GetDataOfs()),Sprite[sm].GetDataLen());
        }
    }

}

SFF2_SpriteNode* SFF2File::GetSpriteNode(SFF32_u index)
{
    //return &Sprite[index];
    return &Sprite[index];
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
