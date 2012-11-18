#include "stdafx.h"
#include <stdio.h>

int main(int argc, char *argv[]) {
  // #### SETUP: Parse Arguments
  LogOutput Log;
  THash<TStr, TStr> Arguments;
  ArgumentParser::ParseArguments(argc, argv, Arguments, Log);

  TStr OutputDirectory = ArgumentParser::GetArgument(Arguments, "directory", "");
  TStr StartString = ArgumentParser::GetArgument(Arguments, "start", "2009-02-01");
  TStr QBDBCDirectory = ArgumentParser::GetArgument(Arguments, "qbdbc", "/lfs/1/tmp/curis/QBDBC-C/");
  TStr QBDBDirectory = ArgumentParser::GetArgument(Arguments, "qbdb", "/lfs/1/tmp/curis/QBDB/");
  TInt WindowSize = ArgumentParser::GetArgument(Arguments, "window", "14").GetInt();
  TStr EdgeString = ArgumentParser::GetArgument(Arguments, "edge", "lsh");
  QuoteGraph::SetEdgeCreation(EdgeString);

  Log.DisableLogging();

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

  TIntV AllClusters;
  CB.GetAllClusterIdsSortByFreq(AllClusters);
  //PostCluster::FilterAndCacheClusterSize(&DB, &QB, &CB, Log, AllClusters, PresentTime);
  //PostCluster::FilterAndCacheClusterPeaks(&DB, &QB, &CB, Log, AllClusters, PresentTime);

  int NumClusters = AllClusters.Len();
  FILE* F = fopen("nifty-day-2012-01-01.txt", "w");
  for (int i = 0; i < NumClusters; i++) {
    TCluster C;
    CB.GetCluster(AllClusters[i], C);
    if (C.GetNumUniqueQuotes() > 1) {
      TIntV Quotes;
      C.GetQuoteIds(Quotes);
      TStr RepStr;
      C.GetRepresentativeQuoteString(RepStr, &QB);
      fprintf(F, "%d\t%d\t%s\t%d\n", Quotes.Len(), C.GetNumQuotes().Val, RepStr.CStr(), C.GetId().Val);
      for (int j = 0; j < Quotes.Len(); j++) {
        TQuote Q;
        QB.GetQuote(Quotes[j], Q);
        TStr Str;
        Q.GetContentString(Str);
        fprintf(F, "\t%d\t%d\t%s\t%d\n", Q.GetNumSources().Val, Q.GetNumSources().Val, Str.CStr(), Q.GetId().Val);
      }
      fprintf(F, "\n");
    }

  }
  fclose(F);

  TStr Directory;
  Log.GetDirectory(Directory);
  Err("Done with memeoutput! Directory created at: %s\n", Directory.CStr());
  //printf("%d\n", TStringUtil::f_counter);
  return 0;
}
