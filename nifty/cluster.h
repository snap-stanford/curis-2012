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
  TInt GetRepresentativeQuoteId();
  TInt GetNumQuotes();
  TInt GetNumUniqueQuotes();
  void GetQuoteIds(TIntV &QuoteIds);

  void AddQuote(TQuoteBase *QB, const TIntV &QuoteIds);
  void AddQuote(TQuoteBase *QB, TInt QuoteId);
  void SetRepresentativeQuoteId(TInt QuoteId);
};

#endif
