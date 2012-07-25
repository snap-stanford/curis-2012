#ifndef postcluster_h
#define postcluster_h

#include "stdafx.h"
#include "quote.h"
#include "doc.h"
#include "logoutput.h"

class PostCluster {
private:
  double ComputeClusterSourceOverlap(TIntV& Larger, TIntV& Smaller);

public:
  static const int FrequencyCutoff;
  static const double ClusterSourceOverlapThreshold;
  static const int BucketSize;
  static const int SlidingWindowSize;
  static const int PeakThreshold;

  void GetTopFilteredClusters(TDocBase *DB, TQuoteBase *QB, LogOutput& Log, TVec<TCluster>& SortedClusters, TVec<TCluster>& TopFilteredClusters);
  void GetTopClusters(TVec<TCluster>& SortedClusters, TVec<TCluster>& TopClusters);
  void MergeClustersBasedOnSubstrings(TQuoteBase *QB, TVec<TCluster>& TopClusters);
  void MergeClustersWithCommonSources(TQuoteBase* QB, TVec<TCluster>& TopClusters);
  static void FilterAndCacheClusterPeaks(TDocBase *DB, TQuoteBase *QB, LogOutput& Log, TVec<TCluster>& TopClusters);

};

#endif
