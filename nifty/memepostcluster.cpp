#include "stdafx.h"
#include <stdio.h>

const int FrequencyCutoff = 300;
const double ClusterSourceOverlapThreshold = 0.8;
const int BucketSize = 2;
const int SlidingWindowSize = 1;
const int PeakThreshold = 5;

void FilterAndCacheClusterPeaks(TDocBase *DB, TQuoteBase *QB, LogOutput& Log, TVec<TCluster>& TopClusters) {
  TIntV DiscardedClusterIds;
  TVec<TCluster> DiscardedClusters;
  for (int i = 0; i < TopClusters.Len(); ++i) {
    TFreqTripleV PeakTimesV;
    TFreqTripleV FreqV;
    TopClusters[i].GetPeaks(DB, QB, PeakTimesV, FreqV, BucketSize, SlidingWindowSize, TSecTm(0));
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

  // log the discarded clusters in a log file.
  Log.OutputDiscardedClusters(QB, DiscardedClusters);
}

/// Compare all pairs of clusters with frequency cutoff above the threshold
//  FrequencyCutoff, and merges the pair of clusters if any quote of one is
//  a substring of a quote of the other cluster's
//  (Assumes ClusterSummaries contains clusters sorted by decreasing frequency)
void MergeClustersBasedOnSubstrings(TQuoteBase *QB, TVec<TCluster>& MergedTopClusters,
                                    TVec<TCluster>& ClusterSummaries, TInt FrequencyCutoff) {
  fprintf(stderr, "Merging clusters\n");
  TVec<TInt> ToSkip;  // Contains ids of clusters that have already been merged into another
  TInt NumClusters = ClusterSummaries.Len();

  // Set all cluster id's to their index in the sorted ClusterSummaries vector
  for (int i = 0;  i < NumClusters && ClusterSummaries[i].GetNumQuotes() >= FrequencyCutoff; i++) {
    ClusterSummaries[i].SetId(TInt(i));
  }

  for (int i = 0; i < NumClusters && ClusterSummaries[i].GetNumQuotes() >= FrequencyCutoff; i++) {
  // Assumption: the requirements for merging two clusters are transitive
    if (ToSkip.SearchForw(ClusterSummaries[i].GetId()) >= 0) continue;
    TCluster CurrentCluster = ClusterSummaries[i];  // Other clusters may be merged into this one
    for (int j = i + 1; j < NumClusters && ClusterSummaries[j].GetNumQuotes() >= FrequencyCutoff; j++) {
      // Compare all the quotes of the two clusters to check if one is
      // a substring of another
      TIntV QuoteIds1;
      TIntV QuoteIds2;
      CurrentCluster.GetQuoteIds(QuoteIds1);
      ClusterSummaries[j].GetQuoteIds(QuoteIds2);

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
        CurrentCluster.MergeWithCluster(ClusterSummaries[j], QB, true);
        
        ToSkip.Add(ClusterSummaries[j].GetId());

        // For testing, print out which two clusters were merged:
        TStr RepQuoteStr1, RepQuoteStr2;
        CurrentCluster.GetRepresentativeQuoteString(RepQuoteStr1, QB);
        ClusterSummaries[j].GetRepresentativeQuoteString(RepQuoteStr2, QB);
        fprintf(stderr, "Merged cluster %s into %s\n", RepQuoteStr2.CStr(), RepQuoteStr1.CStr());
      }
    }
    MergedTopClusters.Add(CurrentCluster);
  }

  fprintf(stderr, "merged clusters\n");
}

// typical walkthrough function as covered in CS276.
double ComputeClusterSourceOverlap(TIntV& Larger, TIntV& Smaller) {
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

void MergeClustersWithCommonSources(TQuoteBase* QB, TVec<TCluster>& TopClusters) {
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

  // delete merged clusters
  ToSkip.Sort(true);
  for (int i = 0; i < ToSkip.Len(); ++i) {
    TStr RepQuoteStr;
    TopClusters[ToSkip[i] - i].GetRepresentativeQuoteString(RepQuoteStr, QB);
    TopClusters.Del(ToSkip[i] - i); // -i in order to keep track of deleted count changes.
  }
}

void GetTopClusters(TVec<TCluster>& SortedClusters, TVec<TCluster>& TopClusters) {
  int NumClusters = SortedClusters.Len();
  for (int i = 0; i < NumClusters; ++i) {
    if (SortedClusters[i].GetNumQuotes() < FrequencyCutoff) {
      i = NumClusters; // Clusters are sorted so we can stop adding to TopClusters now
    } else {
      TopClusters.Add(SortedClusters[i]);
    }
  }
}

int main(int argc, char *argv[]) {
  LogOutput Log;

  // LOAD CLUSTERS AND LOG FILE
  fprintf(stderr, "loading clusters\n");
  TVec<TIntV> Clusters;
  TVec<TCluster> ClusterSummaries;
  TFIn ClusterFile("output/clusters.bin");
  Clusters.Load(ClusterFile);
  ClusterSummaries.Load(ClusterFile);
  TCluster tmp = ClusterSummaries[0];
  Log.Load(ClusterFile);
  fprintf(stderr, "Clusters loaded!\n");

  THash<TStr, TStr> Arguments;
  TStr BaseString;
  ArgumentParser::ParseArguments(argc, argv, Arguments, Log, BaseString);

  fprintf(stderr, "Loading QB and DB from file...\n");
  TQuoteBase *QB = new TQuoteBase;
  TDocBase *DB = new TDocBase;
  TSecTm PresentTime = TDataLoader::LoadQBDB("/lfs/1/tmp/curis/QBDB/", BaseString, *QB, *DB);
  fprintf(stderr, "Done!\n");



  // Cull the cluster listing so we are only dealing with the top few clusters.
  TVec<TCluster> TopClusters;
  GetTopClusters(ClusterSummaries, TopClusters);

  // Merge clusters whose subquotes are encompassed by parent quotes.
  TVec<TCluster> MergedTopClusters;
  MergeClustersBasedOnSubstrings(QB, MergedTopClusters, TopClusters, FrequencyCutoff);
  // Merge clusters who share many similar sources.
  MergeClustersWithCommonSources(QB, MergedTopClusters);

  // Calculate and cache cluster peaks and frequency
  Log.SetupFiles(); // safe to make files now.
  FilterAndCacheClusterPeaks(DB, QB, Log, MergedTopClusters);

  // Sort remaining clusters by popularity
  for (int i = 0; i < MergedTopClusters.Len(); i++) {
    MergedTopClusters[i].CalculatePopularity(QB, DB, PresentTime);
  }
  MergedTopClusters.SortCmp(TCmpTClusterByPopularity(false));

  // TODO: consider if quote is dead?

  // OUTPUT
  //Log.SetupFiles(); // safe to make files now.
  Log.OutputClusterInformation(DB, QB, MergedTopClusters, PresentTime);
  Log.WriteClusteringOutputToFile();

  // plot output
  ClusterPlot Plotter(TStr("/lfs/1/tmp/curis/"));
  Plotter.PlotClusterSizeUnique(Clusters);
  Plotter.PlotClusterSize(ClusterSummaries);
  Plotter.PlotQuoteFrequencies(QB);

  delete QB;
  delete DB;
  printf("Done!\n");
  return 0;
}
