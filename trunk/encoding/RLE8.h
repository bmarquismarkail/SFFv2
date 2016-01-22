#ifndef RLE8
#define RLE8

#include "..\utiltype\sff2int.h"

void RLE8_Decode(SFF8_u* dst, const SFF8_u* src, SFF32_u len);
void RLE8_Encode(SFF8_u* dst, const SFF8_u* src);

#endif // RLE8
