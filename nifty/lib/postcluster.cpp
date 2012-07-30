#include "postcluster.h"
#include <assert.h>

const double PostCluster::ClusterSourceOverlapThreshold = 0.8;
const int PostCluster::BucketSize = 2;
const int PostCluster::SlidingWindowSize = 1;
const int PostCluster::PeakThreshold = 5;

void PostCluster::GetTopFilteredClusters(TClusterBase *CB, TDocBase *DB, TQuoteBase *QB, LogOutput& Log, TIntV& TopFilteredClusters, TSecTm PresentTime) {
  CB->GetTopClusterIdsByFreq(TopFilteredClusters);
  //MergeAllClustersBasedOnSubstrings(QB, CB);
  MergeClustersBasedOnSubstrings(QB, TopFilteredClusters, CB);
  MergeClustersWithCommonSources(QB, TopFilteredClusters, CB);
  FilterAndCacheClusterPeaks(DB, QB, CB, Log, TopFilteredClusters);

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
  fprintf(stderr, "Done!\n");
}

/// Merges pairs of clusters if any quote of one is a substring of a quote
//  of the other cluster's
void PostCluster::MergeAllClustersBasedOnSubstrings(TQuoteBase *QB, TClusterBase *CB) {
  // Hash cluster-ids into buckets based on the word-shingles of the quotes in the cluster
  THash<TMd5Sig, TIntV> Shingles;
  LSH::HashShinglesOfClusters(QB, CB, LSH::ShingleWordLen, Shingles);

  // Check all clusters in the same bucket for substring merge
  // Keep track of compared clusters, so we don't compare the same pair of clusters more than once
  THashSet<TIntPr> ComparedClusters;  // in each pair of cluster ids, the first must be smaller
  TIntSet ToSkip;  // Contains ids of clusters have have already been merged into another

  for (THash<TMd5Sig, TIntV>::TIter Shingle = Shingles.BegI(); Shingle < Shingles.EndI(); Shingle++) {
    TIntV ClusterIds = Shingle.GetDat();

    // For testing, print out the quotes in each bucket
    for (int i = 0; i < ClusterIds.Len(); i++) {
      TCluster Cluster;
      CB->GetCluster(ClusterIds[i], Cluster);
      TStr ClusterRepQuote;
      Cluster.GetRepresentativeQuoteString(ClusterRepQuote, QB);
      fprintf(stderr, "\t%s\n", ClusterRepQuote.CStr());
    }
    fprintf(stderr, "\n\n");

    ClusterIds.Sort(true);
    for (int i = 0; i < ClusterIds.Len(); i++) {

      for (int j = i + 1; j < ClusterIds.Len(); j++) {
        if (ComparedClusters.IsKey(TIntPr(ClusterIds[i], ClusterIds[j]))) { continue; }
        ComparedClusters.AddKey(TIntPr(ClusterIds[i], ClusterIds[j]));
        TCluster Cluster1, Cluster2;
        CB->GetCluster(ClusterIds[i], Cluster1);
        CB->GetCluster(ClusterIds[j], Cluster2);
        if (ShouldMergeClusters(QB, Cluster1, Cluster2)) {
          CB->MergeCluster2Into1(ClusterIds[i], ClusterIds[j], QB, true);
          ToSkip.AddKey(ClusterIds[j]);

          // For testing, print out which two clusters were merged:
          TStr RepQuoteStr1, RepQuoteStr2;
          Cluster1.GetRepresentativeQuoteString(RepQuoteStr1, QB);
          Cluster2.GetRepresentativeQuoteString(RepQuoteStr2, QB);
          fprintf(stderr, "Merged cluster %s INTO %s\n", RepQuoteStr2.CStr(), RepQuoteStr1.CStr());
        }
      }
    }
  }

  // Remove from the ClusterBase those clusters that were merged into another
  for (THashSet<TInt>::TIter Id = ToSkip.BegI(); Id < ToSkip.EndI(); Id++) {
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
  fprintf(stderr, "Merging clusters\n");
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
      double Overlap = ComputeClusterSourceOverlap(MoreUniqueSources, UniqueSources);
      if (Overlap > ClusterSourceOverlapThreshold) {
        TStr RepQuoteStr1, RepQuoteStr2;
        Ci.GetRepresentativeQuoteString(RepQuoteStr1, QB);
        Cj.GetRepresentativeQuoteString(RepQuoteStr2, QB);
        fprintf(stderr, "CLUSTER1: %s\nCLUSTER2: %s\n", RepQuoteStr2.CStr(), RepQuoteStr1.CStr());

        CB->MergeCluster2Into1(TopClusters[i], TopClusters[j], QB, false);
        ToSkip.AddKey(TopClusters[j]);
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


void PostCluster::FilterAndCacheClusterPeaks(TDocBase *DB, TQuoteBase *QB, TClusterBase *CB, LogOutput& Log, TIntV& TopClusters) {
  fprintf(stderr, "Filtering clusters that have too many peaks...\n");
  TIntV DiscardedClusterIds;
  TVec<TCluster> DiscardedClusters;
  for (int i = 0; i < TopClusters.Len(); ++i) {
    TCluster C;
    CB->GetCluster(TopClusters[i], C);
    TFreqTripleV PeakTimesV;
    TFreqTripleV FreqV;
    C.GetPeaks(DB, QB, PeakTimesV, FreqV, BucketSize, SlidingWindowSize, TSecTm(0), true);
    // Add clusters with too many peaks to the discard list.
    if (PeakTimesV.Len() > PeakThreshold) {
      DiscardedClusterIds.Add(i);
      DiscardedClusters.Add(C);
    }
  }

  // delete the discarded clusters from the top clusters list.
  for (int i = 0; i < DiscardedClusterIds.Len(); ++i) {
    TopClusters.Del(DiscardedClusterIds[i] - i); // -i in order to keep track of deleted count changes.
  }
  fprintf(stderr, "Logging discarded clusters...\n");

  // log the discarded clusters in a log file.
  Log.OutputDiscardedClusters(QB, DiscardedClusters);
  fprintf(stderr, "Done!\n");
}
