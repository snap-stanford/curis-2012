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
  TSecTm BirthDate;
  TBool Archived;
  TInt DiscardState; // 0 = clean, 1 = peaks, 2 = variants
  TDateFreq MaxPeakDateFreq;

public:
  TSecTm DeathDate;
  TCluster();
  TCluster(TIntV& RepresentativeQuoteIds, TInt NumQuotes, TIntV QuoteIds, TQuoteBase *QB, TDocBase *DB, TSecTm BirthDate);
  TCluster(TSIn& SIn) : RepresentativeQuoteIds(SIn), NumQuotes(SIn), QuoteIds(SIn), Id(SIn), PeakTimesV(SIn), FreqV(SIn), BirthDate(SIn) , Archived(SIn), DiscardState(SIn), MaxPeakDateFreq(SIn), DeathDate(SIn) { }
  void Save(TSOut& SOut) const;
  void Load(TSIn& SIn);
  void Archive();
  bool IsArchived();
  TInt GetDiscardState();
  void SetDiscardState(TInt State);
  void SetBirthDate(TSecTm& BirthDate);
  void GetBirthDate(TSecTm& BirthDate);
  void GetRepresentativeQuoteIds(TIntV& RepQuoteIds) const;
  TInt GetMostPopularQuoteId(TQuoteBase *QB);
  TInt GetNumRepresentativeQuoteIds() const;
  void GetRepresentativeQuoteString(TStr& RepStr, TQuoteBase *QB) const;
  TInt GetNumQuotes() const;
  TInt GetNumUniqueQuotes() const;
  void GetQuoteIds(TIntV &QuoteIds) const;
  TInt GetId() const;
  TFlt GetPopularity(TQuoteBase *QuoteBase, TDocBase *DocBase, TSecTm CurrentTime);
  void GetRepresentativeQuoteURL(TQuoteBase *QB, TDocBase *DB, TStr& RepURL) const;
  void SetId(TInt Id);
  void GetMaxPeakInfo(TDateFreq& MaxPeakDateFreq);

  void AddQuote(TQuoteBase *QB, TDocBase *DB, const TIntV &QuoteIds);
  void AddQuote(TQuoteBase *QB, TDocBase *DB, TInt QuoteId, bool UpdatePeak = true);
  void SetRepresentativeQuoteIds(TIntV& QuoteIds);

  void SetQuoteIds(TQuoteBase *QB, TDocBase *DB, TIntV& NewQuoteIds);
  //void ReplaceQuote(TQuoteBase *QB, TInt OldQuoteId, TInt NewQuoteId);

  //void GetTopPeak(TDocBase *DocBase, TQuoteBase *QuoteBase, TInt BucketSize, TInt SlidingWindowSize, TSecTm PresentTime, bool reset = false);
  void GetPeaks(TDocBase *DocBase, TQuoteBase *QuoteBase, TFreqTripleV& PeakTimesV, TFreqTripleV& FreqV, TInt BucketSize, TInt SlidingWindowSize, TSecTm PresentTime, bool reset = false);
  void GraphFreqOverTime(TDocBase *DocBase, TQuoteBase *QuoteBase, TStr Filename, TSecTm PresentTime);
  void GraphFreqOverTime(TDocBase *DocBase, TQuoteBase *QuoteBase, TStr Filename, TInt BucketSize, TInt SlidingWindowSize, TSecTm PresentTime);
  TInt GetNumSources(TQuoteBase *QuoteBase);
  TInt GetNumUniqueSources(TQuoteBase *QuoteBase);

  static void GetUniqueSources(TVec<TUInt>& UniqueSources, const TIntV& QuoteIds, TQuoteBase *QB);

  //void GetFreqTimes(TFreqTripleV& FreqV, bool reset = false);
  //void GetPeakTimes(TFreqTripleV& PeakTimesV, bool reset = false);
  //void GetPeakAndFreqTimes(TFreqTripleV& PeakTimesV, TFreqTripleV FreqV, bool reset = false);

private:
  void UpdateMaxPeak(TIntV QuoteIds, TVec<TUInt> UniqueSources, TQuoteBase *QB, TDocBase *DB);
};

class TClusterBase {
private:
  THash<TInt, TCluster> IdToTCluster;
  THash<TInt, TInt> QuoteIdToClusterId;
  TInt ClusterIdCounter;

public:
  static const int FrequencyCutoff;

  TClusterBase();
  TClusterBase(TInt OldCounter);
  void Save(TSOut& SOut) const;
  void Load(TSIn& SIn);
  TInt AddCluster(TCluster& Cluster, const TClusterBase *OldCB, TSecTm& PresentTime);
  TInt AddCluster(TCluster& Cluster);
  void AddStaticCluster(TInt ClusterId, TCluster& Cluster);
  bool AddQuoteToCluster(TQuoteBase *QB, TDocBase *DB, const TIntV& QuoteIds, TInt ClusterId);
  //bool ReplaceQuoteInCluster(TQuoteBase *QB, TInt OldQuoteId, TInt NewQuoteId, TInt ClusterId);
  void RemoveCluster(TInt ClusterId);

  bool GetCluster(TInt ClusterId, TCluster &RefC) const;
  TInt GetClusterIdFromQuoteId(TInt QuoteId) const;
  void GetAllClusterIds(TIntV &ClusterIds) const;
  void GetAllClusterIdsSortByFreq(TIntV &ClusterIds);
  void GetTopClusterIdsByFreq(TIntV &TopClusterIds);
  void Clr();
  int Len();
  void MergeCluster2Into1(TInt Id1, TInt Id2, TQuoteBase *QB, TDocBase *DB, bool KeepOneRepId);
  TInt GetCounter();
  TStr ContainsEmptyClusters();
  bool IsQuoteInArchivedCluster(TInt& QuoteId);
  void SortClustersByPopularity(TDocBase *DB, TQuoteBase *QB, TIntV& Clusters, TSecTm& CurrentTime);
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
  bool operator () (const TPair<TInt, TFlt> P1, const TPair<TInt, TFlt> P2) const {
    if (IsAsc) {
      return P1.Val2 < P2.Val2;
    } else {
      return P2.Val2 < P1.Val2;
    }
  }
};


#endif
