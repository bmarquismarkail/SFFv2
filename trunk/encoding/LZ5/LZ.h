#ifndef LZ5LZ
#define LZ5LZ

#include "..\..\utiltype\sff2int.h"

SFF32_u ProcessLZ(SFF8_u* dst, const SFF8_u* src, SFF32_u &dstpos, SFF32_u &srcpos, SFF8_u &recyclebyte, SFF8_u &recyclecount);

SFF32_u ProcessSLZ(SFF8_u* dst, const SFF8_u* src, SFF32_u &dstpos, SFF32_u &srcpos, SFF8_u &recyclebyte, SFF8_u &recyclecount);
SFF32_u ProcessLLZ(SFF8_u* dst, const SFF8_u* src, SFF32_u &dstpos, SFF32_u &srcpos);

SFF32_u pos_compare(const SFF8_u* lhs, const SFF8_u* rhs, SFF32_u count);
#endif // LZ5LZ
