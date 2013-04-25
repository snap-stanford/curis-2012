#ifndef postcluster_h
#define postcluster_h

#include "stdafx.h"
#include "quote.h"
#include "doc.h"
#include "logoutput.h"

class PostCluster {
private:
  static double ComputeClusterSourceOverlap(TVec<TUInt64>& Larger, TVec<TUInt64>& Smaller);

  static bool ShouldMergeClusters(TQuoteBase *QB, TCluster& Cluster1, TCluster& Cluster2);

public:
  static const double ClusterSourceOverlapThreshold;
  static const int BucketSize;
  static const int SlidingWindowSize;
  static const int PeakThreshold;
  static const int DayThreshold;
  static const int QuoteThreshold;

  static void GetTopFilteredClusters(TClusterBase *CB, TDocBase *DB, TQuoteBase *QB, LogOutput& Log, TIntV& TopFilteredClusters, TSecTm& PresentTime, PNGraph& QGraph, bool RemoveClusters = true);
  static void MergeAllClustersBasedOnSubstrings(TQuoteBase *QB, TIntV& TopClusters, TClusterBase *CB);
  static void MergeClustersBasedOnSubstrings(TQuoteBase *QB, TIntV& TopClusters, TClusterBase *CB);
  static void MergeClustersWithCommonSources(TQuoteBase* QB, TIntV& TopClusters, TClusterBase *CB);
  static void FilterAndCacheClusterPeaks(TDocBase *DB, TQuoteBase *QB, TClusterBase *CB, LogOutput& Log, TIntV &TopClusters, TSecTm& PresentTime);
  static void FilterAndCacheClusterSize(TDocBase *DB, TQuoteBase *QB, TClusterBase *CB, LogOutput& Log, TIntV& TopClusters, TSecTm& PresentTime);
  static void RemoveOldClusters(TQuoteBase *QB, TDocBase *DB, TClusterBase *CB, LogOutput& Log, TSecTm& PresentTime, PNGraph& QGraph);
  static void NukeCluster(TQuoteBase *QB, TClusterBase *CB, TInt ClusterId, TSecTm& PresentTime, PNGraph& QGraph, bool record);
};

#endif
