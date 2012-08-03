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

  TStr StartString, EndString, OutputDirectory;
  if (!Arguments.IsKeyGetDat("start", StartString)) {
    StartString = "2012-07-08";
  }
  if (!Arguments.IsKeyGetDat("end", EndString)) {
    EndString = "2012-07-09";
  }
  if (!Arguments.IsKeyGetDat("directory", OutputDirectory)) {
    Log.SetupNewOutputDirectory();
  } else {
    Log.SetDirectory(OutputDirectory);
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
  TDataLoader::LoadCumulative(QBDBC_DIRECTORY, OldDate.GetDtYmdStr(), QB, DB, CB, OldQGraph);
  fprintf(stderr, "\tDone loading cumulative QBDBCB!\n");

  // #### GET TOP FILTERED CLUSTERS FROM PREVIOUS DAY WHYY
  TIntV OldTopClusters;
  PostCluster::GetTopFilteredClusters(&CB, &DB, &QB, Log, OldTopClusters, OldDate);

  // #### MAIN CLUSTERING STEP.
  TSecTm CurrentDate = StartDate;
  while(CurrentDate < EndDate) {
    // ## LOAD NEW QBDB
    TQuoteBase NewQB;
    TDocBase NewDB;
    fprintf(stderr, "Loading QBDB for %s...\n", CurrentDate.GetDtYmdStr().CStr());
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
    TIncrementalClustering ClusterJob(&QB, NewQuotes, QGraph, AffectedNodes);
    TClusterBase NewCB(CB.GetCounter());
    ClusterJob.BuildClusters(&NewCB, &QB, &DB, Log, &CB);
    TIntV SortedClusters;
    NewCB.GetAllClusterIdsSortByFreq(SortedClusters);

    // ## POSTCLUSTERING STEP AND OUTPUT?
    TIntV TopFilteredClusters;
    PostCluster::GetTopFilteredClusters(&NewCB, &DB, &QB, Log, TopFilteredClusters, CurrentDate);
    Log.OutputClusterInformation(&DB, &QB, &NewCB, TopFilteredClusters, CurrentDate, OldTopClusters);
    Log.WriteClusteringOutputToFile(CurrentDate);

    // ## SAVE CLUSTERS OR SAVE THEM TO VARIABLES.
    OldQGraph = QGraph;
    CB = NewCB;
    OldTopClusters = TopFilteredClusters;
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
