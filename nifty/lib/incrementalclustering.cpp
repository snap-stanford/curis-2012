#include "stdafx.h"
#include "incrementalclustering.h"

const TInt TIncrementalClustering::DayThreshold = 3;
const TInt TIncrementalClustering::QuoteThreshold = 20;

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

/// Remove clusters whose quotes have fewer than QuoteThreshold sources (total) for the last three days
void TIncrementalClustering::RemoveOldClusters(TVec<TIntV>& NewMergedClusters, TVec<TIntV>& MergedClusters,
                                               TQuoteBase& QB, TDocBase& DB, TSecTm PresentTime) {
  for (int i = 0; i < MergedClusters.Len(); i++) {
    TIntV ClusterQuotes = MergedClusters[i];
    TInt NumRecentSources = 0;
    TIntSet AllSources;
    for (int j = 0; j < ClusterQuotes.Len(); j++) {
      TQuote Q;
      QB.GetQuote(ClusterQuotes[j], Q);
      TIntV QSources;
      Q.GetSources(QSources);
      AllSources.AddKeyV(QSources);
    }

    TUInt ThresholdTime = PresentTime.GetAbsSecs() - DayThreshold * Peaks::NumSecondsInDay;
    for (TIntSet::TIter SourceId = AllSources.BegI(); SourceId < AllSources.EndI(); SourceId++) {
      TDoc Doc;
      DB.GetDoc(*SourceId, Doc);
      if (Doc.GetDate().GetAbsSecs() >= ThresholdTime) {
        NumRecentSources += 1;
      }
    }

    if (NumRecentSources >= QuoteThreshold) {
      NewMergedClusters.Add(ClusterQuotes);
    }
  }
  return;
}
