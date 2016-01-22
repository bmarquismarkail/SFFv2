#ifndef SFFLZ5
#define SFFLZ5

#include "..\utiltype\sff2int.h"

void LZ5_Decode(SFF8_u* dst, const SFF8_u* src, SFF32_u len);
SFF32_u LZ5_Encode(SFF8_u* dst, const SFF8_u* src, SFF32_u len);

void naivememcpy(SFF8_u* dst, SFF32_u &dstpos , SFF32_u ofs, SFF32_u len);
void LZ_cpy(SFF8_u* dst, SFF32_u &dstpos , SFF32_u ofs, SFF32_u len);


#endif // SFFLZ5
