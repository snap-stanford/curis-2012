#include "stdafx.h"
#include <stdio.h>

int main(int argc, char *argv[]) {
  // #### SETUP: Parse Arguments
  LogOutput Log;
  THash<TStr, TStr> Arguments;
  ArgumentParser::ParseArguments(argc, argv, Arguments, Log);

  TStr StartString = ArgumentParser::GetArgument(Arguments, "start", "2009-02-01");
  TStr QBDBCDirectory = ArgumentParser::GetArgument(Arguments, "qbdbc", "/lfs/1/tmp/curis/QBDBC/");
  TStr QBDBDirectory = ArgumentParser::GetArgument(Arguments, "qbdb", "/lfs/1/tmp/curis/QBDB/");
  TInt WindowSize = ArgumentParser::GetArgument(Arguments, "window", "14").GetInt();

  // #### DATA LOADING: Load everything!
  TQuoteBase QB;
  TDocBase DB;
  fprintf(stderr, "Loading QB and DB from file for %d days, starting from %s...\n", WindowSize.Val, StartString.CStr());
  TSecTm PresentTime = TDataLoader::LoadQBDBByWindow(QBDBDirectory, StartString, WindowSize, QB, DB);
  fprintf(stderr, "\tQBDB successfully loaded!\n");

  // #### CLUSTERING STEP
  fprintf(stderr, "Creating clusters\n");
  TClusterBase CB;
  QuoteGraph GraphCreator(&QB, &CB);
  PNGraph QGraph;
  GraphCreator.CreateGraph(QGraph);
  Clustering ClusterJob(QGraph);
  ClusterJob.BuildClusters(&CB, &QB, &DB, Log, PresentTime);

  // #### POST CLUSTERING STEP YO
  Log.SetupNewOutputDirectory(""); // safe to make files now.
  TIntV TopFilteredClusters;
  //CB.GetAllClusterIdsSortByFreq(TopFilteredClusters);
  PostCluster::GetTopFilteredClusters(&CB, &DB, &QB, Log, TopFilteredClusters, PresentTime, QGraph);

  // #### SAVE THE DOLPHINS! I MEAN CLUSTERS
  TStr FileName = QBDBCDirectory + "QBDBC" + PresentTime.GetDtYmdStr() + ".bin";
  fprintf(stderr, "Saving Cluster information to file: %s", FileName.CStr());
  TFOut FOut(FileName);
  QB.Save(FOut);
  DB.Save(FOut);
  CB.Save(FOut);
  QGraph->Save(FOut);

  // TODO: consider if quote is dead?
  TIntV Temp;
  Log.LogAllInformation(&DB, &QB, &CB, TopFilteredClusters, PresentTime, Temp, QBDBCDirectory);

  TStr Directory;
  Log.GetDirectory(Directory);
  Err("Done with memeseed! Directory created at: %s\n", Directory.CStr());
  return 0;
}
