#ifndef LZ5RLE
#define LZ5RLE

#include "../../utiltype/sff2int.h"

SFF32_u ProcessRLE(SFF8_u* dst, const SFF8_u* src, SFF32_u &dstpos, SFF32_u &srcpos);

SFF32_u ProcessSRLE(SFF8_u* dst, const SFF8_u* src, SFF32_u &dstpos, SFF32_u &srcpos);
SFF32_u ProcessLRLE(SFF8_u* dst, const SFF8_u* src, SFF32_u &dstpos, SFF32_u &srcpos);

#endif // LZ5RLE
