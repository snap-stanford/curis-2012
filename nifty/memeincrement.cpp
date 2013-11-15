#include "stdafx.h"
#include <stdio.h>

/// Must pass in argument for the new day to be added, in format YYYY-MM-DD (-newday)
int main(int argc, char *argv[]) {
  // #### SETUP: Parse Arguments
  LogOutput Log;
  THash<TStr, TStr> Arguments;
  ArgumentParser::ParseArguments(argc, argv, Arguments, Log);
  if(ArgumentParser::Contains("fastjson")) {
    Err("FAST JSON BOOYEAH\n");
  }

  TStr OutputDirectory;
  TStr StartString = ArgumentParser::GetArgument(Arguments, "start", "2009-02-01");
  TStr EndString = ArgumentParser::GetArgument(Arguments, "end", "2009-02-06");
  TStr QBDBCDirectory = ArgumentParser::GetArgument(Arguments, "qbdbc", QBDBC_DIR_DEFAULT);
  TStr QBDBDirectory = ArgumentParser::GetArgument(Arguments, "qbdb", QBDB_DIR_DEFAULT);
  TStr ClustMethod = ArgumentParser::GetArgument(Arguments, "method", "incremental");
  TStr NewDir = ArgumentParser::GetArgument(Arguments, "newdir", "");

  if (ArgumentParser::GetArgument(Arguments, "nolog", "") != "") {
    Log.DisableLogging();
  } else if (!Arguments.IsKeyGetDat("directory", OutputDirectory)) {
    Log.SetupNewOutputDirectory("");
  } else if (NewDir != "") {
	Log.SetupNewOutputDirectory(OutputDirectory);
  } else {
    Log.SetDirectory(OutputDirectory);
  }

  bool CheckEdgesDel = Arguments.IsKey("edgesdel");

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
    Err("INCREMENTAL COMPARISONS: %d\n", GraphCreator.IncrementCount);
    TIntSet AffectedNodes;
    GraphCreator.GetAffectedNodes(AffectedNodes);

    TIncrementalClustering ClusterJob(&QB, NewQuotes, QGraph, AffectedNodes);
    TClusterBase NewCB(CB.GetCounter());
    ClusterJob.BuildClusters(&NewCB, &QB, &DB, Log, CurrentDate, &CB, ClustMethod, CheckEdgesDel);
    fprintf(stderr, "Done building clusters!\n");
    TIntV SortedClusters;
    NewCB.GetAllClusterIdsSortByFreq(SortedClusters);

    // ## POSTCLUSTERING STEP AND OUTPUT?
    TIntV TopFilteredClusters;
    PostCluster::GetTopFilteredClusters(&NewCB, &DB, &QB, Log, TopFilteredClusters, CurrentDate, QGraph);
    
    // ## SAVE CLUSTERS OR SAVE THEM TO VARIABLES.
    OldQGraph = QGraph;
    CB = NewCB;
    TStr FileName = QBDBCDirectory + "QBDBC" + CurrentDate.GetDtYmdStr() + ".bin";
    TDataLoader::SaveQBDBCQ(FileName, &QB, &DB, &CB, QGraph);
    FileName = QBDBCDirectory + "topQBDBC" + CurrentDate.GetDtYmdStr() + ".bin";
    PostCluster::SaveTopFilteredClusters(FileName, &QB, &DB, &CB, TopFilteredClusters, QGraph);

    // ## LOG.
    Log.LogAllInformation(&DB, &QB, &NewCB, QGraph, TopFilteredClusters, CurrentDate, OldTopClusters, QBDBCDirectory);

    OldTopClusters = TopFilteredClusters;
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
  printf("Done with POST CLUSTERING!\n");
  return 0;
}
