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
  void GetTopClusters(TVec<TCluster>& SortedClusters, TVec<TCluster>& TopClusters);
  void MergeClustersBasedOnSubstrings(TQuoteBase *QB, TVec<TCluster>& MergedTopClusters,
                                      TVec<TCluster>& ClusterSummaries, TInt FrequencyCutoff);
  void MergeClustersWithCommonSources(TQuoteBase* QB, TVec<TCluster>& TopClusters);
  static void FilterAndCacheClusterPeaks(TDocBase *DB, TQuoteBase *QB, LogOutput& Log, TVec<TCluster>& TopClusters);

};

#endif
