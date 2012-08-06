#include "postcluster.h"
#include "lsh.h"
#include <assert.h>

const double PostCluster::ClusterSourceOverlapThreshold = 0.8;
const int PostCluster::BucketSize = 2;
const int PostCluster::SlidingWindowSize = 1;
const int PostCluster::PeakThreshold = 5;
const int PostCluster::DayThreshold = 3;
const int PostCluster::QuoteThreshold = 20;

void PostCluster::GetTopFilteredClusters(TClusterBase *CB, TDocBase *DB, TQuoteBase *QB, LogOutput& Log, TIntV& TopFilteredClusters, TSecTm PresentTime) {
  RemoveOldClusters(QB, DB, CB, PresentTime);
  CB->GetTopClusterIdsByFreq(TopFilteredClusters);
  MergeAllClustersBasedOnSubstrings(QB, TopFilteredClusters, CB);
  MergeClustersBasedOnSubstrings(QB, TopFilteredClusters, CB);
  //MergeClustersWithCommonSources(QB, TopFilteredClusters, CB);
  FilterAndCacheClusterPeaks(DB, QB, CB, Log, TopFilteredClusters, PresentTime);

  // sort by popularity
  // Sort remaining clusters by popularity
  TVec<TPair<TInt, TFlt> > PopularityVec;
  fprintf(stderr, "Sorting by popularity...\n");
  for (int i = 0; i < TopFilteredClusters.Len(); i++) {
    //fprintf(stderr, "Calculating popularity for quote %d of %d\n", i, TopFilteredClusters.Len());
    TCluster C;
    CB->GetCluster(TopFilteredClusters[i], C);
    TFlt Score = C.GetPopularity(QB, DB, PresentTime);
    PopularityVec.Add(TPair<TInt, TFlt>(TopFilteredClusters[i], Score));
  }
  PopularityVec.SortCmp(TCmpTClusterByPopularity(false));

  TIntV TopFilteredClustersByPopularity;
  for (int i = 0; i < PopularityVec.Len(); i++) {
    TopFilteredClustersByPopularity.Add(PopularityVec[i].Val1);
  }
  TopFilteredClusters = TopFilteredClustersByPopularity;
  fprintf(stderr, "Done!\n");
}

/// Merges pairs of clusters if any quote of one is a substring of a quote
//  of the other cluster's. Only merges into the clusters in TopFilterdClusters
void PostCluster::MergeAllClustersBasedOnSubstrings(TQuoteBase *QB, TIntV& TopClusters, TClusterBase *CB) {
  fprintf(stderr, "Merging all clusters based on substrings\n");
  // Hash cluster-ids into buckets based on the word-shingles of the quotes in the cluster
  THash<TMd5Sig, TIntV> TopClustersShingles;
  LSH::HashShinglesOfClusters(QB, CB, TopClusters, LSH::ShingleWordLen, TopClustersShingles);

  // Check all clusters not in TopClusters for possible merging
  TIntV AllClusterIds;
  CB->GetAllClusterIds(AllClusterIds);
  TIntSet TopClustersSet(TopClusters);
  TIntSet ToRemove;

  for (int i = 0; i < AllClusterIds.Len(); i++) {
    if (i % 1000 == 0) {
      fprintf(stderr, "%d out of %d checked for merge\n", i, AllClusterIds.Len());
    }
    if (TopClustersSet.IsKey(AllClusterIds[i])) { continue; }
    TCluster CurrCluster;
    CB->GetCluster(AllClusterIds[i], CurrCluster);
    THashSet<TMd5Sig> ClusterShingles;
    LSH::GetHashedShinglesOfCluster(QB, CurrCluster, LSH::ShingleWordLen, ClusterShingles);
    TIntSet AlreadyCompared;
    bool FoundMatch = false;
    // Compare this cluster to the top clusters that share one or more of the cluster's shingles
    for (THashSet<TMd5Sig>::TIter Shingle = ClusterShingles.BegI(); Shingle < ClusterShingles.EndI(); Shingle++) {
      if (TopClustersShingles.IsKey(*Shingle)) {
        TIntV TopClustersSubset = TopClustersShingles.GetDat(*Shingle);
        for (TIntV::TIter Id = TopClustersSubset.BegI(); Id < TopClustersSubset.EndI(); Id++) {
          if (AlreadyCompared.IsKey(*Id)) { continue; }
          TCluster TopCluster;
          CB->GetCluster(*Id, TopCluster);

          // For testing purposes
          TStr TopClusterStr;
          TopCluster.GetRepresentativeQuoteString(TopClusterStr, QB);
          //fprintf(stderr, "\t%s\n", TopClusterStr.CStr());


          if (ShouldMergeClusters(QB, TopCluster, CurrCluster)) {
            CB->MergeCluster2Into1(TopCluster.GetId(), CurrCluster.GetId(), QB, true);
            FoundMatch = true;

            // For testing, print out which two clusters were merged:
            TStr RepQuoteStr1, RepQuoteStr2;
            TopCluster.GetRepresentativeQuoteString(RepQuoteStr1, QB);
            CurrCluster.GetRepresentativeQuoteString(RepQuoteStr2, QB);
            fprintf(stderr, "Merged cluster %s INTO %s\n", RepQuoteStr2.CStr(), RepQuoteStr1.CStr());
            break;
          }
          AlreadyCompared.AddKey(*Id);
        }
      }
      if (FoundMatch) {
        ToRemove.AddKey(AllClusterIds[i]);
        break;  // Only want to merge with one top cluster
      }
    }
  }

  // Remove from the ClusterBase those clusters that were merged into another
  for (THashSet<TInt>::TIter Id = ToRemove.BegI(); Id < ToRemove.EndI(); Id++) {
    CB->RemoveCluster(*Id);
  }
}

/// Compares all quotes of the two clusters to check if one is a substring of another;
//  returns true if that is the case, false otherwise.
bool PostCluster::ShouldMergeClusters(TQuoteBase *QB, TCluster& Cluster1, TCluster& Cluster2) {
  TIntV QuoteIds1;
  TIntV QuoteIds2;
  Cluster1.GetQuoteIds(QuoteIds1);
  Cluster2.GetQuoteIds(QuoteIds2);

  for (int q1 = 0; q1 < QuoteIds1.Len(); q1++) {
    for (int q2 = 0; q2 < QuoteIds2.Len(); q2++) {
      if (QB->IsSubstring(QuoteIds1[q1], QuoteIds2[q2])) {
        return true;
      }
    }
  }
  return false;
}

/// Compare all pairs of clusters with frequency cutoff above the threshold
//  FrequencyCutoff, and merges the pair of clusters if any quote of one is
//  a substring of a quote of the other cluster's
//  (Assumes ClusterSummaries contains clusters sorted by decreasing frequency)
void PostCluster::MergeClustersBasedOnSubstrings(TQuoteBase *QB, TIntV &TopClusters, TClusterBase *CB) {
  fprintf(stderr, "Merging only top clusters\n");
  TIntSet ToSkip;  // Contains ids of clusters that have already been merged into another
  TInt NumClusters = TopClusters.Len();

  for (int i = 0; i < NumClusters; i++) {
  // Assumption: the requirements for merging two clusters are transitive
    if (ToSkip.IsKey(TopClusters[i])) { continue; }
    TCluster Ci;
    CB->GetCluster(TopClusters[i], Ci);
    for (int j = i + 1; j < NumClusters; j++) {
      if (ToSkip.IsKey(TopClusters[j])) { continue; }
      TCluster Cj;
      CB->GetCluster(TopClusters[j], Cj);
      if (ShouldMergeClusters(QB, Ci, Cj)) {
        CB->MergeCluster2Into1(TopClusters[i], TopClusters[j], QB, true);
        ToSkip.AddKey(TopClusters[j]);

        // For testing, print out which two clusters were merged:
        TStr RepQuoteStr1, RepQuoteStr2;
        Ci.GetRepresentativeQuoteString(RepQuoteStr1, QB);
        Cj.GetRepresentativeQuoteString(RepQuoteStr2, QB);
        fprintf(stderr, "Merged cluster %s INTO %s\n", RepQuoteStr2.CStr(), RepQuoteStr1.CStr());
        break;
      }
    }
  }

  // delete merged clusters and remove from ClusterBase
  for (TIntSet::TIter Id = ToSkip.BegI(); Id < ToSkip.EndI(); Id++) {
    CB->RemoveCluster(*Id);
  }
  TIntV NewTopClusters;
  for (int i = 0; i < TopClusters.Len(); i++) {
    if (!ToSkip.IsKey(TopClusters[i])) {
      NewTopClusters.Add(TopClusters[i]);
    }
  }
  TopClusters = NewTopClusters;

  fprintf(stderr, "merged clusters\n");
}

// typical walkthrough function as covered in CS276.
double PostCluster::ComputeClusterSourceOverlap(TIntV& Larger, TIntV& Smaller) {
  int Li = 0;
  int Si = 0;
  int count = 0;
  while (Li < Larger.Len() && Si < Smaller.Len()) {
    if (Larger[Li] == Smaller[Si]) {
      ++count;
      ++Li;
      ++Si;
    } else if(Larger[Li] > Smaller[Si]) {
      ++Si;
    } else {
      ++Li;
    }
  }
  return count * 1.0 / Smaller.Len();
}

void PostCluster::MergeClustersWithCommonSources(TQuoteBase* QB, TIntV& TopClusters, TClusterBase *CB) {
  fprintf(stderr, "Merging clusters with common sources\n");
  int NumClusters = TopClusters.Len();
  TIntSet ToSkip;  // Contains cluster ids
  for (int i = 0; i < NumClusters; i++) {
    if (ToSkip.IsKey(TopClusters[i])) { continue; }
    TCluster Ci;
    CB->GetCluster(TopClusters[i], Ci);
    TIntV QuoteIds;
    Ci.GetQuoteIds(QuoteIds);
    TIntV UniqueSources;
    TCluster::GetUniqueSources(UniqueSources, QuoteIds, QB);
    UniqueSources.Sort(true);
    for (int j = i + 1; j < NumClusters; j++) {
      if (ToSkip.IsKey(TopClusters[j])) { continue; }
      TCluster Cj;
      CB->GetCluster(TopClusters[j], Cj);
      TIntV MoreQuoteIds;
      Cj.GetQuoteIds(MoreQuoteIds);
      TIntV MoreUniqueSources;
      TCluster::GetUniqueSources(MoreUniqueSources, MoreQuoteIds, QB);
      MoreUniqueSources.Sort(true);
      double Overlap;
      if (MoreUniqueSources.Len() < UniqueSources.Len()) {
        Overlap = ComputeClusterSourceOverlap(UniqueSources, MoreUniqueSources);
      } else {
        Overlap = ComputeClusterSourceOverlap(MoreUniqueSources, UniqueSources);
      }
      if (Overlap > ClusterSourceOverlapThreshold) {
        TStr RepQuoteStr1, RepQuoteStr2;
        Ci.GetRepresentativeQuoteString(RepQuoteStr1, QB);
        Cj.GetRepresentativeQuoteString(RepQuoteStr2, QB);
        //fprintf(stderr, "CLUSTER1: %s\nCLUSTER2: %s\n", RepQuoteStr2.CStr(), RepQuoteStr1.CStr());

        CB->MergeCluster2Into1(TopClusters[i], TopClusters[j], QB, false);
        ToSkip.AddKey(TopClusters[j]);

        // FOR TESTING
        /*TCluster NewC;
        CB->GetCluster(TopClusters[i], NewC);
        TIntV NewQuoteIds;
        NewC.GetQuoteIds(NewQuoteIds);
        for (int k = 0; k < NewQuoteIds.Len(); k++) {
          TQuote NewQ;
          QB->GetQuote(NewQuoteIds[k], NewQ);
          TStr NewQStr;
          NewQ.GetContentString(NewQStr);
          fprintf(stderr, "\t%s\n", NewQStr.CStr());
        }
        fprintf(stderr, "\n\n"); */

        break; // we really only want to merge once.
      }
    }
  }

  fprintf(stderr, "deleting extra clusters...\n");
  // delete merged clusters and remove from ClusterBase
  for (TIntSet::TIter Id = ToSkip.BegI(); Id < ToSkip.EndI(); Id++) {
    CB->RemoveCluster(*Id);
  }
  TIntV NewTopClusters;
  for (int i = 0; i < TopClusters.Len(); i++) {
    if (!ToSkip.IsKey(TopClusters[i])) {
      NewTopClusters.Add(TopClusters[i]);
    }
  }
  TopClusters = NewTopClusters;
  fprintf(stderr, "done!\n");
}


void PostCluster::FilterAndCacheClusterPeaks(TDocBase *DB, TQuoteBase *QB, TClusterBase *CB, LogOutput& Log, TIntV& TopClusters, TSecTm& PresentTime) {
  fprintf(stderr, "Filtering clusters that have too many peaks...\n");
  TIntSet DiscardedClusterIds;  // Contains id's of clusters to be discarded
  TVec<TCluster> DiscardedClusters;
  for (int i = 0; i < TopClusters.Len(); ++i) {
    TCluster C;
    CB->GetCluster(TopClusters[i], C);
    TFreqTripleV PeakTimesV;
    TFreqTripleV FreqV;
    C.GetPeaks(DB, QB, PeakTimesV, FreqV, BucketSize, SlidingWindowSize, TSecTm(0), true);

    //fprintf(stderr, "Number of peaks: %d\n", PeakTimesV.Len());
    // Add clusters with too many peaks to the discard list.
    if (PeakTimesV.Len() > PeakThreshold) {
      DiscardedClusterIds.AddKey(TopClusters[i]);
      DiscardedClusters.Add(C);
    }
  }

  fprintf(stderr, "CHECKPOINT 0\n");

  // delete the discarded clusters from the top clusters list.
  TIntV FilteredTopClusters;
  for (int i = 0; i < TopClusters.Len(); ++i) {
    if (!DiscardedClusterIds.IsKey(TopClusters[i])) {
      FilteredTopClusters.Add(TopClusters[i]);
    }
  }
  TopClusters = FilteredTopClusters;

  fprintf(stderr, "Logging discarded clusters...\n");

  // log the discarded clusters in a log file.
  Log.OutputDiscardedClusters(QB, DiscardedClusters, PresentTime);
  fprintf(stderr, "Done!\n");
}

/// Remove clusters whose quotes have fewer than QuoteThreshold sources (total) for the last three days
void PostCluster::RemoveOldClusters(TQuoteBase *QB, TDocBase *DB, TClusterBase *CB, TSecTm PresentTime) {
  TIntV AllClusterIds;
  CB->GetAllClusterIds(AllClusterIds);

  for (int i = 0; i < AllClusterIds.Len(); i++) {
    TCluster C;
    CB->GetCluster(AllClusterIds[i], C);
    TIntV ClusterQuoteIds;
    C.GetQuoteIds(ClusterQuoteIds);
    TIntSet AllSources;
    for (int j = 0; j < ClusterQuoteIds.Len(); j++) {
      TQuote Q;
      QB->GetQuote(ClusterQuoteIds[j], Q);
      TIntV QSources;
      Q.GetSources(QSources);
      AllSources.AddKeyV(QSources);
    }

    TInt NumRecentSources = 0;
    // Round PresentTime to the nearest day afterward
    TUInt PresentTimeI = TUInt(PresentTime.GetAbsSecs());
    PresentTimeI = TUInt(uint(PresentTimeI / Peaks::NumSecondsInDay + 1) * Peaks::NumSecondsInDay);  // round to next 12am

    TUInt ThresholdTime = PresentTimeI - DayThreshold * Peaks::NumSecondsInDay;
    for (TIntSet::TIter SourceId = AllSources.BegI(); SourceId < AllSources.EndI(); SourceId++) {
      TDoc Doc;
      DB->GetDoc(*SourceId, Doc);
      if (Doc.GetDate().GetAbsSecs() >= ThresholdTime) {
        NumRecentSources += 1;
      }
    }

    // If the number of recent sources does not pass the threshold, remove that cluster and all its quotes
    if (NumRecentSources < QuoteThreshold) {
      for (int j = 0; j < ClusterQuoteIds.Len(); j++) {
        QB->RemoveQuote(ClusterQuoteIds[j]);
      }
      CB->RemoveCluster(AllClusterIds[i]);
    }
  }
  return;
}
