#ifndef postcluster_h
#define postcluster_h

#include "stdafx.h"
#include "quote.h"
#include "doc.h"
#include "logoutput.h"

class PostCluster {
private:
  static double ComputeClusterSourceOverlap(TIntV& Larger, TIntV& Smaller);

public:
  static const int FrequencyCutoff;
  static const double ClusterSourceOverlapThreshold;
  static const int BucketSize;
  static const int SlidingWindowSize;
  static const int PeakThreshold;

  static void GetTopFilteredClusters(TClusterBase *CB, TDocBase *DB, TQuoteBase *QB, LogOutput& Log, TVec<TCluster>& TopFilteredClusters, TSecTm PresentTime);
  static void MergeAllClustersBasedOnSubstrings(TQuoteBase *QB, TVec<TCluster>& AllClusters);
  static void MergeClustersBasedOnSubstrings(TQuoteBase *QB, TVec<TCluster>& TopClusters);
  static void MergeClustersWithCommonSources(TQuoteBase* QB, TVec<TCluster>& TopClusters);
  static void FilterAndCacheClusterPeaks(TDocBase *DB, TQuoteBase *QB, LogOutput& Log, TVec<TCluster>& TopClusters);

};

#endif
