#include "stdafx.h"
#include <stdio.h>

/// Must pass in argument for the new day to be added, in format YYYY-MM-DD (-newday)
//  TODO: Add framework for Log file (as in memecluster.cpp)
int main(int argc, char *argv[]) {
  // PARSE ARGUMENTS AND SET UP EVERYTHING
  LogOutput Log;
  THash<TStr, TStr> Arguments;
  TStr BaseString;
  bool Increment;
  ArgumentParser::ParseArguments(argc, argv, Arguments, Log, BaseString, Increment);

  if (!Arguments.IsKey("newday")) {
    fprintf(stderr, "Must input date of the new day to be added, in the format YYYY-MM-DD (-newday)");
    exit(1);
  }
  TStr OldDayDate = "2012-07-01";
  TStr NewDayDate = "2012-07-02";

  // LOAD OLD QBDB
  TQuoteBase QB;
  TDocBase DB;
  TVec<TCluster> ClusterSummaries;
  PNGraph OldQGraph;
  fprintf(stderr, "Loading cumulative QB, DB, and clusters from file...\n");
  TDataLoader::LoadCumulative("/lfs/1/tmp/curis/QBDBC/",
                                                        OldDayDate, QB, DB, ClusterSummaries, OldQGraph);
  fprintf(stderr, "Done!\n");
  fprintf(stderr, "Old Quote Counter: %d\n", QB.GetCurCounterValue().Val);
  // LOAD NEW QBDB
  TQuoteBase NewDayQB;
  TDocBase NewDayDB;
  fprintf(stderr, "Loading New Day QB and DB from file...\n");
  TDataLoader::LoadQBDB("/lfs/1/tmp/curis/QBDB/", NewDayDate, NewDayQB, NewDayDB);
  fprintf(stderr, "Done!\n");
  fprintf(stderr, "New Quote Counter: %d\n", NewDayQB.GetCurCounterValue().Val);

  // MERGE OLD AND NEW QBDB
  fprintf(stderr, "Adding new quotes to clusters and creating new ones\n");
  //IncrementalClustering ClusterJob;
  TVec<TIntV> MergedClusters;
  //ClusterJob.BuildClusters(MergedClusters, ClusterSummaries, QB, DB, NewDayQB, NewDayDB);
  TIntSet NewQuotes(TDataLoader::MergeQBDB(QB, DB, NewDayQB, NewDayDB));
  fprintf(stderr, "Merged Quote Counter: %d\n", QB.GetCurCounterValue().Val);
  TIntV TempV;
  QB.GetAllQuoteIds(TempV);
  fprintf(stderr, "Number of quotes: %d\n", TempV.Len());

  TSecTm PresentTime = TSecTm::GetDtTmFromYmdHmsStr(NewDayDate + " 23:59:59");

  // create clusters and save!
  /*QuoteGraph GraphCreator(&QB);
  PNGraph QGraph;
  GraphCreator.CreateGraph(QGraph);
  Clustering ClusterJob;
  ClusterJob.SetGraph(QGraph);
  TIntSet RootNodes;
  TVec<TIntV> Clusters;
  ClusterJob.BuildClusters(RootNodes, Clusters, &QB, &DB, Log);
  ClusterSummaries.Clr();
  ClusterJob.SortClustersByFreq(ClusterSummaries, Clusters, &QB);*/

  IncrementalQuoteGraph GraphCreator(&QB, NewQuotes, OldQGraph);
  PNGraph QGraph;
  GraphCreator.UpdateGraph(QGraph);
  TIntSet AffectedNodes;
  GraphCreator.GetAffectedNodes(AffectedNodes);
  IncrementalClustering2 ClusterJob(&QB, NewQuotes, QGraph, AffectedNodes);
  TIntSet RootNodes;
  TVec<TIntV> Clusters;
  ClusterJob.BuildClusters(RootNodes, Clusters, &QB, &DB, Log);
  ClusterSummaries.Clr();
  Clustering::SortClustersByFreq(ClusterSummaries, Clusters, &QB);

  // save clusters
  TStr Command = "mkdir -p output";
  system(Command.CStr());
  TFOut FOut("output/clusters.bin");
  Clusters.Save(FOut); //TODO: rewrite the method that needs this?
  ClusterSummaries.Save(FOut);
  Log.Save(FOut);

  // post clustering
  Log.SetupFiles(); // safe to make files now.
  TVec<TCluster> TopFilteredClusters;
  PostCluster::GetTopFilteredClusters(&DB, &QB, Log, ClusterSummaries, TopFilteredClusters, PresentTime);

  // OUTPUT
  //Log.SetupFiles(); // safe to make files now.
  Log.OutputClusterInformation(&DB, &QB, TopFilteredClusters, PresentTime);
  Log.WriteClusteringOutputToFile();

  // plot output
  ClusterPlot Plotter(TStr("/lfs/1/tmp/curis/"));
  Plotter.PlotClusterSizeUnique(Clusters);
  Plotter.PlotClusterSize(ClusterSummaries);
  Plotter.PlotQuoteFrequencies(&QB);

  // Save to file
  /*Command = "mkdir -p output";
  system(Command.CStr());
  TFOut FOut2("output/cumulativeclusters" + NewDayDate.CStr() + ".bin");
  MergedClusterSummaries.Save(FOut2);*/

  //delete QB;
  //delete DB;
  printf("Done!\n");
  return 0;
}
