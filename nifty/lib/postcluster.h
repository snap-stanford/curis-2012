#ifndef postcluster_h
#define postcluster_h

#include "stdafx.h"
#include "quote.h"
#include "doc.h"
#include "logoutput.h"

class PostCluster {
private:
  static double ComputeClusterSourceOverlap(TIntV& Larger, TIntV& Smaller);

  static bool ShouldMergeClusters(TQuoteBase *QB, TCluster& Cluster1, TCluster& Cluster2);

public:
  static const int FrequencyCutoff;
  static const double ClusterSourceOverlapThreshold;
  static const int BucketSize;
  static const int SlidingWindowSize;
  static const int PeakThreshold;

  static void GetTopFilteredClusters(TDocBase *DB, TQuoteBase *QB, LogOutput& Log, TVec<TCluster>& SortedClusters, TVec<TCluster>& TopFilteredClusters, TSecTm PresentTime);
  static void GetTopClusters(TVec<TCluster>& SortedClusters, TVec<TCluster>& TopClusters);
  static void MergeAllClustersBasedOnSubstrings(TQuoteBase *QB, TClusterBase *CB);
  static void MergeClustersBasedOnSubstrings(TQuoteBase *QB, TVec<TCluster>& TopClusters);
  static void MergeClustersWithCommonSources(TQuoteBase* QB, TVec<TCluster>& TopClusters);
  static void FilterAndCacheClusterPeaks(TDocBase *DB, TQuoteBase *QB, LogOutput& Log, TVec<TCluster>& TopClusters);

};

#endif
