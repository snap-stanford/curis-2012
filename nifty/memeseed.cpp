#include "stdafx.h"
#include <stdio.h>

int main(int argc, char *argv[]) {
  // #### SETUP: Parse Arguments
  LogOutput Log;
  THash<TStr, TStr> Arguments;
  ArgumentParser::ParseArguments(argc, argv, Arguments, Log);

  TStr OutputDirectory = ArgumentParser::GetArgument(Arguments, "directory", "");
  TStr StartString = ArgumentParser::GetArgument(Arguments, "start", "2009-02-01");
  TStr QBDBCDirectory = ArgumentParser::GetArgument(Arguments, "qbdbc", QBDBC_DIR_DEFAULT);
  TStr QBDBDirectory = ArgumentParser::GetArgument(Arguments, "qbdb", QBDB_DIR_DEFAULT);
  TInt WindowSize = ArgumentParser::GetArgument(Arguments, "window", "14").GetInt();
  TStr EdgeString = ArgumentParser::GetArgument(Arguments, "edge", EDGE_CREATION_STYLE);
  QuoteGraph::SetEdgeCreation(EdgeString);


  if (ArgumentParser::GetArgument(Arguments, "nolog", "") != "") {
    Log.DisableLogging();
  } else if (OutputDirectory == "") {
    Log.SetupNewOutputDirectory("");
  } else if (ArgumentParser::GetArgument(Arguments, "nosetup", "") == ""){
    Log.SetupNewOutputDirectory(OutputDirectory);
  } else {
    Log.SetDirectory(OutputDirectory);
  }

  TQuoteBase QB;
  TDocBase DB;
  TClusterBase CB;

  // Start memeseed after a break in the middle
  TStr LastDate = ArgumentParser::GetArgument(Arguments, "last", "");
  if (LastDate != "") {
    {
    TQuoteBase OldQB;
    TDocBase OldDB;
    TClusterBase OldCB;
    PNGraph OldQGraph;
    TDataLoader::LoadCumulative(QBDBCDirectory, LastDate, OldQB, OldDB, OldCB, OldQGraph);
    QB = TQuoteBase(OldQB.GetCounter());
    DB = TDocBase(OldDB.GetCounter());
    CB = TClusterBase(OldCB.GetCounter());
    }
    Err("Counters updated!\n");
  }

  TStr LogDirectory;
  Log.GetDirectory(LogDirectory);
  Err("Output directory: %s\n", LogDirectory.CStr());

  // #### DATA LOADING: Load ALL the things!
  fprintf(stderr, "Loading QB and DB from file for %d days, starting from %s...\n", WindowSize.Val, StartString.CStr());
  Err("%s\n", QBDBDirectory.CStr());
  TSecTm PresentTime = TDataLoader::LoadQBDBByWindow(QBDBDirectory, StartString, WindowSize, QB, DB);
  fprintf(stderr, "\tQBDB successfully loaded!\n");

  // #### CLUSTERING STEP
  fprintf(stderr, "Creating clusters\n");
  QuoteGraph GraphCreator(&QB, &CB);
  PNGraph QGraph;
  GraphCreator.CreateGraph(QGraph);
  Clustering ClusterJob(QGraph);
  ClusterJob.BuildClusters(&CB, &QB, &DB, Log, PresentTime);
  GraphCreator.LogEdges("WordsCheapAfter.txt");

  // #### POST CLUSTERING STEP YO
  TIntV TopFilteredClusters;
  //CB.GetAllClusterIdsSortByFreq(TopFilteredClusters);
  PostCluster::GetTopFilteredClusters(&CB, &DB, &QB, Log, TopFilteredClusters, PresentTime, QGraph);

  // #### SAVE THE DOLPHINS! I MEAN CLUSTERS
  TStr FileName = QBDBCDirectory + "QBDBC" + PresentTime.GetDtYmdStr() + ".bin";
  TDataLoader::SaveQBDBCQ(FileName, &QB, &DB, &CB, QGraph);

  TIntV Temp;
  //Log.LogAllInformation(&DB, &QB, &CB, TopFilteredClusters, PresentTime, Temp, QBDBCDirectory);

  TStr Directory;
  Log.GetDirectory(Directory);
  Err("Done with memeseed! Directory created at: %s\n", Directory.CStr());
  //printf("%d\n", TStringUtil::f_counter);
  return 0;
}
