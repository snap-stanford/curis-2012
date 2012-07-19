#include "stdafx.h"
#include <stdio.h>

const int FrequencyCutoff = 300;
const double ClusterSourceOverlapThreshold = 0.9;

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

  return 0;
}
