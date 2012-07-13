#include "stdafx.h"
#include "clustering.h"
#include "quotegraph.h"
#include "quote.h"
#include "doc.h"
#include "clusterplot.h"
#include "logoutput.h"
#include <stdio.h>

void PlotQuoteFreq(TQuoteBase *QB, TDocBase *DB) {
  printf("Testing graph quote\n");
  TIntV AllQuotes;
  QB->GetAllQuoteIds(AllQuotes);
  // Sort by descending frequency of quote
  AllQuotes.SortCmp(TCmpQuoteByFreq(false, QB)); 

  for (int i = 0; i < 100; i++) {
    TQuote Q;
    QB->GetQuote(AllQuotes[i], Q);
    TStr Filename = TStr("./plots/Freq" + Q.GetNumSources().GetStr() + "Quote" + Q.GetId().GetStr());
    //Q.GraphFreqOverTime(DB, Filename);
  }
}

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
    if (QSourcesUrl.IsSorted()) {
      printf("%s\n", "Successfully sorted!");
    } else {
      printf("%s\n", "Failed to sort :(");
    }
  }

  fclose(F);
}

int main(int argc, char *argv[]) {
  LogOutput log;
  THash<TStr, TStr> Arguments;
  for (int i = 1; i < argc; i++) {
    if (strlen(argv[i]) >= 2 && argv[i][0] == '-' && i + 1 < argc) {
      Arguments.AddDat(TStr(argv[i] + 1), TStr(argv[i + 1]));
      log.LogValue(TStr(argv[i] + 1), TStr(argv[i + 1]));
      i++;
    } else {
      printf("Error: incorrect format. Usage: ./memetracker [-paramName parameter]");
      exit(1);
    }
  }
  // load QB and DB. Custom variables can be added later.
  TStr BaseString = "/lfs/1/tmp/curis/QBDB.bin";
  TStr OutputString = "TopClusters.txt";
  if (Arguments.IsKey("qbdb")) {
    BaseString = Arguments.GetDat("qbdb");
  }
  if (Arguments.IsKey("output")) {
    OutputString = Arguments.GetDat("output");
  }

  TFIn BaseFile(BaseString);

  if (argc >= 3) {
    OutputString = TStr(argv[2]);
  }
  TQuoteBase *QB = new TQuoteBase;
  TDocBase *DB = new TDocBase;
  QB->Load(BaseFile);
  DB->Load(BaseFile);

  //PlotQuoteFreq(QB, DB);
  PrintQuoteURLs(QB, DB);

  // create clusters and save!
  QuoteGraph GraphCreator(QB);
  PNGraph QGraph;
  GraphCreator.CreateGraph(QGraph);
  Clustering ClusterJob;
  ClusterJob.SetGraph(QGraph);
  TIntSet RootNodes;
  TVec<TIntV> Clusters;
  ClusterJob.BuildClusters(RootNodes, Clusters, QB, DB, log);
  TVec<TTriple<TInt, TInt, TIntV> > RepQuotesAndFreq;
  ClusterJob.SortClustersByFreq(RepQuotesAndFreq, Clusters, QB);

  // OUTPUT
  log.SetupFiles(); // safe to make files now.
  log.OutputClusterInformation(QB, RepQuotesAndFreq);
  log.WriteClusteringOutputToFile();

  // plot output
  ClusterPlot Plotter(TStr("/lfs/1/tmp/curis/"));
  Plotter.PlotClusterSizeUnique(Clusters);
  Plotter.PlotClusterSize(RepQuotesAndFreq);
  Plotter.PlotQuoteFrequencies(QB);
  delete QB;
  delete DB;
  printf("Done!\n");
  return 0;
}
