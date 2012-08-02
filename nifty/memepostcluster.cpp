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

  THash<TStr, TStr> Arguments;
  TStr BaseString;
  ArgumentParser::ParseArguments(argc, argv, Arguments, Log, BaseString);

  fprintf(stderr, "Loading QB and DB from file...\n");
  TSecTm PresentTime = TDataLoader::LoadBulkQBDB("/lfs/1/tmp/curis/QBDB/", BaseString, QB, DB);
  fprintf(stderr, "Done!\n");

  Log.SetupNewOutputDirectory(); // safe to make files now.
  TIntV TopFilteredClusters;
  PostCluster::GetTopFilteredClusters(&CB, &DB, &QB, Log, TopFilteredClusters, PresentTime);

  // TODO: consider if quote is dead?

  // OUTPUT
  //Log.SetupFiles(); // safe to make files now.
  TIntV TempV;
  Log.OutputClusterInformation(&DB, &QB, &CB, TopFilteredClusters, PresentTime, TempV);
  Log.WriteClusteringOutputToFile(PresentTime);

  // plot output
  ClusterPlot Plotter(TStr("/lfs/1/tmp/curis/"));
  Plotter.PlotClusterSizeUnique(&CB);
  Plotter.PlotClusterSize(&CB);
  Plotter.PlotQuoteFrequencies(&QB);

  printf("Done!\n");
  return 0;
}
