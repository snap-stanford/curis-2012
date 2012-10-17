#include "stdafx.h"
#include <stdio.h>

int main(int argc, char *argv[]) {
  // #### SETUP: Parse Arguments
  LogOutput Log;
  THash<TStr, TStr> Arguments;
  ArgumentParser::ParseArguments(argc, argv, Arguments, Log);

  TStr StartString = ArgumentParser::GetArgument(Arguments, "date", "2009-02-01");
  TStr QBDBCDirectory = ArgumentParser::GetArgument(Arguments, "qbdbc", "/lfs/1/tmp/curis/QBDBC/");
  TStr QBDBDirectory = ArgumentParser::GetArgument(Arguments, "qbdb", "/lfs/1/tmp/curis/QBDB/");

  TQuoteBase QB;
  TDocBase DB;
  TClusterBase CB;
  PNGraph QGraph;
  fprintf(stderr, "Loading cumulative QBDBCB from file...\n");
  TDataLoader::LoadCumulative(QBDBCDirectory, StartString, QB, DB, CB, QGraph);

  // #### POST CLUSTERING STEP YO
  TStr OutputDirectory;
  if (!Arguments.IsKeyGetDat("directory", OutputDirectory)) {
    Log.SetupNewOutputDirectory("");
  } else {
    Log.SetDirectory(OutputDirectory);
  }

  Log.DisableLogging();
  TSecTm PresentTime = TSecTm::GetDtTmFromYmdHmsStr(StartString);
  TIntV TopFilteredClusters;
  PostCluster::GetTopFilteredClusters(&CB, &DB, &QB, Log, TopFilteredClusters, PresentTime, QGraph, false);
  Log.EnableLogging();

  // TODO: consider if quote is dead?
  TIntV Temp;
  Log.LogAllInformation(&DB, &QB, &CB, TopFilteredClusters, PresentTime, Temp, QBDBCDirectory);

  TStr Directory;
  Log.GetDirectory(Directory);
  Err("Done with memelog!");
  return 0;
}
