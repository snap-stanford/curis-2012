#include "stdafx.h"
#include <stdio.h>

int main(int argc, char *argv[]) {
  // #### SETUP: Parse Arguments
  LogOutput Log;
  THash<TStr, TStr> Arguments;
  TStr BaseString;
  ArgumentParser::ParseArguments(argc, argv, Arguments, Log, BaseString);

  TStr StartString, WindowString;
  if (!Arguments.IsKeyGetDat("start", StartString)) {
    StartString = "2012-06-24";
  }

  TInt WindowSize = 14;
  if (Arguments.IsKeyGetDat("window", WindowString)) {
    WindowSize = WindowString.GetInt();
  }

  // #### DATA LOADING: Load everything!
  TQuoteBase QB;
  TDocBase DB;
  fprintf(stderr, "Loading QB and DB from file for %d days, starting from %s...\n", WindowSize.Val, StartString.CStr());
  TSecTm PresentTime = TDataLoader::LoadQBDBByWindow(QBDB_DIRECTORY, StartString, WindowSize, QB, DB);
  fprintf(stderr, "\tQBDB successfully loaded!\n");

  // #### CLUSTERING STEP
  fprintf(stderr, "Creating clusters\n");
  TClusterBase CB;
  QuoteGraph GraphCreator(&QB);
  PNGraph QGraph;
  GraphCreator.CreateGraph(QGraph);
  Clustering ClusterJob(QGraph);
  ClusterJob.BuildClusters(&CB, &QB, &DB, Log, PresentTime);

  // #### SAVE THE DOLPHINS! I MEAN CLUSTERS
  TStr FileName = TStr(QBDBC_DIRECTORY) + "QBDBC" + PresentTime.GetDtYmdStr() + ".bin";
  fprintf(stderr, "Saving Cluster information to file: %s", FileName.CStr());
  TFOut FOut(FileName);
  QB.Save(FOut);
  DB.Save(FOut);
  CB.Save(FOut);
  QGraph->Save(FOut);

  // #### POST CLUSTERING STEP YO
  Log.SetupNewOutputDirectory(); // safe to make files now.
  TIntV TopFilteredClusters;
  //CB.GetAllClusterIdsSortByFreq(TopFilteredClusters);
  PostCluster::GetTopFilteredClusters(&CB, &DB, &QB, Log, TopFilteredClusters, PresentTime);

  // TODO: consider if quote is dead?
  Log.OutputClusterInformation(&DB, &QB, &CB, TopFilteredClusters, PresentTime);
  Log.WriteClusteringOutputToFile(PresentTime);

  return 0;
}
