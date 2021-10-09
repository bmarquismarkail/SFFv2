#ifndef SFF2FILE_H
#define SFF2FILE_H

#include <fstream>
#include <vector>

#include "sff2header.h"
#include "SFF2_SpriteNode.h"
#include "SFF2_PalNode.h"
#include "utiltype/SFF2MemBlock.h"

#include "/home/brandon/dev/src/lang/C/Libraries/SFF2/sff2_streaminterface.hpp"

class SFF2File
{
    public:
        SFF2File();
        SFF2File(SFF2_StreamInterface *in);
        SFF2File(SFF2_StreamInterface *in, const char* Filename);
        ~SFF2File();
        void Load_Sprite();
        SFF2_Header* GetHeader() { return Head; }
        SFF2_SpriteNode* GetSpriteNode(SFF32_u index);
        SFF32_u GetSpriteIndex(SFF16_u groupNo, SFF16_u SprNo);
        SFF2_PalNode*    GetPalNode(SFF32_u index);
        SFF2MemBlock* GetPalData(SFF32_u index);
        SFF2MemBlock* GetSprData(SFF32_u index);
    protected:
    private:
        SFF2_StreamInterface *interface;
        SFF2_Header* Head;
        std::vector<SFF2_SpriteNode> Sprite;
        std::vector<SFF2_PalNode> Pal;
        SFF2MemBlock* SprMem;
        SFF2MemBlock* PalMem;

};

#endif // SFF2FILE_H
