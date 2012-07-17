#ifndef cluster_h
#define cluster_h

#include "stdafx.h"
#include "quote.h"

class TCluster {
private:
  TInt RepresentativeQuoteId;
  TInt NumQuotes;
  TIntV QuoteIds;
  TIntV Id;

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
  void GetId();
  void SetId();

  void AddQuote(TQuoteBase *QB, const TIntV &QuoteIds);
  void AddQuote(TQuoteBase *QB, TInt QuoteId);
  void SetRepresentativeQuoteId(TInt QuoteId);

  void GetPeaks(TDocBase *DocBase, TQuoteBase *QuoteBase, TVec<TSecTm>& PeakTimesV, TInt BucketSize, TInt SlidingWindowSize);
  void GraphFreqOverTime(TDocBase *DocBase, TQuoteBase *QuoteBase, TStr Filename);
  void GraphFreqOverTime(TDocBase *DocBase, TQuoteBase *QuoteBase, TStr Filename, TInt BucketSize, TInt SlidingWindowSize);
};

class TClusterBase {
private:
  TInt ClusterIdCounter;
  THash<TInt, TQuote> IdToTClusters;
  TInt GetNewQuoteId();

public:
  TClusterBase();
  void Save(TSOut& SOut) const;
  void Load(TSIn& SIn);
  TQuote AddCluster(const TCluster& Cluster);
  void RemoveCluster(TInt ClusterId);
  bool GetCluster(TInt ClusterId, TCluster& RefC);
  void GetAllClusterIds(TIntV &KeyV);
  int Len();
};
};

#endif
