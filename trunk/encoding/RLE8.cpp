#include "RLE8.h"



void RLE8_Decode(SFF8_u* dst, const SFF8_u* src, SFF32_u len)
{
    SFF32_u dstpos = 0;
    SFF32_u srcpos = 4;

    while (srcpos < len)
    {
        if ( ((src[srcpos] & 0xC0) == 0x40 ))
        {
            for(int run = 0; run < ( src[srcpos] & 0x3F) ; run ++)
            {
                dst[dstpos] = src[srcpos + 1];
                dstpos++;
            }
            srcpos +=2;
        }
        else
        {
            dst[dstpos] = src[srcpos];
            dstpos++;
            srcpos++;
        }
    }


}

void RLE8_Encode(SFF8_u* dst, const SFF8_u* src)
{

}
