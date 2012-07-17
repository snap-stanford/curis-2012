#ifndef cluster_h
#define cluster_h

#include "stdafx.h"
#include "quote.h"

class TCluster {
private:
  TInt RepresentativeQuoteId;
  TInt NumQuotes;
  TIntV QuoteIds;

public:
  TCluster();
  TCluster(TInt RepresentativeQuoteId, TInt NumQuotes, const TIntV QuoteIds);
  TCluster(TSIn& SIn) : RepresentativeQuoteId(SIn), NumQuotes(SIn), QuoteIds(SIn) { }
  void Save(TSOut& SOut) const;
  void Load(TSIn& SIn);
  TInt GetRepresentativeQuoteId() const;
  TInt GetNumQuotes() const;
  TInt GetNumUniqueQuotes() const;
  void GetQuoteIds(TIntV &QuoteIds) const;

  void AddQuote(TQuoteBase *QB, const TIntV &QuoteIds);
  void AddQuote(TQuoteBase *QB, TInt QuoteId);
  void SetRepresentativeQuoteId(TInt QuoteId);
};

// Compares TClusters by sum of quote frequencies
class TCmpTClusterByNumQuotes {
private:
  bool IsAsc;
public:
  TCmpTClusterByNumQuotes(const bool& AscSort=true) : IsAsc(AscSort) { }
  bool operator () (const TCluster& P1, const TCluster& P2) const {
    if (IsAsc) {
      return P1.GetNumQuotes() < P2.GetNumQuotes();
    } else {
      return P2.GetNumQuotes() < P1.GetNumQuotes();
    }
  }
};


#endif
