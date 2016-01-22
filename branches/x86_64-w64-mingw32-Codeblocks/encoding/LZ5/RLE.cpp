#include "RLE.h"
#include "../LZ5.h"



SFF32_u ProcessRLE(SFF8_u* dst, const SFF8_u* src, SFF32_u &dstpos, SFF32_u &srcpos)
{

    //Process an RLE Packet by ANDing the first byte by 0xC0 and checking if it's 0
    if(src[srcpos] & 0xE0)
        ProcessSRLE(dst, src, dstpos, srcpos);
    else ProcessLRLE(dst, src, dstpos, srcpos);
    return 0;
}

SFF32_u ProcessSRLE(SFF8_u* dst, const SFF8_u* src, SFF32_u &dstpos, SFF32_u &srcpos)
{
    for (int run = 0 ; run < (src[srcpos] >> 5); run ++)
    {
        dst[dstpos] = (src[srcpos] & 0x1F);
        dstpos++;
    }
    srcpos++;
    return 0;
}

SFF32_u ProcessLRLE(SFF8_u* dst, const SFF8_u* src, SFF32_u &dstpos, SFF32_u &srcpos)
{
    for (int run = 0 ; run < (src[srcpos + 1] + 8)  ; run++)
    {
        dst[dstpos] = (src[srcpos] & 0x1F);
        dstpos++;
    }
    srcpos += 2;
    return 0;
}
