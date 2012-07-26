#include "stdafx.h"
#include <stdio.h>

/// Must pass in argument for the new day to be added, in format YYYY-MM-DD (-newday)
//  TODO: Add framework for Log file (as in memepostcluster.cpp)
int main(int argc, char *argv[]) {
  bool DoIncrementalClustering = false;
  THash<TStr, TStr> Arguments;
  TStr BaseString;
  LogOutput Log;
  Log.SetupFiles();
  ArgumentParser::ParseArguments(argc, argv, Arguments, Log, BaseString, DoIncrementalClustering);

  if (!Arguments.IsKey("newday")) {
    fprintf(stderr, "Must input date of the new day to be added, in the format YYYY-MM-DD (-newday)");
  }
  TStr NewDayDate = Arguments.GetDat("newday");
  TSecTm PresentTime = TSecTm::GetDtTmFromYmdHmsStr(NewDayDate + " 23:59:59");

  fprintf(stderr, "Loading QB and DB from file...\n");
  TQuoteBase QB;
  TDocBase DB;
  TVec<TCluster> ClusterSummaries;
  fprintf(stderr, "Loading cumulative QB, DB, and clusters from file...\n");
  TDataLoader::LoadCumulative("/lfs/1/tmp/curis/QBDBC/", NewDayDate, QB, DB, ClusterSummaries);
  fprintf(stderr, "Done!\n");

  TVec<TCluster> TopFilteredClusters;
  PostCluster::GetTopFilteredClusters(&DB, &QB, Log, ClusterSummaries, TopFilteredClusters, PresentTime);

  Log.OutputClusterInformation(&DB, &QB, TopFilteredClusters, PresentTime);
  Log.WriteClusteringOutputToFile();

  // TODO: consider if quote is dead?

  // plot output
  //ClusterPlot Plotter(TStr("/lfs/1/tmp/curis/"));
  //Plotter.PlotClusterSizeUnique(Clusters);
  //Plotter.PlotClusterSize(ClusterSummaries);
  //Plotter.PlotQuoteFrequencies(QB);

  printf("Done!\n");
  return 0;
}
