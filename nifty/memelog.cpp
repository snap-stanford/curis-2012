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
  if (!Arguments.IsKeyGetDat("directory", OutputDirectory)) {
    Log.SetupNewOutputDirectory("");
  } else {
    if (ShouldSetupDirectory == "true") {
      Log.SetupNewOutputDirectory(OutputDirectory);
    } else {
      Log.SetDirectory(OutputDirectory);
    }
  }

  TSecTm CurrentDate = StartDate;
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

    // TODO: consider if quote is dead?
    TIntV Temp;
    Log.LogAllInformation(&DB, &QB, &CB, TopFilteredClusters, CurrentDate, Temp, QBDBCDirectory);

    CurrentDate.AddDays(1);
  }

  TStr Directory;
  Log.GetDirectory(Directory);
  Err("Done with memelog! Written to: %s\n", Directory.CStr());
  return 0;
}
