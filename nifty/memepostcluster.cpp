#include "stdafx.h"
#include <stdio.h>

const int FrequencyCutoff = 300;
const double ClusterSourceOverlapThreshold = 0.8;
const int BucketSize = 2;
const int SlidingWindowSize = 1;
const int PeakThreshold = 5;

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

  bool DoIncrementalClustering = false;
  THash<TStr, TStr> Arguments;
  TStr BaseString;
  ArgumentParser::ParseArguments(argc, argv, Arguments, Log, BaseString, DoIncrementalClustering);

  fprintf(stderr, "Loading QB and DB from file...\n");
  TQuoteBase *QB = new TQuoteBase;
  TDocBase *DB = new TDocBase;
  TSecTm PresentTime = TDataLoader::LoadBulkQBDB("/lfs/1/tmp/curis/QBDB/", BaseString, *QB, *DB);
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
