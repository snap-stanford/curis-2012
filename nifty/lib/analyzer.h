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
  TInt RepStrLen;
  TStr RepStr;
  TStr RepURL;
  TSecTm First;
  TSecTm Last;
  TSecTm Peak;
  TBool Archived; // TODO: calculate if should be discarded
  TVec<DQuote> Quotes;

  DCluster() {}
  DCluster(TStr LineInput);
  DCluster(TSIn& SIn) : Start(SIn), End(SIn), DiffDay(SIn), Unique(SIn), Size(SIn),
      NumPeaks(SIn), RepStrLen(SIn), RepStr(SIn), RepURL(SIn), First(SIn), Last(SIn),
      Peak(SIn), Archived(SIn), Quotes(SIn) { }
  void Save(TSOut& SOut) const;
  void Load(TSIn& SIn);
  static TStr GetClusterString(TQuoteBase *QB, TDocBase *DB, TCluster& C, TFreqTripleV &FreqV, TInt NumPeaks, TStr &End);
  static void GetFMP(TFreqTripleV& FreqV, TSecTm& First, TSecTm& Last, TSecTm& Peak, TStr &Quote);
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

  DQuote() {}
  DQuote(TStr LineInput);
  DQuote(TSIn& SIn) : Size(SIn), NumPeaks(SIn), StrLen(SIn), Str(SIn),
      RepURL(SIn), First(SIn), Last(SIn), Peak(SIn) { }
  void Save(TSOut& SOut) const;
  void Load(TSIn& SIn);
  static TStr GetQuoteString(TDocBase *DB, TQuote& Quote, TSecTm &PresentTime);
};

#endif
