#ifndef analyzer_h
#define analyzer_h

#include "stdafx.h"
#include "logoutput.h"

class DQuote;

class DCluster {
public:
  TSecTm Start;
  TSecTm End;
  TInt DiffDay;
  TInt Unique;
  TInt Size;
  TInt NumPeaks;
  TInt PopStrLen;
  TStr RepStr;
  TStr RepURL;
  TSecTm First;
  TSecTm Last;
  TSecTm Peak;
  TBool Archived; // TODO: calculate if should be discarded
  TVec<DQuote> Quotes;

  DCluster(TStr LineInput);
  static TStr GetClusterString(TQuoteBase *QB, TDocBase *DB, TCluster& C, TFreqTripleV &FreqV, TInt NumPeaks, TStr &End);
  static void GetFMP(TFreqTripleV& FreqV, TSecTm& First, TSecTm& Last, TSecTm& Peak);
};

class DQuote {
public:
  TInt Size;
  TInt NumPeaks;
  TInt StrLen;
  TStr Str;
  TStr RepURL;
  TSecTm First;
  TSecTm Last;
  TSecTm Peak;

  static TStr GetQuoteString(TDocBase *DB, TQuote& Quote, TSecTm &PresentTime);
};

#endif
