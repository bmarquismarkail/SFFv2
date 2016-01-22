#include "LZ5.h"
#include "LZ5/RLE.h"
#include "LZ5/LZ.h"


void LZ5_Decode(SFF8_u* dst, const SFF8_u* src, SFF32_u len)
{
    SFF32_u dstpos = 0;
    SFF32_u srcpos = 4;
    SFF8_u  recbyt = 0;
    SFF8_u  reccount = 0;

    while (srcpos < len)
    {
        //read a control packet and process it bit by bit
        //if the bit is 0, it is an RLE Packet, otherwise it is an LZ packet.
        SFF8_u ctrlbyt = src[srcpos];
        srcpos++;
        for (int b = 0; (b < 8); b++)
        {
            if (!(ctrlbyt & (1 << b)))
                ProcessRLE(dst, src, dstpos, srcpos);
            else ProcessLZ(dst, src, dstpos, srcpos, recbyt, reccount);
            if (srcpos >= len) break;
        }

    }
}

SFF32_u LZ5_Encode(SFF8_u* dst, const SFF8_u* src, SFF32_u len)
{
    SFF32_u dstcount = 0;                   // The size of the encoded string.
    SFF32_u srcpos = 0;                     // Current position of decoded string to be evaluated.
    SFF8_u comparebyt = src[srcpos];        // For RLE strings, the value of the byte to be RLE encoded.
    SFF16_u bytecounter = 1;                // Length of packet
    SFF32_u offset = 0;                     // Offset of packet
    bool LZ = false;                        // Sets if the current packet can be an LZ packet.
    bool RLE = true;                        // Sets if the current packet can be an RLE packet. Depreciated, a packet can always be an RLE or group of RLE packets
    //Are we at EOL?
    while (srcpos <= len)
    {
        srcpos++;
        if( comparebyt == src[srcpos] ) //Search for groups with the same value as comparebyt.
            if ( ( LZ && bytecounter < 258 ) || ( RLE && bytecounter  < 263 ) ) // this ensures that the max length has not been exceeded.
                bytecounter++;
        if ( ( comparebyt !=src[srcpos] ) || (( LZ && bytecounter == 258 ) || ( RLE && bytecounter  == 263 ) ) )  //most likely can optimize this with an else statement
        {
            if(offset >= bytecounter) //If the Bytecounter is greater than the offset, no use trying to search.
            {
                //Check and see if this string can be made using bytes from the back dictionary.
                SFF32_u searchoffset  = offset-bytecounter;                                             //used for searching.
                do
                {
                    int result = pos_compare(src+searchoffset, src+offset, bytecounter);
                    if (!result)
                    {
                        //Set LZ, change the comparebyt, and continue
                        if (!LZ) LZ = true;
                        comparebyt = src[srcpos];
                        break;
                    }
                    else
                    {
                        if (searchoffset != 0)
                            searchoffset -= result;
                        else
                            continue;
                            //process_packet
                    }
                } while (searchoffset >= bytecounter);
            }
            else //bytecounter is greater than or equal to the offset.
            {
                //process_packet
            }
        }

    }
    return dstcount;
}

void naivememcpy(SFF8_u* dst, SFF32_u &dstpos, SFF32_u ofs, SFF32_u len)
{
    for (int run = len; run > 0; run--)
    {
        dst[dstpos] = dst[(dstpos - ofs)];
        dstpos++;
    }
}


void LZ_cpy(SFF8_u* dst, SFF32_u &dstpos, SFF32_u ofs, SFF32_u len)
{
    naivememcpy(dst, dstpos, ofs, len);
}

