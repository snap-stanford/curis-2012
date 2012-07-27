#include "stdafx.h"
#include "incrementalclustering.h"

const TInt TIncrementalClustering::DayThreshold = 3;
const TInt TIncrementalClustering::QuoteThreshold = 20;

void TIncrementalClustering::BuildClusters(TVec<TIntV>& MergedClusters, TVec<TCluster>& ClusterSummaries,
                                           TQuoteBase& QB, TDocBase& DB, TIntV& NewQuotes) {
  THashSet<Int> NewQuotesSet;
  for (int i = 0; i < NewQuotes.Len(); i++) {
    NewQuotesSet.AddKey(NewQuotes[i]);
  }

  THash<TMd5Sig, TIntSet> Shingles;
  LSH::HashShingles(&QB, LSH::ShingleLen, Shingles);
  TVec<THash<TIntV, TIntSet> > BucketsVector;
  LSH::MinHash(Shingles, BucketsVector);

  for (int i = 0; i < BucketsVector.Len(); i++) {
    printf("Processing band signature %d of %d\n", i+1, BucketsVector.Len());
    TVec<TIntV> Buckets;
    BucketsVector[i].GetKeyV(Buckets);
    TVec<TIntV>::TIter BucketEnd = Buckets.EndI();
    for (TVec<TIntV>::TIter BucketSig = Buckets.BegI(); BucketSig < BucketEnd; BucketSig++) {
      TIntSet Bucket  = BucketsVector[i].GetDat(*BucketSig);
      TVec<TQuote> NQuotes, OQuotes;
      for (TIntSet::TIter Quote = Bucket.BegI(); Quote < Bucket.EndI(); Quote++) {
        TInt QuoteId = Quote.GetKey();
        if (NewQuotesSet.IsKey(QuoteId)) {
          NQuotes.Add(QuoteId);
        } else {
          OQuotes.Add(QuoteId);
        }
      }
      for (int j = 0; j < NQuotes.Len(); j++) {
        TQuote NewQ;
        QB.GetQuote(NQuotes[j], NewQ);
        for (int k = 0; k < OQuotes.Len(); k++) {
          TQuote Q;
          QB.GetQuote(OQuotes[j], Q);
          if (QuoteGraph::EdgeShouldBeCreated(NewQ, Q)) {
            // add newQ to cluster of Q
          }
        }
      }
    }
  }

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
        TQuote RepQuote;
        Clustering::CalcRepresentativeQuote(RepQuote, MergedClusters[j], &QB);
        TStr RepQuoteStr;
        RepQuote.GetContentString(RepQuoteStr);
        fprintf(stderr, "2: %s\n", RepQuoteStr.CStr());
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
