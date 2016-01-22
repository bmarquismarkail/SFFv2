#ifndef RLE5
#define RLE5

#include "..\utiltype\sff2int.h"

void RLE5_Decode(SFF8_u* dst, const SFF8_u* src, SFF32_u len);
void RLE5_Encode(SFF8_u* dst, const SFF8_u* src);

#endif // RLE5
