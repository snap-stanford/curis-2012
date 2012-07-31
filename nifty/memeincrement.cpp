#include "stdafx.h"
#include <stdio.h>

/// Must pass in argument for the new day to be added, in format YYYY-MM-DD (-newday)
//  TODO: Add framework for Log file (as in memecluster.cpp)
int main(int argc, char *argv[]) {
  // #### SETUP: Parse Arguments
  LogOutput Log;
  THash<TStr, TStr> Arguments;
  TStr BaseString;
  ArgumentParser::ParseArguments(argc, argv, Arguments, Log, BaseString);

  TStr StartString, EndString;
  if (!Arguments.IsKeyGetDat("start", StartString)) {
    StartString = "2012-07-08";
  }
  if (!Arguments.IsKeyGetDat("end", EndString)) {
    EndString = "2012-07-09";
  }

  TSecTm StartDate = TSecTm::GetDtTmFromYmdHmsStr(StartString);
  TSecTm EndDate = TSecTm::GetDtTmFromYmdHmsStr(EndString);

  // #### LOAD OLD QBDB
  TSecTm OldDate = StartDate;
  OldDate.AddDays(-1);
  TQuoteBase QB;
  TDocBase DB;
  TClusterBase CB;
  PNGraph OldQGraph;
  fprintf(stderr, "Loading cumulative QBDBCB from file...\n");
  TDataLoader::LoadCumulative(QBDBC_DIRECTORY, OldDate, QB, DB, CB, OldQGraph);
  fprintf(stderr, "\tDone loading cumulative QBDBCB!\n");

  // #### MAIN CLUSTERING STEP.
  TSecTm CurrentDate = StartDate;
  while(CurrentDate < EndDate) {
    // ## LOAD NEW QBDB
    TQuoteBase NewQB;
    TDocBase NewDB;
    fprintf(stderr, "Loading QBDB for %s...\n", CurrentDate.GetDtYmdStr());
    TDataLoader::LoadQBDB(QBDB_DIRECTORY, CurrentDate.GetDtYmdStr(), NewQB, NewDB);
    fprintf(stderr, "Done loading new QBDB!\n");

    // ## MERGE AND GET NEW QUOTES
    TIntSet NewQuotes(TDataLoader::MergeQBDB(QB, DB, NewQB, NewDB));

    // ## CLUSTERING STEP
    IncrementalQuoteGraph GraphCreator(&QB, NewQuotes, OldQGraph);
    PNGraph QGraph;
    GraphCreator.UpdateGraph(QGraph);
    TIntSet AffectedNodes;
    GraphCreator.GetAffectedNodes(AffectedNodes);
    IncrementalClustering2 ClusterJob(&QB, NewQuotes, QGraph, AffectedNodes);
    TIntSet RootNodes;
    TClusterBase NewCB;
    ClusterJob.BuildClusters(&NewCB, &QB, &DB, Log);
    TIntV SortedClusters;
    NewCB.GetAllClusterIdsSortByFreq(SortedClusters);

    // ## POSTCLUSTERING STEP AND OUTPUT?
    Log.SetupFiles();
    TIntV TopFilteredClusters;
    PostCluster::GetTopFilteredClusters(&CB, &DB, &QB, Log, TopFilteredClusters, CurrentDate);
    Log.OutputClusterInformation(&DB, &QB, &CB, TopFilteredClusters, CurrentDate);
    Log.WriteClusteringOutputToFile();

    // ## SAVE CLUSTERS OR SAVE THEM TO VARIABLES.
    OldQGraph = QGraph;
    CB = NewCB;
    TStr FileName = TStr(QBDBC_DIRECTORY) + "QBDBC" + CurrentDate.GetDtYmdStr() + ".bin";
    fprintf(stderr, "Saving Cluster information to file: %s", FileName.CStr());
    TFOut FOut(FileName);
    QB.Save(FOut);
    DB.Save(FOut);
    CB.Save(FOut);
    QGraph->Save(FOut);

    CurrentDate.AddDays(1);
  }
  // plot output
  //ClusterPlot Plotter(TStr("/lfs/1/tmp/curis/"));
  //Plotter.PlotClusterSizeUnique(Clusters);
  //Plotter.PlotClusterSize(ClusterSummaries);
  //Plotter.PlotQuoteFrequencies(&QB);

  // Save to file
  /*Command = "mkdir -p output";
  system(Command.CStr());
  TFOut FOut2("output/cumulativeclusters" + NewDayDate.CStr() + ".bin");
  MergedClusterSummaries.Save(FOut2);*/
  printf("Done with EVERYTHING!\n");
  return 0;
}
