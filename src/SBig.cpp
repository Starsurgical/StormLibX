#include "SBig.h"
#include <storm/Big.hpp>


//@601
void STORMAPI SBigAdd(HSBIGNUM result, HSBIGNUM a, HSBIGNUM b) {
  ImplWrapSBigAdd(reinterpret_cast<BigData*>(result), reinterpret_cast<BigData*>(a), reinterpret_cast<BigData*>(b));
}

//@602
void STORMAPI SBigAnd(HSBIGNUM result, HSBIGNUM a, HSBIGNUM b) {
  // TODO
  //ImplWrapSBigAnd(reinterpret_cast<BigData*>(result), reinterpret_cast<BigData*>(a), reinterpret_cast<BigData*>(b));
}

//@603
int STORMAPI SBigCompare(HSBIGNUM a, HSBIGNUM b) {
  return ImplWrapSBigCompare(reinterpret_cast<BigData*>(a), reinterpret_cast<BigData*>(b));
}

//@604
void STORMAPI SBigCopy(HSBIGNUM dst, HSBIGNUM src) {
  // TODO
  //ImplWrapSBigCopy(reinterpret_cast<BigData*>(dst), reinterpret_cast<BigData*>(src));
}

//@605
void STORMAPI SBigDec(HSBIGNUM result, HSBIGNUM num) {
  // TODO
  //ImplWrapSBigDec(reinterpret_cast<BigData*>(result), reinterpret_cast<BigData*>(num));
}

//@606
void STORMAPI SBigDel(HSBIGNUM bignum) {
  ImplWrapSBigDel(reinterpret_cast<BigData*>(bignum));
}

//@607
void STORMAPI SBigDiv(HSBIGNUM result, HSBIGNUM dividend, HSBIGNUM divisor) {
  ImplWrapSBigDiv(reinterpret_cast<BigData*>(result), reinterpret_cast<BigData*>(dividend), reinterpret_cast<BigData*>(divisor));
}

//@608
//void STORMAPI SBigFindPrime(int, int, int, int) {
//}

//@609
void STORMAPI SBigFromBinary(HSBIGNUM result, LPBYTE buffer, DWORD buffersize) {
  ImplWrapSBigFromBinary(reinterpret_cast<BigData*>(result), buffer, buffersize);
}

//@610
void STORMAPI SBigFromStr(HSBIGNUM result, LPCSTR string) {
  // TODO
  //ImplWrapSBigFromString(reinterpret_cast<BigData*>(result), reinterpret_cast<BigData*>(string));
}

//@611
//void STORMAPI SBigFromStream(HSBIGNUM result, int, int, int) {
//}

//@612
void STORMAPI SBigFromUnsigned(HSBIGNUM result, unsigned value) {
  ImplWrapSBigFromUnsigned(reinterpret_cast<BigData*>(result), value);
}

//@613
void STORMAPI SBigGcd(HSBIGNUM result, HSBIGNUM a, HSBIGNUM b) {
  // TODO
  //ImplWrapSBigGcd(reinterpret_cast<BigData*>(result), reinterpret_cast<BigData*>(a), reinterpret_cast<BigData*>(b));
}

//@614
void STORMAPI SBigInc(HSBIGNUM result, HSBIGNUM bignum) {
  ImplWrapSBigInc(reinterpret_cast<BigData*>(result), reinterpret_cast<BigData*>(bignum));
}

//@615
void STORMAPI SBigInvMod(HSBIGNUM result, HSBIGNUM dividend, HSBIGNUM divisor) {
  // TODO
  //ImplWrapSBigInvMod(reinterpret_cast<BigData*>(result), reinterpret_cast<BigData*>(dividend), reinterpret_cast<BigData*>(divisor));
}

//@616
BOOL STORMAPI SBigIsEven(HSBIGNUM bignum) {
  return ImplWrapSBigIsEven(reinterpret_cast<BigData*>(bignum));
}

//@617
BOOL STORMAPI SBigIsOdd(HSBIGNUM bignum) {
  return ImplWrapSBigIsOdd(reinterpret_cast<BigData*>(bignum));
}

//@618
BOOL STORMAPI SBigIsOne(HSBIGNUM bignum) {
  return ImplWrapSBigIsOne(reinterpret_cast<BigData*>(bignum));
}

//@619
BOOL STORMAPI SBigIsPrime(HSBIGNUM bignum) {
  // TODO
  //return ImplWrapSBigIsPrime(reinterpret_cast<BigData*>(bignum));
  return FALSE;
}

//@620
BOOL STORMAPI SBigIsZero(HSBIGNUM bignum) {
  return ImplWrapSBigIsZero(reinterpret_cast<BigData*>(bignum));
}

//@621
void STORMAPI SBigMod(HSBIGNUM result, HSBIGNUM bignum, HSBIGNUM modulus) {
  ImplWrapSBigMod(reinterpret_cast<BigData*>(result), reinterpret_cast<BigData*>(bignum), reinterpret_cast<BigData*>(modulus));
}

//@622
void STORMAPI SBigMul(HSBIGNUM result, HSBIGNUM a, HSBIGNUM b) {
  ImplWrapSBigMul(reinterpret_cast<BigData*>(result), reinterpret_cast<BigData*>(a), reinterpret_cast<BigData*>(b));
}

//@623
void STORMAPI SBigMulMod(HSBIGNUM result, HSBIGNUM a, HSBIGNUM b, HSBIGNUM modulus) {
  ImplWrapSBigMulMod(reinterpret_cast<BigData*>(result), reinterpret_cast<BigData*>(a), reinterpret_cast<BigData*>(b), reinterpret_cast<BigData*>(modulus));
}

//@624
void STORMAPI SBigNew(HSBIGNUM* output) {
  ImplWrapSBigNew(reinterpret_cast<BigData**>(output));
}

//@625
void STORMAPI SBigNot(HSBIGNUM result, HSBIGNUM input) {
  // TODO
  //ImplWrapSBigNot(reinterpret_cast<BigData*>(result), reinterpret_cast<BigData*>(input));
}

//@626
void STORMAPI SBigOr(HSBIGNUM result, HSBIGNUM a, HSBIGNUM b) {
  // TODO
  //ImplWrapSBigOr(reinterpret_cast<BigData*>(result), reinterpret_cast<BigData*>(a), reinterpret_cast<BigData*>(b));
}

//@627
void STORMAPI SBigPow(HSBIGNUM result, HSBIGNUM input, HSBIGNUM power) {
  // TODO
  //ImplWrapSBigPow(reinterpret_cast<BigData*>(result), reinterpret_cast<BigData*>(input), reinterpret_cast<BigData*>(power));
}

//@628
void STORMAPI SBigPowMod(HSBIGNUM result, HSBIGNUM input, HSBIGNUM power, HSBIGNUM modulus) {
  ImplWrapSBigPowMod(reinterpret_cast<BigData*>(result), reinterpret_cast<BigData*>(input), reinterpret_cast<BigData*>(power), reinterpret_cast<BigData*>(modulus));
}

//@629
void STORMAPI SBigRand(HSBIGNUM result, HSBIGNUM a, HSBIGNUM b) {
  // TODO
  //ImplWrapSBigRand(reinterpret_cast<BigData*>(result), reinterpret_cast<BigData*>(a), reinterpret_cast<BigData*>(b));
}

//@630
void STORMAPI SBigSet2Exp(HSBIGNUM result, unsigned exponent) {
  //ImplWrapSBigSet2Exp(reinterpret_cast<BigData*>(result), exponent);
}

//@631
void STORMAPI SBigSetOne(HSBIGNUM result) {
  ImplWrapSBigSetOne(reinterpret_cast<BigData*>(result));
}

//@632
void STORMAPI SBigSetZero(HSBIGNUM result) {
  ImplWrapSBigSetZero(reinterpret_cast<BigData*>(result));
}

//@633
void STORMAPI SBigShl(HSBIGNUM result, HSBIGNUM number, unsigned shift) {
  ImplWrapSBigShl(reinterpret_cast<BigData*>(result), reinterpret_cast<BigData*>(number), shift);
}

//@634
void STORMAPI SBigShr(HSBIGNUM result, HSBIGNUM number, unsigned shift) {
  ImplWrapSBigShr(reinterpret_cast<BigData*>(result), reinterpret_cast<BigData*>(number), shift);
}

//@635
void STORMAPI SBigSquare(HSBIGNUM result, HSBIGNUM number) {
  ImplWrapSBigSquare(reinterpret_cast<BigData*>(result), reinterpret_cast<BigData*>(number));
}

//@636
void STORMAPI SBigSub(HSBIGNUM result, HSBIGNUM a, HSBIGNUM b) {
  ImplWrapSBigSub(reinterpret_cast<BigData*>(result), reinterpret_cast<BigData*>(a), reinterpret_cast<BigData*>(b));
}

//@637
//void STORMAPI SBigToBinaryArray(HSBIGNUM number, int, int) {
//}

//@638
void STORMAPI SBigToBinaryBuffer(HSBIGNUM number, LPBYTE buffer, DWORD buffersize, DWORD* resultsize) {
  uint32_t result;
  ImplWrapSBigToBinaryBuffer(reinterpret_cast<BigData*>(number), buffer, buffersize, &result);
  
  if (resultsize) *resultsize = result;
}

//@639
//void STORMAPI SBigToBinaryPtr(HSBIGNUM number, int, int) {
//}

//@640
//void STORMAPI SBigToStrArray(HSBIGNUM number, int, int) {
//}

//@641
void STORMAPI SBigToStrBuffer(HSBIGNUM number, char* dest, size_t destsize) {
  // TODO
  //ImplWrapSBigToStrBuffer(reinterpret_cast<BigData*>(number), dest, destsize);
}

//@642
void STORMAPI SBigToStrPtr(HSBIGNUM number, char** ptr) {
  // TODO
  //ImplWrapSBigToStrPtr(reinterpret_cast<BigData*>(number), ptr);
}

//@643
//void STORMAPI SBigToStreamArray(HSBIGNUM number, int, int) {
//}

//@644
//void STORMAPI SBigToStreamBuffer(HSBIGNUM number, int, int, int) {
//}

//@645
//void STORMAPI SBigToStreamPtr(HSBIGNUM number, int, int) {
//}

//@646
void STORMAPI SBigToUnsigned(HSBIGNUM number, unsigned* result) {
  // TODO
  //ImplWrapSBigToUnsigned(reinterpret_cast<BigData*>(number), result);
}

//@647
void STORMAPI SBigXor(HSBIGNUM result, HSBIGNUM a, HSBIGNUM b) {
  // TODO
  //ImplWrapSBigXor(reinterpret_cast<BigData*>(result), reinterpret_cast<BigData*>(a), reinterpret_cast<BigData*>(b));
}
