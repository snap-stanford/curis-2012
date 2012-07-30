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
  TFreqTripleV PeakTimesV;
  TFreqTripleV FreqV;

public:
  TCluster();
  TCluster(TIntV& RepresentativeQuoteIds, TInt NumQuotes, TIntV QuoteIds, TQuoteBase *QB);
  TCluster(TSIn& SIn) : RepresentativeQuoteIds(SIn), NumQuotes(SIn), QuoteIds(SIn), Id(SIn), Popularity(SIn), PeakTimesV(SIn), FreqV(SIn){ }
  void Save(TSOut& SOut) const;
  void Load(TSIn& SIn);
  void GetRepresentativeQuoteIds(TIntV& RepQuoteIds) const;
  TInt GetNumRepresentativeQuoteIds() const;
  void GetRepresentativeQuoteString(TStr& RepStr, TQuoteBase *QB) const;
  TInt GetNumQuotes() const;
  TInt GetNumUniqueQuotes() const;
  void GetQuoteIds(TIntV &QuoteIds) const;
  TInt GetId() const;
  TFlt CalculatePopularity(TQuoteBase *QuoteBase, TDocBase *DocBase, TSecTm CurrentTime);
  void SetId(TInt Id);

  void AddQuote(TQuoteBase *QB, const TIntV &QuoteIds);
  void AddQuote(TQuoteBase *QB, TInt QuoteId);
  void SetRepresentativeQuoteIds(TIntV& QuoteIds);

  //void GetTopPeak(TDocBase *DocBase, TQuoteBase *QuoteBase, TInt BucketSize, TInt SlidingWindowSize, TSecTm PresentTime, bool reset = false);
  void GetPeaks(TDocBase *DocBase, TQuoteBase *QuoteBase, TFreqTripleV& PeakTimesV, TFreqTripleV& FreqV, TInt BucketSize, TInt SlidingWindowSize, TSecTm PresentTime, bool reset = false);
  void GraphFreqOverTime(TDocBase *DocBase, TQuoteBase *QuoteBase, TStr Filename, TSecTm PresentTime);
  void GraphFreqOverTime(TDocBase *DocBase, TQuoteBase *QuoteBase, TStr Filename, TInt BucketSize, TInt SlidingWindowSize, TSecTm PresentTime);

  void MergeWithCluster(TCluster& OtherCluster, TQuoteBase *QB, bool KeepOneRepId);
  static void GetUniqueSources(TIntV& UniqueSources, TIntV& QuoteIds, TQuoteBase *QB);

  //void GetFreqTimes(TFreqTripleV& FreqV, bool reset = false);
  //void GetPeakTimes(TFreqTripleV& PeakTimesV, bool reset = false);
  //void GetPeakAndFreqTimes(TFreqTripleV& PeakTimesV, TFreqTripleV FreqV, bool reset = false);
};

class TClusterBase {
private:
  THash<TInt, TCluster> IdToTCluster;
  THash<TInt, TInt> QuoteIdToClusterId;
  TInt ClusterIdCounter;

public:
  static const int FrequencyCutoff;

  TClusterBase();
  void Save(TSOut& SOut) const;
  void Load(TSIn& SIn);
  TInt AddCluster(const TCluster& Cluster);
  bool AddQuoteToCluster(TQuoteBase *QB, TInt QuoteId, TInt ClusterId);

  bool GetCluster(TInt ClusterId, TCluster &RefC);
  TInt GetClusterIdFromQuoteId(TInt QuoteId);
  void GetAllClusterIds(TIntV &ClusterIds);
  void GetAllClusterIdsSortByFreq(TIntV &ClusterIds);
  void GetTopClusterIdsByFreq(TIntV &TopClusterIds);
  void Clr();
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

// Compares TCluster Ids by sum of quote frequencies
class TCmpTClusterIdByNumQuotes {
private:
  bool IsAsc;
  TClusterBase *ClusterBase;
public:
  TCmpTClusterIdByNumQuotes(const bool& AscSort=true, TClusterBase *CB = NULL) : IsAsc(AscSort) {
    ClusterBase = CB;
  }
  bool operator() (const TInt C1, const TInt C2) const {
    TCluster Cluster1, Cluster2;
    ClusterBase->GetCluster(C1, Cluster1);
    ClusterBase->GetCluster(C2, Cluster2);
    if (IsAsc) {
      return Cluster1.GetNumQuotes() < Cluster2.GetNumQuotes();
    } else {
      return Cluster2.GetNumQuotes() < Cluster1.GetNumQuotes();
    }
  }
};

// Compares TClusters by sum of quote frequencies
class TCmpTClusterByPopularity {
private:
  bool IsAsc;
public:
  TCmpTClusterByPopularity(const bool& AscSort=true) : IsAsc(AscSort) { }
  bool operator () (const TCluster& P1, const TCluster& P2) const {
    if (IsAsc) {
      return P1.GetPopularity() < P2.GetPopularity();
    } else {
      return P2.GetPopularity() < P1.GetPopularity();
    }
  }
};


#endif
