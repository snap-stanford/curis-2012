#include "stdafx.h"
#include <stdio.h>

/// Must pass in argument for the new day to be added, in format YYYY-MM-DD (-newday)
//  TODO: Add framework for Log file (as in memecluster.cpp)
int main(int argc, char *argv[]) {
  LogOutput Log;
  THash<TStr, TStr> Arguments;
  TStr BaseString;
  ArgumentParser::ParseArguments(argc, argv, Arguments, Log, BaseString);

  if (!Arguments.IsKey("newday")) {
    fprintf(stderr, "Must input date of the new day to be added, in the format YYYY-MM-DD (-newday)");
  }
  TStr NewDayDate = Arguments.GetDat("newday");

  TQuoteBase QB;
  TDocBase DB;
  TVec<TCluster> ClusterSummaries;
  fprintf(stderr, "Loading cumulative QB, DB, and clusters from file...\n");
  TSecTm PresentTime = TDataLoader::LoadQBDBAndClusters("/lfs/1/tmp/curis/cumulativeresults/",
                                                        NewDayDate, QB, DB, ClusterSummaries);
  fprintf(stderr, "Done!\n");

  TQuoteBase NewDayQB;
  TDocBase NewDayDB;
  fprintf(stderr, "Loading New Day QB and DB from file...\n");
  PresentTime = TDataLoader::LoadQBDB("/lfs/1/tmp/curis/QBDB/", NewDayDate, NewDayQB, NewDayDB);
  fprintf(stderr, "Done!\n");

  fprintf(stderr, "Adding new quotes to clusters and creating new ones\n");
  IncrementalClustering ClusterJob;
  TVec<TIntV> MergedClusters;
  ClusterJob.BuildClusters(MergedClusters, ClusterSummaries, QB, DB, NewDayQB, NewDayDB);
  TDataLoader::MergeQBDB(QB, DB, NewDayQB, NewDayDB);
  TVec<TCluster> MergedClusterSummaries;
  Clustering::SortClustersByFreq(MergedClusterSummaries, MergedClusters, QB);

  // Save to file
  TStr Command = "mkdir -p output";
  system(Command.CStr());
  TFOut FOut("output/cumulativeclusters" + NewDayDate.CStr() + ".bin");
  MergedClusterSummaries.Save(FOut);

  printf("Done!\n");
  return 0;
}
