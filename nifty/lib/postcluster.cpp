#include "postcluster.h"

const int PostCluster::FrequencyCutoff = 300;
const double PostCluster::ClusterSourceOverlapThreshold = 0.8;
const int PostCluster::BucketSize = 2;
const int PostCluster::SlidingWindowSize = 1;
const int PostCluster::PeakThreshold = 5;

void PostCluster::GetTopFilteredClusters(TDocBase *DB, TQuoteBase *QB, LogOutput& Log, TVec<TCluster>& SortedClusters, TVec<TCluster>& TopFilteredClusters, TSecTm PresentTime) {
  GetTopClusters(SortedClusters, TopFilteredClusters);
  MergeClustersBasedOnSubstrings(QB, TopFilteredClusters);
  MergeClustersWithCommonSources(QB, TopFilteredClusters);
  FilterAndCacheClusterPeaks(DB, QB, Log, TopFilteredClusters);

  // sort by popularity
  // Sort remaining clusters by popularity
  fprintf(stderr, "Sorting by popularity...\n");
  for (int i = 0; i < TopFilteredClusters.Len(); i++) {
    //fprintf(stderr, "Calculating popularity for quote %d of %d\n", i, TopFilteredClusters.Len());
    TopFilteredClusters[i].CalculatePopularity(QB, DB, PresentTime);
  }
  TopFilteredClusters.SortCmp(TCmpTClusterByPopularity(false));
  fprintf(stderr, "Done!\n");
}

void PostCluster::GetTopClusters(TVec<TCluster>& SortedClusters, TVec<TCluster>& TopClusters) {
  int NumClusters = SortedClusters.Len();
  for (int i = 0; i < NumClusters; ++i) {
    if (SortedClusters[i].GetNumQuotes() < FrequencyCutoff) {
      i = NumClusters; // Clusters are sorted so we can stop adding to TopClusters now
    } else {
      TopClusters.Add(SortedClusters[i]);
    }
  }
}

/// Merges pairs of clusters if any quote of one is a substring of a quote
//  of the other cluster's
void PostCluster::MergeAllClustersBasedOnSubstrings(TQuoteBase *QB, TClusterBase *CB) {

  // Hash cluster-ids into buckets based on the word-shingles of the quotes in the cluster
  THash<TMd5Sig, TIntSet> Shingles;
  LSH::HashShinglesOfClusters(&QB, &CB, LSH::ShingleWordLen, Shingles, false);
  


}

/// Compare all pairs of clusters with frequency cutoff above the threshold
//  FrequencyCutoff, and merges the pair of clusters if any quote of one is
//  a substring of a quote of the other cluster's
//  (Assumes ClusterSummaries contains clusters sorted by decreasing frequency)
void PostCluster::MergeClustersBasedOnSubstrings(TQuoteBase *QB, TVec<TCluster>& TopClusters) {
  fprintf(stderr, "Merging clusters\n");
  TVec<TInt> ToSkip;  // Contains ids of clusters that have already been merged into another
  TInt NumClusters = TopClusters.Len();

  for (int i = 0; i < NumClusters; i++) {
  // Assumption: the requirements for merging two clusters are transitive
    if (ToSkip.SearchForw(i) >= 0) continue;
    for (int j = i + 1; j < NumClusters; j++) {
      // Compare all the quotes of the two clusters to check if one is
      // a substring of another
      TIntV QuoteIds1;
      TIntV QuoteIds2;
      TopClusters[i].GetQuoteIds(QuoteIds1);
      TopClusters[j].GetQuoteIds(QuoteIds2);

      bool DoMerge = false;
      for (int q1 = 0; q1 < QuoteIds1.Len(); q1++) {
        for (int q2 = 0; q2 < QuoteIds2.Len(); q2++) {
          if (QB->IsSubstring(QuoteIds1[q1], QuoteIds2[q2])) {
            DoMerge = true;
            break;
          }
        }
        if (DoMerge) { break; }
      }

      if (DoMerge) {
        TopClusters[i].MergeWithCluster(TopClusters[j], QB, true);

        ToSkip.Add(j);

        // For testing, print out which two clusters were merged:
        TStr RepQuoteStr1, RepQuoteStr2;
        TopClusters[i].GetRepresentativeQuoteString(RepQuoteStr1, QB);
        TopClusters[j].GetRepresentativeQuoteString(RepQuoteStr2, QB);
        fprintf(stderr, "Merged cluster %s into %s\n", RepQuoteStr2.CStr(), RepQuoteStr1.CStr());
      }
    }
  }

  // delete merged clusters
  ToSkip.Sort(true);
  for (int i = 0; i < ToSkip.Len(); ++i) {
    TStr RepQuoteStr;
    TopClusters[ToSkip[i] - i].GetRepresentativeQuoteString(RepQuoteStr, QB);
    TopClusters.Del(ToSkip[i] - i); // -i in order to keep track of deleted count changes.
  }

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

void PostCluster::MergeClustersWithCommonSources(TQuoteBase* QB, TVec<TCluster>& TopClusters) {
  fprintf(stderr, "Merging clusters with common sources\n");
  int NumClusters = TopClusters.Len();
  TVec<TInt> ToSkip;
  for (int i = 1; i < NumClusters; ++i) {
    TIntV QuoteIds;
    TopClusters[i].GetQuoteIds(QuoteIds);
    TIntV UniqueSources;
    UniqueSources.Sort(true);
    TCluster::GetUniqueSources(UniqueSources, QuoteIds, QB);
    for (int j = 0; j < i; ++j) {
      TIntV MoreQuoteIds;
      TopClusters[j].GetQuoteIds(MoreQuoteIds);
      TIntV MoreUniqueSources;
      TCluster::GetUniqueSources(MoreUniqueSources, MoreQuoteIds, QB);
      MoreUniqueSources.Sort(true);
      double Overlap = ComputeClusterSourceOverlap(MoreUniqueSources, UniqueSources);
      if (Overlap > ClusterSourceOverlapThreshold) {
        TStr RepQuoteStr1, RepQuoteStr2;
        TopClusters[i].GetRepresentativeQuoteString(RepQuoteStr1, QB);
        TopClusters[j].GetRepresentativeQuoteString(RepQuoteStr2, QB);
        fprintf(stderr, "CLUSTER1: %s\nCLUSTER2: %s\n", RepQuoteStr2.CStr(), RepQuoteStr1.CStr());

        TopClusters[j].MergeWithCluster(TopClusters[i], QB, false);
        ToSkip.Add(i);
        break; // we really only want to merge once.
      }
    }
  }

  fprintf(stderr, "deleting extra clusters...\n");
  // delete merged clusters
  ToSkip.Sort(true);
  for (int i = 0; i < ToSkip.Len(); ++i) {
    TStr RepQuoteStr;
    TopClusters[ToSkip[i] - i].GetRepresentativeQuoteString(RepQuoteStr, QB);
    TopClusters.Del(ToSkip[i] - i); // -i in order to keep track of deleted count changes.
  }
  fprintf(stderr, "done!\n");
}


void PostCluster::FilterAndCacheClusterPeaks(TDocBase *DB, TQuoteBase *QB, LogOutput& Log, TVec<TCluster>& TopClusters) {
  fprintf(stderr, "Filtering clusters that have too many peaks...\n");
  TIntV DiscardedClusterIds;
  TVec<TCluster> DiscardedClusters;
  for (int i = 0; i < TopClusters.Len(); ++i) {
    TFreqTripleV PeakTimesV;
    TFreqTripleV FreqV;
    TopClusters[i].GetPeaks(DB, QB, PeakTimesV, FreqV, BucketSize, SlidingWindowSize, TSecTm(0), true);
    // Add clusters with too many peaks to the discard list.
    if (PeakTimesV.Len() > PeakThreshold) {
      DiscardedClusterIds.Add(i);
      DiscardedClusters.Add(TopClusters[i]);
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
