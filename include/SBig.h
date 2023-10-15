#ifndef __STORMLIBX_SBIG_H__
#define __STORMLIBX_SBIG_H__

#include "StormTypes.h"


DECLARE_HANDLE(HSBIGNUM);


//@601
void STORMAPI SBigAdd(HSBIGNUM result, HSBIGNUM a, HSBIGNUM b);

//@602
void STORMAPI SBigAnd(HSBIGNUM result, HSBIGNUM a, HSBIGNUM b);

//@603
int STORMAPI SBigCompare(HSBIGNUM a, HSBIGNUM b);

//@604
void STORMAPI SBigCopy(HSBIGNUM dst, HSBIGNUM src);

//@605
void STORMAPI SBigDec(HSBIGNUM result, HSBIGNUM num);

//@606
void STORMAPI SBigDel(HSBIGNUM bignum);

//@607
void STORMAPI SBigDiv(HSBIGNUM result, HSBIGNUM dividend, HSBIGNUM divisor);

//@608
//void STORMAPI SBigFindPrime(int, int, int, int);

//@609
void STORMAPI SBigFromBinary(HSBIGNUM result, LPBYTE buffer, DWORD buffersize);

//@610
void STORMAPI SBigFromStr(HSBIGNUM result, LPCSTR string);

//@611
//void STORMAPI SBigFromStream(HSBIGNUM result, int, int, int);

//@612
void STORMAPI SBigFromUnsigned(HSBIGNUM result, unsigned value);

//@613
void STORMAPI SBigGcd(HSBIGNUM result, HSBIGNUM a, HSBIGNUM b);

//@614
void STORMAPI SBigInc(HSBIGNUM result, HSBIGNUM bignum);

//@615
void STORMAPI SBigInvMod(HSBIGNUM result, HSBIGNUM dividend, HSBIGNUM divisor);

//@616
BOOL STORMAPI SBigIsEven(HSBIGNUM bignum);

//@617
BOOL STORMAPI SBigIsOdd(HSBIGNUM bignum);

//@618
BOOL STORMAPI SBigIsOne(HSBIGNUM bignum);

//@619
BOOL STORMAPI SBigIsPrime(HSBIGNUM bignum);

//@620
BOOL STORMAPI SBigIsZero(HSBIGNUM bignum);

//@621
void STORMAPI SBigMod(HSBIGNUM result, HSBIGNUM bignum, HSBIGNUM modulus);

//@622
void STORMAPI SBigMul(HSBIGNUM result, HSBIGNUM a, HSBIGNUM b);

//@623
void STORMAPI SBigMulMod(HSBIGNUM result, HSBIGNUM a, HSBIGNUM b, HSBIGNUM modulus);

//@624
void STORMAPI SBigNew(HSBIGNUM* output);

//@625
void STORMAPI SBigNot(HSBIGNUM result, HSBIGNUM input);

//@626
void STORMAPI SBigOr(HSBIGNUM result, HSBIGNUM input);

//@627
void STORMAPI SBigPow(HSBIGNUM result, HSBIGNUM input, HSBIGNUM power);

//@628
void STORMAPI SBigPowMod(HSBIGNUM result, HSBIGNUM input, HSBIGNUM power, HSBIGNUM modulus);

//@629
void STORMAPI SBigRand(HSBIGNUM result, HSBIGNUM a, HSBIGNUM b);

//@630
void STORMAPI SBigSet2Exp(HSBIGNUM result, HSBIGNUM exponent);

//@631
void STORMAPI SBigSetOne(HSBIGNUM result);

//@632
void STORMAPI SBigSetZero(HSBIGNUM result);

//@633
void STORMAPI SBigShl(HSBIGNUM result, HSBIGNUM number, unsigned shift);

//@634
void STORMAPI SBigShr(HSBIGNUM result, HSBIGNUM number, unsigned shift);

//@635
void STORMAPI SBigSquare(HSBIGNUM result, HSBIGNUM number);

//@636
void STORMAPI SBigSub(HSBIGNUM result, HSBIGNUM a, HSBIGNUM b);

//@637
//void STORMAPI SBigToBinaryArray(HSBIGNUM number, int, int);

//@638
void STORMAPI SBigToBinaryBuffer(HSBIGNUM number, LPBYTE buffer, DWORD buffersize, DWORD* resultsize);

//@639
//void STORMAPI SBigToBinaryPtr(HSBIGNUM number, int, int);

//@640
//void STORMAPI SBigToStrArray(HSBIGNUM number, int, int);

//@641
void STORMAPI SBigToStrBuffer(HSBIGNUM number, char *dest, size_t destsize);

//@642
void STORMAPI SBigToStrPtr(HSBIGNUM number, char **ptr);

//@643
//void STORMAPI SBigToStreamArray(HSBIGNUM number, int, int);

//@644
//void STORMAPI SBigToStreamBuffer(HSBIGNUM number, int, int, int);

//@645
//void STORMAPI SBigToStreamPtr(HSBIGNUM number, int, int);

//@646
void STORMAPI SBigToUnsigned(HSBIGNUM number, unsigned *result);

//@647
void STORMAPI SBigXor(HSBIGNUM result, HSBIGNUM a, HSBIGNUM b);


class SBigNum {
public:
  HSBIGNUM num;

  SBigNum() {
    SBigNew(&num);
  }

  SBigNum(LPBYTE buffer, DWORD buffersize) {
    SBigNew(&num);
    SBigFromBinary(num, buffer, buffersize);
  }

  ~SBigNum() {
    SBigDel(num);
  }
};


#endif
