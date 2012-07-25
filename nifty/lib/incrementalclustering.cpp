#include "stdafx.h"
#include "dataloader.h"

void TIncrementalClustering::BuildClusters(TVec<TIntV>& MergedClusters, TVec<TCluster>& ClusterSummaries,
                                           TQuoteBase& QB, TDocBase& DB, TIntV& NewQuotes) {
  return;
}

/// Remove clusters whose quotes have fewer than five sources (total) per day for the last three days
void TIncrementalClustering::RemoveOldClusters(TVec<TIntV>& MergedClusters, TQuoteBase& QB, TDocBase& DB) {
  return;
}
