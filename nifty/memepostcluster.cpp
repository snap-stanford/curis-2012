#include "stdafx.h"
#include <stdio.h>

int main(int argc, char *argv[]) {
  LogOutput Log;
  THash<TStr, TStr> Arguments;
  for (int i = 1; i < argc; i++) {
    if (strlen(argv[i]) >= 2 && argv[i][0] == '-' && i + 1 < argc) {
      Arguments.AddDat(TStr(argv[i] + 1), TStr(argv[i + 1]));
      Log.LogValue(TStr(argv[i] + 1), TStr(argv[i + 1]));
      i++;
    } else {
      printf("Error: incorrect format. Usage: ./memepostfilter [-paramName parameter]");
      exit(1);
    }
  }
  // load QB and DB. Custom variables can be added later.
  TStr BaseString = TWOWEEK_DIRECTORY;
  if (Arguments.IsKey("qbdb")) {
    TStr BaseArg = Arguments.GetDat("qbdb");
    if (BaseArg == "week") {
      BaseString = WEEK_DIRECTORY;
    } else if (BaseArg == "day"){
      BaseString = DAY_DIRECTORY;
    }
  }
  if (Arguments.IsKey("nolog")) {
    Log.DisableLogging();
  }

  TFIn BaseFile(BaseString + "QBDB.bin");
  TQuoteBase *QB = new TQuoteBase;
  TDocBase *DB = new TDocBase;
  QB->Load(BaseFile);
  DB->Load(BaseFile);

  fprintf(stderr, "loading clusters\n");

  TVec<TCluster> *ClusterSummaries = new TVec<TCluster>();
  TVec<TIntV> *Clusters = new TVec<TIntV>();
  TFIn ClusterFile(BaseString + "clusters.bin");
  fprintf(stderr, "%s%s\n", BaseString.CStr(), "clusters.bin");
  ClusterSummaries->Load(ClusterFile);
  fprintf(stderr, "did it get here?\n");
  TCluster tmp = (*ClusterSummaries)[0];
  fprintf(stderr, "ASDFSADFAF %d\n", tmp.GetRepresentativeQuoteId().Val);
  fprintf(stderr, "loading clusters\n");
  Log.Load(ClusterFile);
  fprintf(stderr, "loading clusters\n");
  Clusters->Load(ClusterFile);
  fprintf(stderr, "loading clusters\n");

  // OUTPUT
  /*Log.SetupFiles(); // safe to make files now.
  fprintf(stderr, "Writing cluster information to file\n");
  Log.OutputClusterInformation(DB, QB, *ClusterSummaries);
  fprintf(stderr, "Writing top clusters to file\n");
  Log.WriteClusteringOutputToFile();*/

  // plot output
  ClusterPlot Plotter(TStr("/lfs/1/tmp/curis/"));
  Plotter.PlotClusterSizeUnique(*Clusters);
  Plotter.PlotClusterSize(*ClusterSummaries);
  Plotter.PlotQuoteFrequencies(QB);

  delete QB;
  delete DB;
  printf("Done!\n");
  return 0;

  return 0;
}
