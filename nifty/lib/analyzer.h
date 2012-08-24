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

//##### COMPARISON FUNCTIONS YO
class TCmpDQuoteBySize {
private:
  bool IsAsc;
public:
  TCmpDQuoteBySize(const bool& AscSort=true) : IsAsc(AscSort) { }
  bool operator () (const DQuote& D1, const DQuote& D2) const {
    if (IsAsc) {
      return D1.Size < D2.Size;
    } else {
      return D2.Size > D1.Size;
    }
  }
};

class TCmpDQuoteByPeak {
private:
  bool IsAsc;
public:
  TCmpDQuoteByPeak(const bool& AscSort=true) : IsAsc(AscSort) { }
  bool operator () (const DQuote& D1, const DQuote& D2) const {
    if (IsAsc) {
      return D1.Peak < D2.Peak;
    } else {
      return D2.Peak > D1.Peak;
    }
  }
};

class TCmpDQuoteByFirst {
private:
  bool IsAsc;
public:
  TCmpDQuoteByFirst(const bool& AscSort=true) : IsAsc(AscSort) {}
  bool operator () (const DQuote& D1, const DQuote& D2) const {
    if (IsAsc) {
      return D1.First < D2.First;
    } else {
      return D2.First > D1.First;
    }
  }
};

#endif
