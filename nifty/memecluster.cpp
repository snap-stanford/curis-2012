#include "stdafx.h"
#include "clustering.h"
#include "quotegraph.h"
#include "quote.h"
#include "doc.h"
#include "clusterplot.h"
#include "logoutput.h"
#include <stdio.h>

void OutputClusterInformation(TQuoteBase* QB, TVec<TTriple<TInt, TInt, TIntV> >& RepQuotesAndFreq, TStr FileName) {
  FILE *F = fopen(FileName.CStr(), "w");
  TFOut ClusterFile(FileName);

  for (int i = 0; i < RepQuotesAndFreq.Len(); i++) {
    TQuote RepQuote;
    QB->GetQuote(RepQuotesAndFreq[i].Val1, RepQuote);
    TStr RepQuoteStr;
    RepQuote.GetContentString(RepQuoteStr);
    TInt FreqOfAllClusterQuotes = RepQuotesAndFreq[i].Val2;
    TIntV QuotesInCluster = RepQuotesAndFreq[i].Val3;
    fprintf(F, "%d\t%d\t%s\n", FreqOfAllClusterQuotes.Val, QuotesInCluster.Len(), RepQuoteStr.CStr());
    for (int j = 0; j < QuotesInCluster.Len(); j++) {
      TQuote Quote;
      QB->GetQuote(QuotesInCluster[j], Quote);
      TStr QuoteStr;
      Quote.GetContentString(QuoteStr);
      fprintf(F, "\t%d\t%s\n", Quote.GetNumSources().Val, QuoteStr.CStr());
    }
  }
  //Save(QuotesFile);
  fclose(F);
}

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

int main(int argc, char *argv[]) {
  THash<TStr, TStr> Arguments;
  for (int i = 1; i < argc; i++) {
    if (strlen(argv[i]) >= 2 && argv[i][0] == '-' && i + 1 < argc) {
      Arguments.AddDat(TStr(argv[i] + 1), TStr(argv[i + 1]));
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
  LogOutput log;

  TFIn BaseFile(BaseString);

  if (argc >= 3) {
    OutputString = TStr(argv[2]);
  }
  TQuoteBase *QB = new TQuoteBase;
  TDocBase *DB = new TDocBase;
  QB->Load(BaseFile);
  DB->Load(BaseFile);

  //PlotQuoteFreq(QB, DB);

  // create clusters and save!
  QuoteGraph GraphCreator(QB);
  PNGraph QGraph;
  GraphCreator.CreateGraph(QGraph);
  Clustering ClusterJob(log);
  ClusterJob.SetGraph(QGraph);
  TIntSet RootNodes;
  TVec<TIntV> Clusters;
  ClusterJob.BuildClusters(RootNodes, Clusters, QB, DB);
  TVec<TTriple<TInt, TInt, TIntV> > RepQuotesAndFreq;
  ClusterJob.SortClustersByFreq(RepQuotesAndFreq, Clusters, QB);
  OutputClusterInformation(QB, RepQuotesAndFreq, OutputString);

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
