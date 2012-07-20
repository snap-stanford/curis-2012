#ifndef cluster_h
#define cluster_h

#include "stdafx.h"
#include "quote.h"
#include "peaks.h"

class TCluster {
private:
  TIntV RepresentativeQuoteIds;
  TInt NumQuotes;
  TIntV QuoteIds;
  TInt Id;

public:
  TCluster();
  TCluster(TIntV& RepresentativeQuoteIds, TInt NumQuotes, const TIntV QuoteIds);
  TCluster(TSIn& SIn) : RepresentativeQuoteIds(SIn), NumQuotes(SIn), QuoteIds(SIn), Id(SIn){ }
  void Save(TSOut& SOut) const;
  void Load(TSIn& SIn);
  void GetRepresentativeQuoteIds(TIntV& RepQuoteIds) const;
  TInt GetNumRepresentativeQuoteIds() const;
  void GetRepresentativeQuoteString(TStr& RepStr, TQuoteBase *QB) const;
  TInt GetNumQuotes() const;
  TInt GetNumUniqueQuotes() const;
  void GetQuoteIds(TIntV &QuoteIds) const;
  TInt GetId();
  void SetId(TInt Id);

  void AddQuote(TQuoteBase *QB, const TIntV &QuoteIds);
  void AddQuote(TQuoteBase *QB, TInt QuoteId);
  void SetRepresentativeQuoteIds(TIntV& QuoteIds);

  void GetPeaks(TDocBase *DocBase, TQuoteBase *QuoteBase, TFreqTripleV& PeakTimesV, TFreqTripleV& FreqV, TInt BucketSize, TInt SlidingWindowSize);
  void GraphFreqOverTime(TDocBase *DocBase, TQuoteBase *QuoteBase, TStr Filename);
  void GraphFreqOverTime(TDocBase *DocBase, TQuoteBase *QuoteBase, TStr Filename, TInt BucketSize, TInt SlidingWindowSize);

  static void MergeClusters(TCluster& MergedCluster, TCluster& Cluster1, TCluster& Cluster2, TQuoteBase *QB);
  static void GetUniqueSources(TIntV& UniqueSources, TIntV& QuoteIds, TQuoteBase *QB);
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
