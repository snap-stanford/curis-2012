#include "stdafx.h"
#include <stdio.h>

int main(int argc, char *argv[]) {
  // #### SETUP: Parse Arguments
  LogOutput Log;
  THash<TStr, TStr> Arguments;
  ArgumentParser::ParseArguments(argc, argv, Arguments, Log);

  TStr StartString = ArgumentParser::GetArgument(Arguments, "start", "2009-02-01");
  TStr EndString = ArgumentParser::GetArgument(Arguments, "end", "2009-02-02");
  TStr QBDBCDirectory = ArgumentParser::GetArgument(Arguments, "qbdbc", QBDBC_DIR_DEFAULT);
  TStr QBDBDirectory = ArgumentParser::GetArgument(Arguments, "qbdb", QBDB_DIR_DEFAULT);
  TStr ShouldSetupDirectory = ArgumentParser::GetArgument(Arguments, "setup", "false");

  TSecTm StartDate = TSecTm::GetDtTmFromYmdHmsStr(StartString);
  TSecTm EndDate = TSecTm::GetDtTmFromYmdHmsStr(EndString);

  // #### POST CLUSTERING STEP YO
  TStr OutputDirectory;
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

  // #### GET OLD CLUSTERS
  TSecTm CurrentDate = StartDate;
  TSecTm OldDate = StartDate;
  OldDate.AddDays(-1);
  TQuoteBase oldQB;
  TDocBase oldDB;
  TClusterBase oldCB;
  PNGraph OldQGraph;
  fprintf(stderr, "Loading cumulative QBDBCB from file...\n");
  TDataLoader::LoadCumulative(QBDBCDirectory, OldDate.GetDtYmdStr(), oldQB, oldDB, oldCB, OldQGraph);
  fprintf(stderr, "\tDone loading cumulative QBDBCB!\n");

  // #### GET TOP FILTERED CLUSTERS FROM PREVIOUS DAY WHYY
  TIntV OldTopClusters;
  PostCluster::GetTopFilteredClusters(&oldCB, &oldDB, &oldQB, Log, OldTopClusters, OldDate, OldQGraph, false);


  while(CurrentDate < EndDate) {
    TStr CurDateString = CurrentDate.GetDtYmdStr();
    TQuoteBase QB;
    TDocBase DB;
    TClusterBase CB;
    PNGraph QGraph;
    fprintf(stderr, "Loading cumulative QBDBCB from file...\n");
    TDataLoader::LoadCumulative(QBDBCDirectory, CurrentDate.GetDtYmdStr(), QB, DB, CB, QGraph);

    Log.DisableLogging();
    TIntV TopFilteredClusters;
    PostCluster::GetTopFilteredClusters(&CB, &DB, &QB, Log, TopFilteredClusters, CurrentDate, QGraph, false);
    Log.EnableLogging();

    TStr FileName = QBDBCDirectory + "topQBDBC" + CurrentDate.GetDtYmdStr() + ".bin";
    PostCluster::SaveTopFilteredClusters(FileName, &QB, &DB, &CB, TopFilteredClusters, QGraph);

    // TODO: consider if quote is dead?
    TIntV Temp;
    Log.LogAllInformation(&DB, &QB, &CB, QGraph, TopFilteredClusters, CurrentDate, OldTopClusters, QBDBCDirectory);
    OldTopClusters = TopFilteredClusters;

    CurrentDate.AddDays(1);
  }

  TStr Directory;
  Log.GetDirectory(Directory);
  Err("Done with memelog! Written to: %s\n", Directory.CStr());
  return 0;
}
