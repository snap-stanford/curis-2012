#include "stdafx.h"
#include <stdio.h>

int main(int argc, char *argv[]) {
  TQuoteBase QB;
  TDocBase DB;
  TClusterBase CB;
  LogOutput Log;

  // LOAD CLUSTERS AND LOG FILE
  fprintf(stderr, "Loading CB\n");
  TFIn ClusterFile("output/clusters.bin");
  CB.Load(ClusterFile);
  Log.Load(ClusterFile);
  fprintf(stderr, "Clusters loaded!\n");

  bool DoIncrementalClustering = false;
  THash<TStr, TStr> Arguments;
  TStr BaseString;
  ArgumentParser::ParseArguments(argc, argv, Arguments, Log, BaseString, DoIncrementalClustering);

  fprintf(stderr, "Loading QB and DB from file...\n");
  TSecTm PresentTime = TDataLoader::LoadBulkQBDB("/lfs/1/tmp/curis/QBDB/", BaseString, QB, DB);
  fprintf(stderr, "Done!\n");

  Log.SetupFiles(); // safe to make files now.
  TIntV TopFilteredClusters;
  PostCluster::GetTopFilteredClusters(&CB, &DB, &QB, Log, TopFilteredClusters, PresentTime);

  // TODO: consider if quote is dead?

  // OUTPUT
  //Log.SetupFiles(); // safe to make files now.
  Log.OutputClusterInformation(&DB, &QB, &CB, TopFilteredClusters, PresentTime);
  Log.WriteClusteringOutputToFile();

  // plot output
  ClusterPlot Plotter(TStr("/lfs/1/tmp/curis/"));
  Plotter.PlotClusterSizeUnique(&CB);
  Plotter.PlotClusterSize(&CB);
  Plotter.PlotQuoteFrequencies(&QB);

  printf("Done!\n");
  return 0;
}
