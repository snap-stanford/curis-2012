#include "stdafx.h"
#include <stdio.h>

/// Must pass in argument for the new day to be added, in format YYYY-MM-DD (-newday)
//  TODO: Add framework for Log file (as in memecluster.cpp)
int main(int argc, char *argv[]) {
  LogOutput Log;
  THash<TStr, TStr> Arguments;
  TStr BaseString;
  bool DoIncrementalClustering = false;
  ArgumentParser::ParseArguments(argc, argv, Arguments, Log, BaseString, DoIncrementalClustering);

  if (!Arguments.IsKey("newday")) {
    fprintf(stderr, "Must input date of the new day to be added, in the format YYYY-MM-DD (-newday)");
    return 1;
  }
  //TStr NewDayDate = Arguments.GetDat("newday");
  TStr OldDayDate = "2012-07-01";
  TStr NewDayDate = "2012-07-02";

  TSecTm PresentTime = TSecTm::GetDtTmFromYmdHmsStr(NewDayDate + " 23:59:59");

  TQuoteBase QB;
  TDocBase DB;
  TVec<TCluster> ClusterSummaries;
  PNGraph OldQGraph;
  fprintf(stderr, "Loading cumulative QB, DB, and clusters from file...\n");
  TDataLoader::LoadCumulative("/lfs/1/tmp/curis/QBDBC/", OldDayDate, QB, DB, ClusterSummaries, OldQGraph);
  fprintf(stderr, "Done!\n");

  TQuoteBase NewDayQB;
  TDocBase NewDayDB;
  fprintf(stderr, "Loading New Day QB and DB from file...\n");
  TDataLoader::LoadQBDB("/lfs/1/tmp/curis/QBDB/", NewDayDate, NewDayQB, NewDayDB);
  fprintf(stderr, "Done!\n");

  fprintf(stderr, "Adding new quotes to clusters and creating new ones\n");
  // NewQuotes stores the indices (in the new QB) of the quotes that are in NewDayQB but not in QB
  TIntV NewQuotes = TDataLoader::MergeQBDB(QB, DB, NewDayQB, NewDayDB);

  TVec<TIntV> TempMergedClusters;
  TIncrementalClustering::BuildClusters(TempMergedClusters, ClusterSummaries, QB, DB, NewQuotes);
  TVec<TIntV> MergedClusters;
  TIncrementalClustering::RemoveOldClusters(MergedClusters, TempMergedClusters, QB, DB, PresentTime);
  TVec<TCluster> MergedClusterSummaries;
  Clustering::SortClustersByFreq(MergedClusterSummaries, MergedClusters, &QB);

  // Save to file
  //TStr Command = "mkdir -p output";
  //system(Command.CStr());
  //TFOut FOut("output/cumulativeclusters" + NewDayDate + ".bin");
  fprintf(stderr, "Saving new cumulative QB, DB, and clusters from file...\n");
  TFOut FOut("/lfs/1/tmp/curis/QBDBC/QBDBC" + NewDayDate + ".bin");
  QB.Save(FOut);
  DB.Save(FOut);
  MergedClusterSummaries.Save(FOut);
  printf("Done!\n");
  return 0;
}
