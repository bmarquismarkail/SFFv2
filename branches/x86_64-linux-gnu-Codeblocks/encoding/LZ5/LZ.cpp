#include "../LZ5.h"
#include "LZ.h"


SFF32_u ProcessLZ(SFF8_u* dst, const SFF8_u* src, SFF32_u &dstpos, SFF32_u &srcpos, SFF8_u &recyclebyte, SFF8_u &recyclecount)
{

    //Process an LZ Packet by ANDing the first byte by 0x3F.
    if(src[srcpos] & 0x3F)
        //if that equation is nonzero, it is a short LZ packet
        ProcessSLZ(dst, src, dstpos, srcpos, recyclebyte, recyclecount);
        //else it is a long LZ packet.
    else ProcessLLZ(dst, src, dstpos, srcpos);

    return 0;
}


SFF32_u ProcessSLZ(SFF8_u* dst, const SFF8_u* src, SFF32_u &dstpos, SFF32_u &srcpos ,SFF8_u &recyclebyte, SFF8_u &recyclecount)
{
    //Process a short LZ packet.
    recyclebyte |= ((src[srcpos] & 0xC0) >>  ( recyclecount * 2 ));
    recyclecount++;
    //the answer used to determine the short packet is the copy length.
    //check and see if this is the forth short LZ packet being processed.
    if(recyclecount == 4)
    {
        //if so, the recycle buffer has the offset of decompressed data for copying.
        LZ_cpy(dst, dstpos , (recyclebyte + 1 ) , (src[srcpos] & 0x3F) + 1);
        srcpos++ ;
        recyclecount = 0;
        recyclebyte = 0;
    }
    //else read another byte. that is the offset of decompressed data for copying.
    else
    {
        LZ_cpy(dst, dstpos, (src[srcpos + 1] + 1) , (src[srcpos] & 0x3F) + 1);
        srcpos += 2;
    }
    return 0;
}

SFF32_u ProcessLLZ(SFF8_u* dst, const SFF8_u* src, SFF32_u &dstpos, SFF32_u &srcpos)
{
    //Process a long LZ packet.
    //the byte read before ANDed by 0xC0 is the top 2 bits of the offset
    //read another byte. That is the bottom 8 bits of the offset.
    SFF16_u offset = ((src[srcpos] & 0xC0) << 2) | (src[srcpos + 1]);
    //read one more byte. That is the copy length.
    //Now copy using the offset - 3
    LZ_cpy(dst,dstpos, offset + 1, (src[srcpos + 2]) + 3);
    srcpos += 3;
    return 0;
}

SFF32_u pos_compare(const SFF8_u* lhs, const SFF8_u* rhs, SFF32_u count)
{
    for(SFF32_u x = 0; x < count; x++)
		if (lhs[x] != rhs[x]) return count - x;
	return 0;
}
