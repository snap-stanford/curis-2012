#include "stdafx.h"
#include <stdio.h>

/// Must pass in argument for the new day to be added, in format YYYY-MM-DD (-newday)
int main(int argc, char *argv[]) {
  // #### SETUP: Parse Arguments
  LogOutput Log;
  THash<TStr, TStr> Arguments;
  ArgumentParser::ParseArguments(argc, argv, Arguments, Log);

  TStr OutputDirectory;
  TStr StartString = ArgumentParser::GetArgument(Arguments, "start", "2009-02-01");
  TStr EndString = ArgumentParser::GetArgument(Arguments, "end", "2009-02-06");
  TStr QBDBCDirectory = ArgumentParser::GetArgument(Arguments, "qbdbc", "/lfs/1/tmp/curis/QBDBC/");
  TStr QBDBDirectory = ArgumentParser::GetArgument(Arguments, "qbdb", "/lfs/1/tmp/curis/QBDB/");

  if (!Arguments.IsKeyGetDat("directory", OutputDirectory)) {
    Log.SetupNewOutputDirectory("");
  } else {
    Log.SetDirectory(OutputDirectory);
  }
  Log.SetupQBDBCBSizeFile();

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
  TDataLoader::LoadCumulative(QBDBCDirectory, OldDate.GetDtYmdStr(), QB, DB, CB, OldQGraph);
  fprintf(stderr, "\tDone loading cumulative QBDBCB!\n");

  // #### GET TOP FILTERED CLUSTERS FROM PREVIOUS DAY WHYY
  TIntV OldTopClusters;
  PostCluster::GetTopFilteredClusters(&CB, &DB, &QB, Log, OldTopClusters, OldDate, OldQGraph, false);

  // #### MAIN CLUSTERING STEP.
  TSecTm CurrentDate = StartDate;
  while(CurrentDate < EndDate) {
    // ## LOAD NEW QBDB
    TQuoteBase NewQB;
    TDocBase NewDB;
    fprintf(stderr, "Loading QBDB for %s...\n", CurrentDate.GetDtYmdStr().CStr());
    TDataLoader::LoadQBDB(QBDBDirectory, CurrentDate.GetDtYmdStr(), NewQB, NewDB);
    fprintf(stderr, "Done loading new QBDB!\n");

    int OldGraphSize = OldQGraph->GetNodes();

    // ## MERGE AND GET NEW QUOTES
    TIntSet NewQuotes(TDataLoader::MergeQBDB(QB, DB, NewQB, NewDB));

    // ## CLUSTERING STEP
    IncrementalQuoteGraph GraphCreator(&QB, &CB, NewQuotes, OldQGraph);
    PNGraph QGraph;
    GraphCreator.UpdateGraph(QGraph);
    TIntSet AffectedNodes;
    GraphCreator.GetAffectedNodes(AffectedNodes);

    TIncrementalClustering ClusterJob(&QB, NewQuotes, QGraph, AffectedNodes);
    TClusterBase NewCB(CB.GetCounter());
    ClusterJob.BuildClusters(&NewCB, &QB, &DB, Log, CurrentDate, &CB);
    fprintf(stderr, "Done building clusters!\n");
    TIntV SortedClusters;
    NewCB.GetAllClusterIdsSortByFreq(SortedClusters);

    // ## POSTCLUSTERING STEP AND OUTPUT?
    TIntV TopFilteredClusters;
    PostCluster::GetTopFilteredClusters(&NewCB, &DB, &QB, Log, TopFilteredClusters, CurrentDate, QGraph);
    
    //Log.OutputClusterInformation(&DB, &QB, &NewCB, TopFilteredClusters, CurrentDate, OldTopClusters);
    Log.WriteClusteringOutputToFile(CurrentDate);
    Log.LogQBDBCBSize(&DB, &QB, &CB);

    // ## SAVE CLUSTERS OR SAVE THEM TO VARIABLES.
    OldQGraph = QGraph;
    CB = NewCB;
    OldTopClusters = TopFilteredClusters;
    TStr FileName = QBDBCDirectory + "QBDBC" + CurrentDate.GetDtYmdStr() + ".bin";
    fprintf(stderr, "Saving Cluster information to file: %s\n", FileName.CStr());
    TFOut FOut(FileName);
    IAssert(!QB.IsContainNullQuote());
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
  Log.ShutDown();
  printf("Done with POST CLUSTERING!\n");
  return 0;
}
