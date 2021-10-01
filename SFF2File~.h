#ifndef SFF2FILE_H
#define SFF2FILE_H

#include <fstream>
#include <cstdlib>

#include "sff2header.h"
#include "SFF2_SpriteNode.h"
#include "SFF2_PalNode.h"
#include "utiltype/SFF2MemBlock.h"

class SFF2File
{
    public:
        SFF2File();
        SFF2File(std::ifstream *in);
        SFF2File(const char* Filename);
        ~SFF2File();
        void Load_Sprite(std::ifstream *in);
        SFF2_Header* GetHeader() { return Head; }
        int GetSpriteIndex(int groupno, int sprno);
        SFF2_SpriteNode* GetSpriteNode(SFF32_u index);
        SFF2_PalNode*    GetPalNode(SFF32_u index);
        SFF2MemBlock* GetPalData(SFF32_u index);
        SFF2MemBlock* GetSprData(SFF32_u index);
    protected:
    private:
        SFF2_Header* Head;
        SFF2_SpriteNode* Sprite;
        SFF2_PalNode* Pal;
        SFF2MemBlock* SprMem;
        SFF2MemBlock* PalMem;

};

#endif // SFF2FILE_H
