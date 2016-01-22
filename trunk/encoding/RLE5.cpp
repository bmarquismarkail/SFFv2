#include "RLE5.h"


void RLE5_Decode(SFF8_u* dst, const SFF8_u* src, SFF32_u len)
{
    SFF32_u dstpos = 0;
    SFF32_u srcpos = 4;
    SFF8_u  color = 0;
    SFF8_u  bytesused = 0;
    while ( srcpos < len)
    {
        if ( src[srcpos +1 ] &  0x80 )
        {
            color = src[srcpos + 2];
            bytesused = 3;
        }
        else
        {
            color = 0;
            bytesused = 2;
        }
        for(int run = 0; run < (src[srcpos] + 1); run++)
        {
            dst[dstpos] = color;
            dstpos++;
        }
        srcpos+= bytesused;
        int datalength = (src[srcpos - bytesused + 1 ] & 0x7F);
        for (int x = 0; x < datalength; x++)
        {
            for(int run = 0; run < (src[srcpos] >> 5) + 1; run ++ )
            {
                dst[dstpos] = (src[srcpos] & 0x1F);
                dstpos++;
            }
            srcpos++;
        }
    }
}

void RLE5_Encode(SFF8_u* dst, const SFF8_u* src)
{

}
