#include "stdafx.h"
#include "incrementalclustering.h"

const TInt TIncrementalClustering::DayThreshold = 3;
const TInt TIncrementalClustering::QuoteThreshold = 20;

void TIncrementalClustering::BuildClusters(TVec<TIntV>& MergedClusters, TVec<TCluster>& ClusterSummaries,
                                           TQuoteBase& QB, TDocBase& DB, TIntV& NewQuotes) {
  // Add all the original clusters to MergedClusters
  for (int i = 0; i < ClusterSummaries.Len(); i++) {
    TIntV QuoteIds;
    ClusterSummaries[i].GetQuoteIds(QuoteIds);
    MergedClusters.Add(QuoteIds);
  }
  for (int i = 0; i < NewQuotes.Len(); i++) {
    TQuote NewQ;
    QB.GetQuote(NewQuotes[i], NewQ);
    TStr NewStr;
    NewQ.GetContentString(NewStr);
    fprintf(stderr, "1: %s\n", NewStr.CStr());
    for (int j = 0; j < MergedClusters.Len(); j++) {
      TIntV QuoteIds = MergedClusters[j];
      int NumSimilar = 0;
      for (int k = 0; k < QuoteIds.Len(); k++) {
        TQuote Q;
        QB.GetQuote(QuoteIds[k], Q);
        if (QuoteGraph::EdgeShouldBeCreated(NewQ, Q)) {
          NumSimilar++;
        }
      }
      // if (5 * NumSimilar >= 4 * QuoteIds.Len()) {
      if (NumSimilar > 0) {
        MergedClusters[j].Add(NewQuotes[i]);
        TStr RepStr;
        MergedClusters[j].GetRepresentativeQuoteString(RepStr, &QB);
        fprintf(stderr, "2: %s\n", RepStr.CStr());
        break;
      }
    }
    // Add new cluster if this new quote does not match to any of the previous clusters
    TIntV NewCluster;
    NewCluster.Add(NewQuotes[i]);
    MergedClusters.Add(NewCluster);
    fprintf(stderr, "%d out of %d new quotes processed\n\n\n", i + 1, NewQuotes.Len());
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
