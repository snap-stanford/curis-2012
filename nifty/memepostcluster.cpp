#include "stdafx.h"
#include <stdio.h>

const int FrequencyCutoff = 300;
const double ClusterSourceOverlapThreshold = 0.9;

/// Compare all pairs of clusters with frequency cutoff above the threshold
//  FrequencyCutoff, and merges the pair of clusters if any quote of one is
//  a substring of a quote of the other cluster's
//  (Assumes ClusterSummaries contains clusters sorted by decreasing frequency)
void MergeClustersBasedOnSubstrings(TVec<TCluster>& MergedTopClusters, TVec<TCluster>& ClusterSummaries,
                                    TInt FrequencyCutoff, TQuoteBase *QB) {
  fprintf(stderr, "Merging clusters\n");
  TVec<TInt> ToSkip;  // Contains ids of clusters that have already been merged into another

  // Set all cluster id's to their index in the sorted ClusterSummaries vector
  for (int i = 0; ClusterSummaries[i].GetNumQuotes() >= FrequencyCutoff; i++) {
    ClusterSummaries[i].SetId(TInt(i));
  }

  for (int i = 0; ClusterSummaries[i].GetNumQuotes() >= FrequencyCutoff; i++) {
  // Assumption: the requirements for merging two clusters are transitive
    if (ToSkip.SearchForw(ClusterSummaries[i].GetId()) >= 0) continue;
    TCluster CurrentCluster = ClusterSummaries[i];  // Other clusters may be merged into this one
    for (int j = i + 1; ClusterSummaries[j].GetNumQuotes() >= FrequencyCutoff; j++) {
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
        CurrentCluster.MergeWithCluster(ClusterSummaries[j], QB);
        
        ToSkip.Add(ClusterSummaries[j].GetId());

        // For testing, print out which two clusters were merged:
        TQuote RepQuote1, RepQuote2;
        QB->GetQuote(CurrentCluster.GetRepresentativeQuoteId(), RepQuote1);
        QB->GetQuote(ClusterSummaries[j].GetRepresentativeQuoteId(), RepQuote2);
        TStr RepQuoteStr1, RepQuoteStr2;
        RepQuote1.GetContentString(RepQuoteStr1);
        RepQuote2.GetContentString(RepQuoteStr2);
        fprintf(stderr, "Merged cluster %s into %s\n", RepQuoteStr2.CStr(), RepQuoteStr1.CStr());
      }
    }
    MergedTopClusters.Add(CurrentCluster);
  }
}

// typical walkthrough function as covered in CS276.
double ComputeClusterSourceOverlap(TIntV& Larger, TIntV& Smaller) {
  int Li = 0;
  int Si = 0;
  int count = 0;
  while (Li < Larger.Len() && Si < Larger.Len()) {
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
  for (int i = 1; i < NumClusters; ++i) {
    TIntV QuoteIds;
    TopClusters[i].GetQuoteIds(QuoteIds);
    TIntV UniqueSources;
    UniqueSources.Sort(true);
    TCluster::GetUniqueSources(UniqueSources, QuoteIds, QB);
    for (int j = 0; j < i; ++j) {
      TIntV MoreQuoteIds;
      TopClusters[i].GetQuoteIds(MoreQuoteIds);
      TIntV MoreUniqueSources;
      TCluster::GetUniqueSources(MoreUniqueSources, MoreQuoteIds, QB);
      MoreUniqueSources.Sort(true);
      double Overlap = ComputeClusterSourceOverlap(MoreUniqueSources, UniqueSources);
      if (Overlap > ClusterSourceOverlapThreshold) {
        // TODO: merging!
      }
    }
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
  THash<TStr, TStr> Arguments;
  for (int i = 1; i < argc; i++) {
    if (strlen(argv[i]) >= 2 && argv[i][0] == '-' && i + 1 < argc) {
      Arguments.AddDat(TStr(argv[i] + 1), TStr(argv[i + 1]));
      Log.LogValue(TStr(argv[i] + 1), TStr(argv[i + 1]));
      i++;
    } else {
      printf("Error: incorrect format. Usage: ./memepostfilter [-paramName parameter]");
      exit(1);
    }
  }
  // load QB and DB. Custom variables can be added later.
  TStr BaseString = TWOWEEK_DIRECTORY;
  if (Arguments.IsKey("qbdb")) {
    TStr BaseArg = Arguments.GetDat("qbdb");
    if (BaseArg == "week") {
      BaseString = WEEK_DIRECTORY;
    } else if (BaseArg == "day"){
      BaseString = DAY_DIRECTORY;
    }
  }
  if (Arguments.IsKey("nolog")) {
    Log.DisableLogging();
  }

  TFIn BaseFile(BaseString + "QBDB.bin");
  TQuoteBase *QB = new TQuoteBase;
  TDocBase *DB = new TDocBase;
  QB->Load(BaseFile);
  DB->Load(BaseFile);

  fprintf(stderr, "loading clusters\n");

  TVec<TIntV> Clusters;
  TVec<TCluster> ClusterSummaries;
  TFIn ClusterFile(BaseString + "clusters.bin");
  fprintf(stderr, "%s%s\n", BaseString.CStr(), "clusters.bin");
  Clusters.Load(ClusterFile);
  ClusterSummaries.Load(ClusterFile);
  TCluster tmp = ClusterSummaries[0];
  Log.Load(ClusterFile);

  TVec<TCluster> MergedTopClusters;
  MergeClustersBasedOnSubstrings(MergedTopClusters, ClusterSummaries, FrequencyCutoff, QB);

  // OUTPUT
  Log.SetupFiles(); // safe to make files now.
  Log.OutputClusterInformation(DB, QB, ClusterSummaries);
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
