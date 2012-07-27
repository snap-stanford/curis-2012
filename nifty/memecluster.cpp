#include "stdafx.h"
#include <stdio.h>

void PlotQuoteFreq(TQuoteBase *QB, TDocBase *DB, TInt BucketSize, TInt SlidingWindowSize, TSecTm PresentTime) {
  fprintf(stderr, "Testing graph quote\n");
  TIntV AllQuotes;
  QB->GetAllQuoteIds(AllQuotes);
  // Sort by descending frequency of quote
  AllQuotes.SortCmp(TCmpQuoteByFreq(false, QB));

  fprintf(stderr, "Sorting complete.\n");

  for (int i = 0; i < 100; i++) {
    TQuote Q;
    QB->GetQuote(AllQuotes[i], Q);
    TStr Filename = TStr(Q.GetNumSources().GetStr() + "Quote" + Q.GetId().GetStr());
    Q.GraphFreqOverTime(DB, Filename, BucketSize, SlidingWindowSize, PresentTime); //TODO FIX
  }
}

/// Prints URLs of each quote to "QuoteURLs.output"
void PrintQuoteURLs(TQuoteBase *QB, TDocBase *DB) {
  printf("Printing Quote URLs to file\n");
  TIntV AllQuotes;
  QB->GetAllQuoteIds(AllQuotes);
  AllQuotes.SortCmp(TCmpQuoteById(true, QB));
  TStr QuoteURLFileName = "QuoteURLs.output";
  FILE *F = fopen(QuoteURLFileName.CStr(), "w");

  for (int i = 0; i < AllQuotes.Len(); i++) {
    TQuote Q;
    QB->GetQuote(AllQuotes[i], Q);
    TStr QContent;
    Q.GetContentString(QContent);
    fprintf(F, "%d\t%s\n", AllQuotes[i].Val, QContent.CStr());
    TIntV QSources;
    Q.GetSources(QSources);
    TStrV QSourcesUrl;
    for (int j = 0; j < QSources.Len(); j++) {
      TDoc D;
      DB->GetDoc(QSources[j], D);
      TStr DUrl;
      D.GetUrl(DUrl);
      QSourcesUrl.Add(DUrl);
    }

    QSourcesUrl.Sort();
    for (int j = 0; j < QSourcesUrl.Len(); j++) {
      fprintf(F, "%s\n", QSourcesUrl[j].CStr());
    }
  }

  fclose(F);
}

int main(int argc, char *argv[]) {
  bool DoIncrementalClustering;
  LogOutput Log;
  THash<TStr, TStr> Arguments;
  TStr BaseString;
  ArgumentParser::ParseArguments(argc, argv, Arguments, Log, BaseString, DoIncrementalClustering);

  TQuoteBase QB;
  TDocBase DB;
  fprintf(stderr, "Loading QB and DB from file...\n");
  TSecTm PresentTime = TDataLoader::LoadBulkQBDB("/lfs/1/tmp/curis/QBDB/", BaseString, QB, DB);
  fprintf(stderr, "Done!\n");

  //PlotQuoteFreq(QB, DB, 1, 6, PresentTime);
  //PrintQuoteURLs(QB, DB);

  fprintf(stderr, "Creating clusters\n");
 
  // create clusters and save!
  QuoteGraph GraphCreator(&QB);
  PNGraph QGraph;
  GraphCreator.CreateGraph(QGraph);
  Clustering ClusterJob;
  ClusterJob.SetGraph(QGraph);
  TIntSet RootNodes;
  TVec<TIntV> Clusters;
  ClusterJob.BuildClusters(RootNodes, Clusters, &QB, &DB, Log);
  TVec<TCluster> ClusterSummaries;
  ClusterJob.SortClustersByFreq(ClusterSummaries, Clusters, &QB);

  fprintf(stderr, "Saving files...\n");
  // Save to file
  if (!DoIncrementalClustering) {
    fprintf(stderr, "non incremental clusering, boo.\n");
    TStr Command = "mkdir -p output";
    system(Command.CStr());
    TFOut FOut("output/clusters.bin");
    Clusters.Save(FOut); //TODO: rewrite the method that needs this?
    ClusterSummaries.Save(FOut);
    Log.Save(FOut);
  } else {
    fprintf(stderr, "Incremental saving, w00t!\n");
    TStr OutputDir = "/lfs/1/tmp/curis/QBDBC/";
    TStr FileName = "QBDBC" + PresentTime.GetDtYmdStr() + ".bin";
    fprintf(stderr, "Attempting save to: %s\n", (OutputDir + FileName).CStr());
    TFOut FOut(OutputDir + FileName);
    fprintf(stderr, "Attempting save to: %s\n", (OutputDir + FileName).CStr());
    QB.Save(FOut);
    fprintf(stderr, "QB saved!\n");
    DB.Save(FOut);
    fprintf(stderr, "DB saved!\n");
    ClusterSummaries.Save(FOut);
    fprintf(stderr, "Cluster Summaries saved!\n");
    QGraph->Save(FOut); //TODO: why is this in memecluster?!?!?
  }

  /*
  // OUTPUT
  Log.SetupFiles(); // safe to make files now.
  fprintf(stderr, "Writing cluster information to file\n");
  Log.OutputClusterInformation(DB, QB, ClusterSummaries);
  fprintf(stderr, "Writing top clusters to file\n");
  Log.WriteClusteringOutputToFile();

  // plot output
  ClusterPlot Plotter(TStr("/lfs/1/tmp/curis/"));
  Plotter.PlotClusterSizeUnique(Clusters);
  Plotter.PlotClusterSize(ClusterSummaries);
  Plotter.PlotQuoteFrequencies(QB);
  */
  //delete QB;
  //delete DB;
  printf("Done!\n");
  return 0;
}
