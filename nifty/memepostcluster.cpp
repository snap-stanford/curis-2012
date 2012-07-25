#include "stdafx.h"
#include <stdio.h>

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

  Log.SetupFiles(); // safe to make files now.
  TVec<TCluster> TopFilteredClusters;
  PostCluster::GetTopFilteredClusters(DB, QB, Log, ClusterSummaries, TopFilteredClusters, PresentTime);

  // TODO: consider if quote is dead?

  // OUTPUT
  //Log.SetupFiles(); // safe to make files now.
  Log.OutputClusterInformation(DB, QB, TopFilteredClusters, PresentTime);
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
