#include "stdafx.h"
#include "dataloader.h"

void TIncrementalClustering::BuildClusters(TVec<TIntV>& MergedClusters, TVec<TCluster>& ClusterSummaries,
                                           TQuoteBase& QB, TDocBase& DB, TIntV& NewQuotes) {
  for (int i = 0; i < ClusterSummaries.Len(); i++) {
    TIntV QuoteIds;
    ClusterSummaries[i].GetQuoteIds(QuoteIds);
    MergedClusters.Add(QuoteIds);
  }
  for (int i = 0; i < NewQuotes.Len(); i++) {
    TQuote NewQ;
    QB.GetQuote(NewQuotes[i], NewQ);
    for (int j = 0; j < ClusterSummaries.Len(); j++) {
      TIntV QuoteIds;
      ClusterSummaries[j].GetQuoteIds(QuoteIds);
      int NumSimilar = 0;
      for (int k = 0; k < QuoteIds.Len(); k++) {
        TQuote Q;
        QB.GetQuote(QuoteIds[k], Q);
        if (QuoteGraph::EdgeShouldBeCreated(NewQ, Q)) {
          NumSimilar++;
        }
      }
      if (5 * NumSimilar >= 4 * QuoteIds.Len()) {
        MergedClusters[j].Add(NewQuotes[i]);
        break;
      }
    }
  }
  return;
}
